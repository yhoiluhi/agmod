//++ BulliT

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "agglobal.h"
#include "agvote.h"
#include "agcommand.h"
#include "cvar.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern int gmsgVote;

FILE_GLOBAL char* s_szCommands[] =
{
  "yes - Vote yes.",
  "no  - Vote no.",
  "<vote> <value> - Call a vote.",
  "aglistvotes - List available votes."
};

FILE_GLOBAL char* s_szVotes[] =
{
  "agkick <name/#number> - Kick a player.",
  "agadmin <name/#number> - Vote a player admin.",
  "agstart <full/nolock> - Start a match. (full as value to start with all weps)",
  "agabort - Abort a match.",
  "agallow <name> - Allow a player into the match.",
  "agpause - Pause server.",
  "agmap <mapname> - Change level.",
  "agnextmap <mapname> - Change level after this is done.",
  "ag_spectalk <0/1> - Allow spectators to talk to all.",
  "agnextmode <mode> - Change mode after this level.",
  "agmaxtime - Extends the timelimit to the max allowed by the server.",
  "agmoretime - Extends the timelimit by a certain amount.",
  "ag_spawn_system <0-3> - Change the player spawn system. 0=classic, 1=random, 2=far, 3=position-aware.",
  "ag_spawn_history_entries <number> - How many of the last used spawnpoints have to be remembered",
  "ag_spawn_avoid_last_spots <fraction> - A fraction of total spawnpoints, that tells the PA and Far systems to avoid using that number of recently used spots. 0.3 is 30%, which is 5 spawns in crossfire, or 7 in boot_camp",
  "ag_spawn_far_spots <fraction> - A fraction of total spawnpoints, that tells the Far system to use that many of the furthest spots to pick one randomly"
  "ag_spawn_pa_visible_chance <number> - Probability of a visible spawnpoint being chosen in the PA system",
  "ag_spawn_pa_audible_chance <number> - Probability of an audible spawnpoint being chosen in the PA system",
  "ag_spawn_pa_safe_chance <number> - Probability of a safe spawnpoint being chosen in the PA system",
  "ag_fov_limit <number> - 0 to disable, any other number to cap everyones' fov to that number",
  "ag_fps_limit <number> - 0 to disable, any other number to cap everyones' fps to that number",
  "ag_fps_limit_auto <number> - 0 to disable, 1 to limit automatically based on most used fps at that moment",
  "ag_fps_limit_check_interval <number> - How often to recalculate the fps limit when it's auto (in seconds)",
};

AgVote::AgVote()
{
    m_fNextCount = 0.0;
    m_fMaxTime = 0.0;
    m_fNextVote = AgTime();
    m_bRunning = false;
    m_sTarget = nullptr;
}

AgVote::~AgVote()
{

}

