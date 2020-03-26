# Singleplayer mode
AG 6.7 introduces a command to enable playing singleplayer maps/campaigns, but in a different way to vanilla HL.
In previous versions, you could only play singleplayer maps by modifying _liblist.gam_.
From AG 6.7 on, you can just type `sv_singleplayer 1`, load a map with `map <name>` and there you go, triggers and other entities
will work.

Singleplayer in AG has some quirks. It uses most of the multiplayer features, like vertical gauss boosting, constant 10hp falldamage...
and some more stuff, like dead corpses being non-solid, scientists and barneys having some more health... you can find the changes
below.

Also some singleplayer maps of HL1 have been modified to fit these changes. The maps are included in the AG release package,
not here in this repo. You can run the AG% category extension of HL1 speedrunning with the package.

## Changes to singleplayer
- [x] You're now able to hold the gauss charge between maps (not on saveloads though).
- [x] Fixed hornet-related crash.
- [x] Fixed weapon drop related crash.
- [x] Fixed satchel softlock.
- [x] Fixed getting stuck on slopes at high FPS.
- [x] Fixed On A Rail collisions (c2a2a ~ c2a2f).
- [x] Fixed the collisions at the start of Residue Processing (at c2a4).
- [x] Fixed collision with the top of slopes (at c2a4d).
- [x] Fixed getting too much damage when hurt during changelevel.
- [x] Fixed scientist on QE disappearing when going through changelevel.
- [x] Fixed scientist on QE going too far upstairs (at c2a4e).
- [x] Fixed grunt on QE going downstairs and scaring the scientist (at c2a4f).
- [x] Fixed WGH door getting closed.
- [x] Removes Game Over triggered by damaging a scientist (at c3a2 and c3a2d).
- [x] Moved further the scientist that may get into your path when finishing the first map of AM (at c1a0).
- [x] Removed some scripted sequences for the scientist at the start of the 2nd map of AM (c1a0d), so it doesn't get in your way.
- [x] Removed glass on the doors at the end of the second map of AM (c1a0d), as starting with HEV suit disables doing that part normally.
- [x] Leechs (worms in Apprehension) are now less annoying.
- [x] Dead corpses are now non-solid.
- [x] Now grunts always drop their items upon death, even when killed with a direct hit of grenade.
- [x] Gman is now killable (it has no death animation though).
- [x] Sped up TC cutscene.
- [x] Cockroaches are now non-solid.
- [x] Slightly improved accuracy with crowbar.
- [x] Decreased crowbar attack delay on pickup.
- [x] Decreased satchel deploy time.
- [x] Removed direct self-gauss.
- [x] Fall damage is now always 10hp, so it doesn't depend on fall height.
- [x] Overall increased damage dealt with weapons.
- [x] Enabled vertical gauss boost.
- [x] Lowered the time to charge gauss fully from 4s to 1.5s.
- [x] Lowered ammo burning factor for gauss, so it takes a bit less charge to get the max. boost.
- [x] Now crowbar always deals full damage (subsequent swings after the first one were halved).
- [x] Now items reappear after a while and chargers get refilled too.
- [x] Halved the spread for 9mmAR.
- [x] Halved the vertical spread for shotgun.
- [x] Enabled 357 zoom.
- [x] Lowered egon ammo usage from 3 ammo/s to 1 ammo/s.
- [x] Increased egon damage radius.
- [x] Enabled crossbow zoomed hitscan and explosive projectile attacks.
- [x] Doubled the RPG ammo received upon pickup.
- [x] Now weapons can be dropped.
- [x] HEV chargers now give at maximum 30 AP.
- [x] Now you start with crowbar and glock in the campaign or map.
- [x] Now scientists and barneys have +50 and +30 hp respectively, to be balanced with the increased weapon damage.

