#include <algorithm>
#include <string>
#include <vector>

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "agflood.h"

namespace AgFlood
{
	namespace
	{
		struct Flooder
		{
			std::string steamID;
			double timeSinceLastNameChange;
			double timeSinceLastModelChange;

			Flooder(const char* steamID, double timeSinceLastNameChange, double timeSinceLastModelChange)
				: steamID(steamID),
				  timeSinceLastNameChange(timeSinceLastNameChange),
				  timeSinceLastModelChange(timeSinceLastModelChange)
			{}
		};

		std::vector<Flooder> flooders;
	}

	void UpdateFlooding(CBasePlayer* player)
	{
		const char* playerSteamID = player->GetAuthID();

		const auto timePassedForName  = AgTime() - player->m_flLastNameChange;
		const auto timePassedForModel = AgTime() - player->m_flLastModelChange;

		const auto it = std::find_if(
			flooders.begin(),
			flooders.end(),
			[=](const Flooder& entry) {
				return !strcmp(entry.steamID.c_str(), playerSteamID);
			});

		if (it != flooders.end())
		{
			it->timeSinceLastNameChange = timePassedForName;
			it->timeSinceLastModelChange = timePassedForModel;
		}
		else
			flooders.emplace_back(playerSteamID, timePassedForName, timePassedForModel);
	}

	void RestoreFlooding(CBasePlayer* player)
	{
		const auto it = std::find_if(
			flooders.begin(),
			flooders.end(),
			[=](const Flooder& entry) {
				return !strcmp(entry.steamID.c_str(), player->GetAuthID());
			});

		if (it != flooders.end())
		{
			player->m_flLastNameChange  = AgTime() - it->timeSinceLastNameChange;
			player->m_flLastModelChange = AgTime() - it->timeSinceLastModelChange;
		}

	}
}
