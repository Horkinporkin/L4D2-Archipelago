# Left 4 Dead 2 Archipelago Setup Guide
## Required software
- Left 4 Dead 2 on Steam
- Archipelago and the L4D2 APWorld
- Project assets
- SourceMod and MetaMod:Source + required plugins


## Installation
### APWorld + Plugin
Install the APWorld like any other. Either run the .apworld by clicking on it, or place it in `<Archipelago directory>\custom_worlds`

The Archipelago plugin and custom assets are included in the .zip. Simply extract the zip into your Left 4 Dead 2 base directory (`steamapps\common\Left 4 Dead 2`)

### SourceMod & MetaMod:Source
Both [SourceMod](https://www.sourcemod.net/downloads.php?branch=stable) and [MetaMod:Source](https://www.sourcemm.net/downloads.php?branch=stable) must be installed for the plugin to run. Simply download both and extract them into `<Left 4 Dead 2 directory>\left4dead2`

#### Required plugins:
These additional SourceMod plugins are required for the Archipelago plugin to run:
- [Socket](https://forums.alliedmods.net/showthread.php?t=67640)
- [Left 4 DHooks](https://forums.alliedmods.net/showthread.php?t=321696)

Extract the 'Socket' zip into `<Left 4 Dead 2 directory>\Left 4 Dead 2\left4dead2`  
Extract the 'Left 4 DHooks' zip into `<Left 4 Dead 2 directory>\Left 4 Dead 2\left4dead2\addons` 

### Steam
You must launch Left 4 Dead 2 with the `-insecure` launch option.
- In your Steam library, right click 'Left 4 Dead 2', go to 'Properties...',  and type `-insecure` in the 'Launch Options' area.
- This also serves as your "toggle" for the plugin until something nicer is implemented. Just remove `-insecure` to launch the game normally.


## Instructions
Once everything is installed, simply run the game and the Archipelago client. At the main menu of the game, press:  
- "Play Campaign" > "Single Player" (or create an Online Lobby) > "Start Game"  

You do not have to select any specific Campaign or Chapter to get started, the plugin will handle that once you load in (and I recommend leaving it on the default Dead Center - Hotel, as choosing certain other Chapters can cause the game to crash.)


## Client Configuration
The SourceMod plugin and Archipelago client listen on `127.0.0.1:12138` by default. **Most users should not change this.**  

If you decide that you need to change the connection, configure the bind address and port with these console commands (must have started a campaign to do so, not at the main menu):

```text
l4d2_ap_host <host>
l4d2_ap_port <port>
```
After running either of these commands, you must reload the plugin either by:
- exiting to the main menu and reloading a chapter
- running the command `sm plugins reload archipelago` in the in-game console

The client/bridge endpoint can be changed with the following launch options:  

```text
--bridge-host <host>
--bridge-port <port>
```

If you are not sure on how to use these launch options, you have a couple choices:
- In the Archipelago launcher, find the 'Left 4 Dead 2 Client', click the hamburger button (the three horizontal lines/menu button thing), and 'Add shortcut on desktop.' Right click the shortcut on the desktop, and put the launch options at the end of 'Target'.
  - The 'Target' field might look like: `C:\ProgramData\Archipelago\ArchipelagoLauncher.exe  "Left 4 Dead 2 Client" --bridge-host 127.0.0.1 --bridge-port 12138`  
  Be sure to adjust based on your install location and desired host/port.
- Open Terminal/Powershell in your Archipelago directory and run the command `.\ArchipelagoLauncher.exe "Left 4 Dead 2 Client" --bridge-host <host> --bridge-port <port>`

**You must use the same `<host>:<port>` on both the plugin and the bridge in order for the client to connect.**


## Commands
### Chat
The plugin allows you to use the following commands in the in-game chat:

`!ap`: This opens the Archipelago menu, primarily allowing you to navigate unlocked levels. This command is essential to the experience, you will be using it lots. 

`!appickup`: Forces the AP Item Carrier to spawn in a level. More of a contingency/cheat in the event that the AP Item Carrier does not spawn. Although, in all of my testing, I never **needed** to use the command. Go ahead and abuse this, I can't stop you. Just know I'll be disappointed.

`!ap_bridge`: Checks the bridge connection status. Useful if you are having connection issues.

### Console
Here are some commands you may find useful during your playthrough:

`sm plugins reload archipelago`: Reloads the Archipelago plugin. Restores the bridge connection between the game and client if it goes out for some reason.
