#pragma semicolon 1
#pragma dynamic 65536

#include <sourcemod>
#include <ap_socket>
#include <left4dhooks>
#include <sdktools>
#include <sdkhooks>
#include <nextmap>

#pragma newdecls required

#define AP_RECV_BUFFER_SIZE       65536
#define AP_NAME_LEN               96
#define AP_ITEM_KEY_LEN           (AP_NAME_LEN + 32)
#define AP_MAP_LEN                64
#define AP_WEAPON_SLOTS           5
#define AP_WEAPON_CLASS_LEN       64
#define AP_CHAT_PREFIX            "\x05[AP]\x01 "

// Weapon inventory slots
#define AP_SLOT_PRIMARY           0
#define AP_SLOT_SECONDARY         1
#define AP_SLOT_THROWABLE         2
#define AP_SLOT_HEALTH            3
#define AP_SLOT_TEMP_HEALTH       4

bool g_stateSynced;                    // SYNC_COMPLETE received for the current connection

// Route data, copied from CONFIGURE_ROUTE
ArrayList g_routeCampaigns;            // AP route: campaign labels
StringMap g_mapToChapter;              // map codename -> AP chapter name
StringMap g_chapterToMap;              // AP chapter name -> map codename
ArrayList g_vanillaCampaignOrder;      // ordered names shown in menus
StringMap g_unlockedChapters;          // chapter item received -> 1
StringMap g_completedChapters;         // chapter location checked (locally or confirmed)

bool g_mapSettled;                     // current map is playable

// Item effect tables (AP item name -> in-game data)
StringMap g_weaponClassnames;          // carriable weapon classnames
StringMap g_meleeWeaponTypes;          // melee script names


// Modules, ordering matters
#include <ap_bridge>
#include <ap_melee>
#include <ap_survival>
#include <ap_weapon_tiers>
#include <ap_inventory>
#include <ap_survivors>
#include <ap_deathlink>
#include <ap_protocol>
#include <ap_routes>
#include <ap_items>
#include <ap_players>
#include <ap_menus>
#include <ap_interactables>
#include <ap_traps>
#include <ap_chat>
#include <ap_pickups>

// Dev tool for finding interactable hooks
// #include <ap_discovery>


public Plugin myinfo =
{
    name = "L4D2 Archipelago Bridge",
    author = "Archipelago",
    description = "Bridges L4D2 game events to the external Archipelago client.",
    version = "0.3.6",
    url = ""
};

public void OnPluginStart()
{
    // Modules
    Bridge_Init();
    Deathlink_Init();
    Protocol_Init();
    Route_Init();
    Inventory_Init();
    Melee_Init();
    Items_Init();
    Survivors_Init();
    Survival_Init();
    Interactables_Init();
    WeaponTiers_Init();
    Pickups_Init();
    Traps_Init();

    // Dev tool for finding interactable hooks
    // Discovery_Init();

    // Game events
    HookEvent("map_transition", Event_MapTransition, EventHookMode_PostNoCopy);
    HookEvent("finale_win", Event_FinaleWin, EventHookMode_PostNoCopy);
    HookEvent("finale_start", Event_FinaleStart, EventHookMode_PostNoCopy);
    HookEvent("gauntlet_finale_start", Event_FinaleStart, EventHookMode_PostNoCopy);
    HookEvent("player_spawn", Event_PlayerSpawn, EventHookMode_Post);
    HookEvent("player_incapacitated", Event_PlayerIncapacitated, EventHookMode_Post);
    HookEvent("player_death", Event_PlayerDeath, EventHookMode_Post);
    HookEvent("mission_lost", Event_MissionLost, EventHookMode_PostNoCopy);
    HookEvent("revive_success", Event_ReviveSuccess, EventHookMode_Post);
    HookEvent("player_team", Event_PlayerTeam, EventHookMode_Post);
    HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);
    HookEvent("round_end", Event_RoundEnd, EventHookMode_PostNoCopy);
    HookEvent("survival_round_start", Event_SurvivalRoundStart, EventHookMode_PostNoCopy);

    // Commands
    RegAdminCmd("sm_ap_bridge", Command_BridgeStatus, ADMFLAG_GENERIC, "Show Archipelago bridge status.");
    RegConsoleCmd("sm_ap", Command_OpenMenu, "Open the Archipelago menu.");
    RegConsoleCmd("sm_appickup", Command_PkBring, "Summon an Archipelago pickup carrier to you.");
    RegConsoleCmd("sm_ap_deathlink_bind", Command_DeathlinkBind,
        "Bind Deathlink to this player's SteamID64.");
    RegConsoleCmd("sm_ap_deathlink_cause", Command_DeathlinkCause,
        "Show last recorded death cause for debugging.");

    // Counter-Strike weapons (MP5, SG552, AUG, Scout, AWP) only spawn with this
    ConVar csmWeapons = FindConVar("sv_unlock_csm_weapons");
    if (csmWeapons != null)
    {
        csmWeapons.SetInt(1, true, false);
    }

    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
        {
            continue;
        }
        WeaponTiers_HookClient(client);

        char auth[64];
        if (IsClientAuthorized(client) && GetClientAuthId(client, AuthId_Engine, auth, sizeof(auth)))
        {
            Survivors_OnClientAuthorized(client, auth);
        }
    }

    Bridge_Start();
}