bool AgVote::HandleCommand(CBasePlayer* pPlayer)
{
    ASSERT(NULL != pPlayer);
    if (!pPlayer)
        return false;
    ASSERT(NULL != pPlayer->pev);
    if (!pPlayer->pev)
        return false;
    ASSERT(NULL != g_pGameRules);
    if (!g_pGameRules || 0 == CMD_ARGC())
        return false;

    if (1 > ag_allow_vote.value)
        return false;

    if (FStrEq(CMD_ARGV(0), "help"))
    {
        for (int i = 0; i < sizeof(s_szCommands) / sizeof(s_szCommands[0]); i++)
            AgConsole(s_szCommands[i], pPlayer);

        return true;
    }

    if (FStrEq(CMD_ARGV(0), "aglistvotes"))
    {
        for (int i = 0; i < sizeof(s_szVotes) / sizeof(s_szVotes[0]); i++)
            AgConsole(s_szVotes[i], pPlayer);

        GameMode.Help(pPlayer);

        return true;
    }

    //Atleast two players.
    int iPlayers = 0;
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayerLoop = AgPlayerByIndex(i);
        if (pPlayerLoop && !pPlayerLoop->IsProxy())
            iPlayers++;
    }

    /*
    if (IS_DEDICATED_SERVER() && 2 > iPlayers)
      return true;
    */
    if (m_fMaxTime || m_fNextCount)
    {
        if (pPlayer->HasVotingRestrictions())
        {
            if (FStrEq("yes", CMD_ARGV(0)) || FStrEq("no", CMD_ARGV(0)))
                AgConsole("Spectators are not allowed to vote during this match.", pPlayer);

            return false;
        }

        if (FStrEq("yes", CMD_ARGV(0)))
        {
            pPlayer->m_iVote = 1;
            return true;
        }
        else if (FStrEq("no", CMD_ARGV(0)))
        {
            pPlayer->m_iVote = 0;
            return true;
        }
        else
        {
            AgConsole("Vote is running, type yes or no in console.", pPlayer);
            return true;
        }
    }
    else
    {
        if (m_fNextVote > AgTime())
        {
            AgConsole(UTIL_VarArgs("Last vote was not accepted - %d seconds until next vote can be called.", (int)(m_fNextVote - AgTime())), pPlayer);

            return true;
        }

        ResetVote();

        if (FStrEq("callvote", CMD_ARGV(0)) || FStrEq("vote", CMD_ARGV(0)))
        {
            if (2 <= CMD_ARGC())
                m_sVote = CMD_ARGV(1);
            if (3 <= CMD_ARGC())
            {
                m_sValue = CMD_ARGV(2);
                m_sFullValue = CMD_ARGS();
            }
            if (4 <= CMD_ARGC())
                m_sValue2 = CMD_ARGV(3);
        }
        else
        {
            if (1 <= CMD_ARGC())
                m_sVote = CMD_ARGV(0);
            if (2 <= CMD_ARGC())
            {
                m_sValue = CMD_ARGV(1);
                m_sFullValue = CMD_ARGS();
            }
            if (3 <= CMD_ARGC())
                m_sValue2 = CMD_ARGV(2);
        }

        if (m_sVote.size() && 64 > m_sVote.size() && 32 > m_sValue.size())
        {
            //Check map
            if (FStrEq(m_sVote.c_str(), "agmap") ||
                FStrEq(m_sVote.c_str(), "changelevel") ||
                FStrEq(m_sVote.c_str(), "map"))
            {
                if (!ag_vote_map.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                char szTemp[64];
                strcpy(szTemp, m_sValue.c_str());

                //Check if it exists.
                if (IS_MAP_VALID(szTemp))
                {
                    m_sVote = "agmap";
                    CallVote(pPlayer);
                }
                else
                    AgConsole("Map doesn't exist on server.", pPlayer);

                return true;
            }
            //Check nextmap
            else if (FStrEq(m_sVote.c_str(), "agnextmap"))
            {
                if (!ag_vote_map.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                char szTemp[64];
                strcpy(szTemp, m_sValue.c_str());

                //Check if it exists.
                if (IS_MAP_VALID(szTemp))
                {
                    m_sVote = "agnextmap";
                    CallVote(pPlayer);
                }
                else
                    AgConsole("Map doesn't exist on server.", pPlayer);
                return true;
            }
            //Check mode
            else if (GameMode.IsGamemode(m_sVote))
            {
                if (!ag_vote_gamemode.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                if (!GameMode.IsAllowedGamemode(m_sVote, pPlayer))
                {
                    AgConsole("Gamemode not allowed by server admin.", pPlayer);
                    return true;
                }

                CallVote(pPlayer);
                return true;
            }
            //Check nextmode
            else if (FStrEq(m_sVote.c_str(), "agnextmode"))
            {
                if (!ag_vote_gamemode.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                if (!GameMode.IsAllowedGamemode(m_sValue, pPlayer))
                {
                    AgConsole("Gamemode not allowed by server admin.", pPlayer);
                    return true;
                }

                CallVote(pPlayer);

                return true;
            }
            //Start and pause should be there.
            else if (FStrEq(m_sVote.c_str(), "agstart") ||
                FStrEq(m_sVote.c_str(), "agabort") ||
                FStrEq(m_sVote.c_str(), "agpause"))
            {
                if (LMS == AgGametype() || ARENA == AgGametype())
                {
                    AgConsole("Vote is not allowed in this gamemode.", pPlayer);
                    return true;
                }

                if (1 > ag_vote_start.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                CallVote(pPlayer);
                return true;
            }
            //Check command
            else if (FStrEq(m_sVote.c_str(), "agallow") ||
                FStrEq(m_sVote.c_str(), "agkick") ||
                FStrEq(m_sVote.c_str(), "agadmin") ||
                0 == strncmp(m_sVote.c_str(), "agforceteam", 11) ||
                FStrEq(m_sVote.c_str(), "agforcespectator"))
            {
                // TODO: this is ugly, refactor
                if ((FStrEq(m_sVote.c_str(), "agkick") && 1 > ag_vote_kick.value)
                    || (FStrEq(m_sVote.c_str(), "agadmin") && 1 > ag_vote_admin.value)
                    || (FStrEq(m_sVote.c_str(), "agallow") && 1 > ag_vote_allow.value)
                    || (0 == strncmp(m_sVote.c_str(), "agforceteam", 11) && 1 > ag_vote_team.value)
                    || (FStrEq(m_sVote.c_str(), "agforcespectator") && 1 > ag_vote_spectator.value)
                    )
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                if (m_sValue.empty())
                {
                    if (!FStrEq(m_sVote.c_str(), "agkick")
                        && 0 != strncmp(m_sVote.c_str(), "agforceteam", 11)
                        && !FStrEq(m_sVote.c_str(), "agforcespectator"))
                    {
                        // Target is the caller
                        m_sValue = pPlayer->GetName();
                        m_sAuthID = pPlayer->GetAuthID();
                        m_sTarget = pPlayer;
                        CallVote(pPlayer);

                        return true;
                    }
                    else
                    {
                        AgConsole("Please, specify a target player for this vote.", pPlayer);
                        return true;
                    }
                }
                else if (m_sValue2.empty() && 0 == strncmp(m_sVote.c_str(), "agforceteam", 11))
                {
                    AgConsole("Please, specify a team for the target player.", pPlayer);
                    return true;
                }

                CBasePlayer* pPlayerLoop = AgPlayerByName(m_sValue, pPlayer);
                if (pPlayerLoop)
                {
                    m_sAuthID = pPlayerLoop->GetAuthID();
                    m_sTarget = pPlayerLoop;
                    CallVote(pPlayer);
                }
                return true;
            }
            else if (FStrEq(m_sVote.c_str(), "agmaxtime"))
            {
                if (!ag_vote_setting.value)
                {
                    AgConsole("Voting settings is not allowed here.", pPlayer);
                    return true;
                }
                CallVote(pPlayer);

                return true;
            }
            else if (FStrEq(m_sVote.c_str(), "agmoretime"))
            {
                if (!ag_vote_setting.value)
                {
                    AgConsole("Voting settings is not allowed here.", pPlayer);
                    return true;
                }
                if (!ag_vote_extra_timelimit.value)
                {
                    AgConsole("Voting for extra timelimit is not allowed here.", pPlayer);
                    return true;
                }

                // Don't allow extending the timelimit too quickly... maybe someone wanted to add days of timelimit by spamming this
                auto remainingMinutes = CVAR_GET_FLOAT("mp_timeleft") / 60.0f;
                auto cooldown = remainingMinutes - (ag_vote_extra_timelimit.value / 2.0f);
                if (cooldown > 0.0f)
                {
                    AgConsole(UTIL_VarArgs("Can't vote for extended timelimit yet. Please, try again in %d minutes",
                        static_cast<int>(std::ceil(cooldown))), pPlayer);
                    return true;
                }
                CallVote(pPlayer);

                return true;
            }
            else if (FStrEq(m_sVote.c_str(), "spawnbot"))
            {
                if (ag_vote_bot.value == 0.0f)
                {
                    AgConsole("Adding bots by vote is not allowed by server admin.", pPlayer);
                    return true;
                }
                if (ag_match_running.value != 0.0f && !g_bLangame)
                {
                    AgConsole("Sorry, can't add a bot during a match.", pPlayer);
                    return true;
                }

                auto botsCount = 0;
                for (int i = 1; i <= gpGlobals->maxClients; i++)
                {
                    CBasePlayer* player = AgPlayerByIndex(i);

                    if (!player)
                        continue;

                    if (player->IsBot())
                        botsCount++;
                }

                if (botsCount >= ag_bot_limit.value)
                {
                    AgConsole(UTIL_VarArgs("The limit of %d bots has been reached", (int) ag_bot_limit.value), pPlayer);
                    return true;
                }

                CallVote(pPlayer);
                return true;
            }
            else if (FStrEq(m_sVote.c_str(), "agdefault"))
            {
                if (ag_vote_setting.value != 2.0f)
                {
                    AgConsole("Defaulting gamemode cvars is not allowed here.", pPlayer);
                    return true;
                }
                CallVote(pPlayer);

                return true;
            }
            else if (0 == strncmp(m_sVote.c_str(), "mp_timelimit", 12))
            {
                if (!ag_vote_setting.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                // TODO: refactor similar checks for fraglimit and others
                if (m_sValue.empty())
                {
                    AgConsole(UTIL_VarArgs("%s is \"%s\"", m_sVote.c_str(), timelimit.string), pPlayer);
                    return true;
                }
                else if (m_sValue == "0" && ag_vote_mp_timelimit_high.value > 0.0f)
                {
                    AgConsole(UTIL_VarArgs("Can't vote unlimited time. The max. you can vote is %s.",
                        ag_vote_mp_timelimit_high.string), pPlayer);

                    return true;
                }

                if (atof(m_sValue.c_str()) > ag_vote_mp_timelimit_high.value && ag_vote_mp_timelimit_high.value != 0.0f)
                {
                    AgConsole(UTIL_VarArgs("Can't vote this. It's too high of a timelimit. Please, try a lower value. (max time: %s)",
                        ag_vote_mp_timelimit_high.string), pPlayer);

                    return true;
                }
                else if (atof(m_sValue.c_str()) < ag_vote_mp_timelimit_low.value && m_sValue != "0")
                {
                    AgConsole(UTIL_VarArgs("Can't vote this. It's too low of a timelimit. Please, try a higher value. (min time: %s)",
                        ag_vote_mp_timelimit_low.string), pPlayer);
                    return true;
                }
                CallVote(pPlayer);
                return true;
            }
            else if (0 == strncmp(m_sVote.c_str(), "mp_fraglimit", 12))
            {
                if (!ag_vote_setting.value)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                if (m_sValue.empty())
                {
                    AgConsole(UTIL_VarArgs("%s is \"%s\"", m_sVote.c_str(), fraglimit.string), pPlayer);
                    return true;
                }
                else if (m_sValue == "0" && ag_vote_mp_fraglimit_high.value > 0.0f)
                {
                    AgConsole(UTIL_VarArgs("Can't vote unlimited frags. The max. you can vote is %s.",
                        ag_vote_mp_fraglimit_high.string), pPlayer);

                    return true;
                }

                if (atof(m_sValue.c_str()) > ag_vote_mp_fraglimit_high.value && ag_vote_mp_fraglimit_high.value != 0.0f)
                {
                    AgConsole(UTIL_VarArgs("Can't vote this. It's too high of a timelimit. Please, try a lower value. (max time: %d)",
                        ag_vote_mp_fraglimit_high.string), pPlayer);

                    return true;
                }
                else if (atof(m_sValue.c_str()) < ag_vote_mp_fraglimit_low.value && m_sValue != "0")
                {
                    AgConsole(UTIL_VarArgs("Can't vote this. It's too low of a timelimit. Please, try a higher value. (min time: %d)",
                        ag_vote_mp_fraglimit_low.string), pPlayer);
                    return true;
                }
                CallVote(pPlayer);
                return true;
            }
            else if (CVar::IsVotable(m_sVote))
            {
                if (CVar::IsBasicSetting(m_sVote))
                {
                    if (ag_vote_setting.value == 0.0f)
                    {
                        AgConsole("Vote is not allowed by server admin.", pPlayer);
                        return true;
                    }
                }
                else if (ag_vote_setting.value != 2.0f)
                {
                    AgConsole("Vote is not allowed by server admin.", pPlayer);
                    return true;
                }

                if (m_sValue.empty())
                {
                    AgConsole(UTIL_VarArgs("%s is \"%s\"", m_sVote.c_str(), CVAR_GET_STRING(m_sVote.c_str())), pPlayer);
                    return true;
                }

                CallVote(pPlayer);
                return true;
            }
        }
    }

    return false;
}

bool AgVote::CallVote(CBasePlayer* pPlayer)
{
    if (pPlayer->IsBot() && ag_bot_allow_vote.value == 0)
    {
        // Bots cannot start votes when sv_ag_bots_allow_vote is 0
        return false;
    }
    if (pPlayer->HasVotingRestrictions() && !pPlayer->IsAdmin())
    {
        AgConsole("Spectators are not allowed to start a vote during this match.", pPlayer);
        return false;
    }

    m_fMaxTime = AgTime() + 30.0;  //30 seconds is enough.
    m_fNextCount = AgTime();       //Next count directly

    pPlayer->m_iVote = 1;          //Voter voted yes
#ifdef _DEBUG
    pPlayer->m_iVote = 0;
#endif
    m_sCalled = pPlayer->GetName();
    m_sCallerID = pPlayer->GetAuthID();
    m_bRunning = true;

    //++ muphicks
    UTIL_LogPrintf("\"%s<%d><%s><%s>\" triggered \"calledvote\" (votename \"%s\") (newsetting \"%s\")\n",
        pPlayer->GetName(), GETPLAYERUSERID(pPlayer->edict()), GETPLAYERAUTHID(pPlayer->edict()), pPlayer->TeamID(),
        m_sVote.c_str(), m_sValue.c_str()
    );
    //-- muphicks

    return true;
}


void AgVote::Think()
{
    if (!m_bRunning)
        return;

    //Count votes.
    if (m_fNextCount != 0.0 && m_fNextCount < AgTime())
    {
        int iFor, iAgainst, iUndecided, iPlayers;
        iFor = iAgainst = iUndecided = iPlayers = 0;

        //Count players
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayerLoop = AgPlayerByIndex(i);
            if (pPlayerLoop && !pPlayerLoop->IsProxy())
            {
                if (pPlayerLoop->IsBot() && ag_bot_allow_vote.value == 0)
                {
                    // Bots do not take part in votes when sv_ag_bot_allow_vote is 0
                    continue;
                }
                if (pPlayerLoop->HasVotingRestrictions())
                {
                    // Players who are not playing in the match cannot vote if votes are restricted
                    continue;
                }

                iPlayers++;

                if (1 == pPlayerLoop->m_iVote)
                    iFor++;
                else if (0 == pPlayerLoop->m_iVote)
                    iAgainst++;
                else
                    iUndecided++;
            }
        }

        //Check if enough.
        if (((float)iFor / (float)iPlayers > 0.5))
        {
#ifdef AG_NO_CLIENT_DLL
            UTIL_ClientPrintAll(HUD_PRINTCENTER, UTIL_VarArgs("Vote: %s %s\nCalled by: %s\nAccepted!", m_sVote.c_str(), m_sValue.c_str(), m_sCalled.c_str()));
#else
            MESSAGE_BEGIN(MSG_BROADCAST, gmsgVote, NULL);
            WRITE_BYTE(Accepted);
            WRITE_BYTE(iFor);
            WRITE_BYTE(iAgainst);
            WRITE_BYTE(iUndecided);
            WRITE_STRING(m_sVote.c_str());
            WRITE_STRING(UTIL_VarArgs("%s %s", m_sValue.c_str(), m_sValue2.c_str()));
            WRITE_STRING(m_sCalled.c_str());
            MESSAGE_END();
#endif

            //Exec vote.
            if (FStrEq(m_sVote.c_str(), "agadmin"))
            {
                if (m_sTarget && m_sTarget->pev)
                {
                    m_sTarget->SetIsAdmin(true);
                }
                else
                {
                    for (int i = 1; i <= gpGlobals->maxClients; i++)
                    {
                        CBasePlayer* pPlayerLoop = AgPlayerByIndex(i);
                        if (pPlayerLoop && pPlayerLoop->GetAuthID() == m_sAuthID)
                        {
                            pPlayerLoop->SetIsAdmin(true);
                            break;
                        }
                    }
                }

            }
            else if (FStrEq(m_sVote.c_str(), "agallow"))
            {
                if (m_sTarget && m_sTarget->pev)
                    Command.Allow(m_sTarget);
                else
                    Command.Allow(m_sValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agmap"))
            {
                Command.Map(m_sValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agnextmap"))
            {
                Command.NextMap(m_sValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agstart"))
            {
                Command.Start(m_sFullValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agpause"))
            {
                Command.Pause(NULL);
            }
            else if (FStrEq(m_sVote.c_str(), "agabort"))
            {
                Command.Abort(NULL);
            }
            else if (GameMode.IsAllowedGamemode(m_sVote))
            {
                // TODO: move this down in the else-if chain, because i guess
                // you could name your gamemode "agkick" and then agkick would
                // no longer work, it would try to load that gamemode instead
                GameMode.Gamemode(m_sVote);
            }
            else if (FStrEq(m_sVote.c_str(), "agnextmode"))
            {
                GameMode.NextGamemode(m_sValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agkick"))
            {
                if (m_sTarget && m_sTarget->pev)
                    Command.Kick(m_sTarget);
                else
                    Command.Kick(m_sValue);
            }
            else if (FStrEq(m_sVote.c_str(), "agmaxtime"))
            {
                Command.MaxTime();
            }
            else if (FStrEq(m_sVote.c_str(), "agmoretime"))
            {
                Command.MoreTime();
            }
            else if (FStrEq(m_sVote.c_str(), "spawnbot"))
            {
                Command.AddRespawningStaticBot(AgPlayerByAuthID(m_sCallerID));
            }
            else if (FStrEq(m_sVote.c_str(), "agdefault"))
            {
                Command.AgGamemodeDefaultCVars();
            }
            else if (0 == strncmp(m_sVote.c_str(), "agforceteam", 11))
            {
                if (m_sTarget && m_sTarget->pev)
                    Command.TeamUp(nullptr, m_sTarget, m_sValue2);
                else
                    Command.TeamUp(nullptr, m_sValue, m_sValue2);
            }
            else if (FStrEq(m_sVote.c_str(), "agforcespectator"))
            {
                if (m_sTarget && m_sTarget->pev)
                    Command.Spectator(nullptr, m_sTarget);
                else
                    Command.Spectator(nullptr, m_sValue);
            }
            else
            {
                Command.Setting(m_sVote.c_str(), m_sValue);
            }

            ResetVote();
        }
        else
        {
            if (m_fMaxTime < AgTime())
            {
#ifdef AG_NO_CLIENT_DLL
                UTIL_ClientPrintAll(HUD_PRINTCENTER, UTIL_VarArgs("Vote: %s %s\nCalled by: %s\nDenied!", m_sVote.c_str(), m_sValue.c_str(), m_sCalled.c_str()));
#else
                MESSAGE_BEGIN(MSG_BROADCAST, gmsgVote, NULL);
                WRITE_BYTE(Denied);
                WRITE_BYTE(iFor);
                WRITE_BYTE(iAgainst);
                WRITE_BYTE(iUndecided);
                WRITE_STRING(m_sVote.c_str());
                WRITE_STRING(UTIL_VarArgs("%s %s", m_sValue.c_str(), m_sValue2.c_str()));
                WRITE_STRING(m_sCalled.c_str());
                MESSAGE_END();
#endif

                ResetVote();
                m_fNextVote = ag_vote_failed_time.value + AgTime();
            }
            else
            {
#ifdef AG_NO_CLIENT_DLL
                UTIL_ClientPrintAll(HUD_PRINTCENTER, UTIL_VarArgs("Vote: %s %s\nCalled by: %s\nFor: %d\nAgainst: %d\nUndecided: %d", m_sVote.c_str(), m_sValue.c_str(), m_sCalled.c_str(), iFor, iAgainst, iUndecided));
#else
                MESSAGE_BEGIN(MSG_BROADCAST, gmsgVote, NULL);
                WRITE_BYTE(Called);
                WRITE_BYTE(iFor);
                WRITE_BYTE(iAgainst);
                WRITE_BYTE(iUndecided);
                WRITE_STRING(m_sVote.c_str());
                WRITE_STRING(UTIL_VarArgs("%s %s", m_sValue.c_str(), m_sValue2.c_str()));
                WRITE_STRING(m_sCalled.c_str());
                MESSAGE_END();
#endif
                m_fNextCount = AgTime() + 2.0; //Two more seconds.
            }
        }
    }
}


bool AgVote::ResetVote()
{
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayerLoop = AgPlayerByIndex(i);
        if (pPlayerLoop)
            pPlayerLoop->m_iVote = -1;
    }

    m_sVote = "";
    m_sValue = "";
    m_sValue2 = "";
    m_sFullValue = "";
    m_sTarget = nullptr;
    m_sCalled = "";
    m_sCallerID = "";
    m_fNextCount = 0.0;
    m_fMaxTime = 0.0;
    m_sAuthID = "";
    m_fNextVote = AgTime();
    m_bRunning = false;
    return true;
}

bool CBasePlayer::HasVotingRestrictions()
{
    if (ag_restrict_vote.value == 0.0f)
        return false;

    if (ag_restrict_vote.value == 1.0f && ag_match_running.value == 1.0f && IsSpectator())
        return true;

    // There may be ag_restrict_value 2 or higher in the future

    return false;
}

//-- Martin Webrant
