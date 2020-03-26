# Singleplayer mode
AG 6.7 introduces a command to enable playing singleplayer maps/campaigns, but in a different way to vanilla HL.
In previous versions, you could only play singleplayer maps by modifying _liblist.gam_.
From AG 6.7 on, you can just type `sv_singleplayer 1`, reload a map with `map <name>` and there you go: triggers and other entities will work as in HL Steam.
You have to be in a map (ingame) when setting `sv_singleplayer` for it to work, and then reload the map. Alternatively, you can also set it in `ag/startup_server.cfg`
which is automatically executed right before the first map loads.

Singleplayer in AG has some quirks. It uses most of the multiplayer features, like vertical gauss boosting, constant 10hp falldamage...
and some more stuff, like dead corpses being non-solid, scientists and barneys having some more health... you can find the changes below.

Also some singleplayer maps of HL1 have been modified to fit these changes. The maps are included in the AG package,
not here in this repo. You can find a download link for AG in the README, and with it you can run the AG% category extension of HL speedrunning. If you're going to do singleplayer speedruns in AG, you'll also need BunnymodXT, [this one](https://drive.google.com/open?id=1fJTl5TG9H2pGBVdUs7lOFzkrwqQUcuO8) works with AG; it's not merged yet with the [official BunnymodXT](https://github.com/YaLTeR/BunnymodXT), but it will be in the future.

If you already have AG, then you can download the minimal files to speedrun AG% from [here](https://drive.google.com/open?id=1hSErqgUqDmbJCYJZLAHsrfC6NziW1494).

## Changelog
These are the changes to singleplayer in AG 6.7 when compared to HL Steam, and some are specific to the HL campaign.
- You're now able to hold the gauss charge between maps (not on saveloads though).
- Fixed hornet-related crash.
- Fixed weapon drop related crash.
- Fixed satchel softlock.
- Fixed getting stuck on slopes at high FPS.
- Fixed On A Rail collisions (c2a2a ~ c2a2f).
- Fixed the collisions at the start of Residue Processing (at c2a4).
- Fixed collision with the top of slopes (at c2a4d).
- Fixed getting too much damage when hurt during changelevel.
- Fixed scientist on QE disappearing when going through changelevel.
- Fixed scientist on QE going too far upstairs (at c2a4e).
- Fixed grunt on QE going downstairs and scaring the scientist (at c2a4f).
- Fixed WGH door getting closed.
- Removes Game Over triggered by damaging a scientist (at c3a2 and c3a2d).
- Moved further the scientist that may get into your path when finishing the first map of AM (at c1a0).
- Removed some scripted sequences for the scientist at the start of the 2nd map of AM (c1a0d), so it doesn't get in your way.
- Removed glass on the doors at the end of the second map of AM (c1a0d), as starting with HEV suit disables doing that part normally.
- Leechs (worms in Apprehension) are now less annoying.
- Dead corpses are now non-solid.
- Now grunts always drop their items upon death, even when killed with a direct hit of grenade.
- Gman is now killable (it has no death animation though).
- Sped up TC cutscene.
- Cockroaches are now non-solid.
- Slightly improved accuracy with crowbar.
- Decreased crowbar attack delay on pickup.
- Decreased satchel deploy time.
- Removed direct self-gauss.
- Fall damage is now always 10hp, so it doesn't depend on fall height.
- Overall increased damage dealt with weapons.
- Enabled vertical gauss boost.
- Lowered the time to charge gauss fully from 4s to 1.5s.
- Lowered ammo burning factor for gauss, so it takes a bit less charge to get the max. boost.
- Now crowbar always deals full damage (subsequent swings after the first one were halved).
- Now items reappear after a while and chargers get refilled too.
- Halved the spread for 9mmAR.
- Halved the vertical spread for shotgun.
- Enabled 357 zoom.
- Lowered egon ammo usage from 3 ammo/s to 1 ammo/s.
- Increased egon damage radius.
- Enabled crossbow zoomed hitscan and explosive projectile attacks.
- Doubled the RPG ammo received upon pickup.
- Now weapons can be dropped.
- HEV chargers now give at maximum 30 AP.
- Now you start with crowbar and glock in the campaign or map.
- Now scientists and barneys have +50 and +30 hp respectively, to be balanced with the increased weapon damage.

## Known issues
Some of these are specific to the HL campaign.
- The modified maps are generally too dark, except the 2nd QE map that maybe it's too bright. This will be fixed.
- Prestrafing is slower. This is because in AG we use `sv_maxspeed 300` by default. This won't change, as this is AG after all, not HL.
- It's harder to get stuck in doors, which you may want to do in case the door deals negative damage (heals you). This issue will probably not be fixed, as it's hard to debug what is causing it, seems like in HL WON there's some kind of attraction towards the door.
- Gauss charging sound is not completely correct and it makes an annoying noise when released after a saveload. This is because the client and server sides of the game are not the same yet. The game client will be updated soon, fixing this issue.
- Gauss charge (secondary fire) is always released on saveload, so remember to not save or load a save while charging, unless you want to quickgauss (I don't really know if that's working properly or is worthy here), because the charge will be released potentially making you lose time and some ammo. This issue can't be fixed, unless you come up with a solution, of course.
- If you go through a load pressing some key and you keep holding the same key after the load, it will keep that state even if you stop pressing that key. This bug exists because of the fix for keeping gauss charge through changelevels. It's not critical because you can just press some other key to avoid that from happening, and you won't notice it if you're bhopping because you're pressing A/D to keep strafing, which cancels the bug as you're pressing and releasing different keys. So, if you notice that this is happening, just press some other key that you weren't pressing during the load. This issue will continue to exist, probably until we find a fix for the issue related to keeping gauss charge during loads.
- The scientist at the start of Lambda Core, where you kill 4 alien grunts, will not cancel his scanner reading animation when shot. This might be fixed in the future, but at least you can throw a grenade right when he finishes reading the first scanner, so he runs towards the door and opens it without having to wait for the sequence in the second scanner.
- If you go walking close to the side of the pipe right before starting Residue Processing, you'll get stuck, this is because the first map of RP has different clipping to fix getting stuck after going down at high speed, so you can always keep the a lot of speed no matter where you land. The solution is to go through the middle of the pipe, or to prestrafe and start the bhop chain right after falling into the pipe, before the first map of RP is loaded, so you are in the air (jumping) at the moment you go through the changelevel trigger. This issue will probably not be fixed, as it requires too much work for the low chances of happening (even if you don't consciously try to avoid it).
- When going upstairs in QE for the gauss, the first bullsquid is too close to the stairs and you might collide with it as it's not expected to be there. You just position yourself near the left or right wall a bit sooner and bhop around it, and after some runs you'll get used to it. This issue will be fixed.
- Some voices in TC overlap, I don't like them overlapping, but it's not that important. This will probably be fixed.
- Some texts are wrong, like the text that appears when you save and chapter titles. This will probably be fixed.
- Banners in the 3rd map of AM don't rotate, like the banner where you can see "PLAY AG - PLAY IT NOW". This will probably not be fixed.