public void OnPluginEnd()
{
    Survivors_OnPluginEnd();
    Pickups_OnPluginEnd();
    Bridge_Close();
}

public void OnMapStart()
{
    Deathlink_OnMapStart();
    Players_OnMapStart();
    Route_OnMapStart();
    Items_OnMapStart();
    Interactables_OnMapStart();
    Survivors_OnMapStart();
    Survival_OnMapStart();
    WeaponTiers_OnMapStart();
    Melee_OnMapStart();
    Pickups_OnMapStart();
    Traps_OnMapStart();
    Inventory_OnMapStart();
}

public void OnMapEnd()
{
    Deathlink_OnMapEnd();
    Route_OnMapEnd();
    Survivors_OnMapEnd();
    Pickups_OnMapEnd();
    Traps_OnMapEnd();
}

public void OnClientAuthorized(int client, const char[] auth)
{
    Survivors_OnClientAuthorized(client, auth);
    Inventory_OnClientAuthorized(client);
}

public void OnClientPutInServer(int client)
{
    Survivors_OnClientPutInServer();
    WeaponTiers_HookClient(client);
}

public void OnClientDisconnect(int client)
{
    Survivors_OnClientDisconnect(client);
    Inventory_OnClientDisconnect(client);
    Route_ReportGameState();
}

public void OnEntityDestroyed(int entity)
{
    Pickups_OnEntityDestroyed(entity);
}

public void Event_MapTransition(Event event, const char[] name, bool dontBroadcast)
{
    Survivors_OnTransition();
    Traps_OnTransition();
    Route_OnMapTransition();
}

public void Event_FinaleWin(Event event, const char[] name, bool dontBroadcast)
{
    Survivors_OnTransition();
    Traps_OnTransition();
    Route_OnFinaleWin();
}

public void Event_FinaleStart(Event event, const char[] name, bool dontBroadcast)
{
    Pickups_OnFinaleStart();
}

public void Event_PlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (!Players_IsSurvivor(client))
    {
        return;
    }

    Survivors_OnPlayerSpawn(client);
    WeaponTiers_OnPlayerSpawn(client);
    Deathlink_OnTargetRestored(client);
    Players_OnPlayerSpawn(client);
    Route_ReportGameState();
}

public void Event_PlayerIncapacitated(Event event, const char[] name, bool dontBroadcast)
{
    Deathlink_OnPlayerIncapacitated(event, GetClientOfUserId(event.GetInt("userid")));
}

public void Event_PlayerDeath(Event event, const char[] name, bool dontBroadcast)
{
    int victim = GetClientOfUserId(event.GetInt("userid"));
    Deathlink_OnPlayerDeath(event, victim);
    Players_OnPlayerDeath(victim);
}

public void Event_RoundEnd(Event event, const char[] name, bool dontBroadcast)
{
    Survival_OnRoundEnd();
}

public void Event_MissionLost(Event event, const char[] name, bool dontBroadcast)
{
    Deathlink_OnMissionLost();
}

public void Event_ReviveSuccess(Event event, const char[] name, bool dontBroadcast)
{
    Deathlink_OnTargetRestored(GetClientOfUserId(event.GetInt("subject")));
}

public void Event_PlayerTeam(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (client > 0 && client <= MaxClients)
    {
        Survivors_OnPlayerTeamChanged();
        Route_ReportGameState();
    }
}

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
    Survivors_OnRoundStart();
    Interactables_OnRoundStart();
    WeaponTiers_EnforceAll();
}

public void Event_SurvivalRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    Survival_OnRoundStart();
}
