#if defined _ap_inventory_included
    #endinput
#endif
#define _ap_inventory_included

// Saves and restores loadout across map transitions
// Skips saving in Survival mode to avoid carrying locked weapons into Coop

// Loadout snapshots
char g_savedWeaponClass[MAXPLAYERS + 1][AP_WEAPON_SLOTS][AP_WEAPON_CLASS_LEN];
char g_savedMeleeType[MAXPLAYERS + 1][AP_WEAPON_SLOTS][AP_NAME_LEN];
int g_savedClip[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
int g_savedAmmoType[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
int g_savedReserveAmmo[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
int g_savedUpgradeBits[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
int g_savedUpgradedAmmoLoaded[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
bool g_savedDualPistols[MAXPLAYERS + 1][AP_WEAPON_SLOTS];
int g_savedActiveSlot[MAXPLAYERS + 1];
bool g_inventoryRestorePending[MAXPLAYERS + 1];
bool g_inventoryRouteVerified;                  // current AP session matches saved gear
char g_inventoryFile[PLATFORM_MAX_PATH];        // per-SteamID loadouts

void Inventory_Init()
{
    BuildPath(Path_SM, g_inventoryFile, sizeof(g_inventoryFile), "data/l4d2_ap_inventory.txt");
}

void Inventory_OnMapStart()
{
    CreateTimer(1.0, Timer_RestorePendingInventories, _, TIMER_FLAG_NO_MAPCHANGE);
}

void Inventory_OnClientAuthorized(int client)
{
    if (client <= 0 || client > MaxClients || IsFakeClient(client))
    {
        return;
    }

    Inventory_ResetClientState(client);
    Inventory_LoadFromDisk(client);
}

void Inventory_OnClientDisconnect(int client)
{
    if (client <= 0 || client > MaxClients)
    {
        return;
    }

    if (Players_IsSurvivor(client, true) && !IsFakeClient(client))
    {
        Inventory_SaveClient(client);
    }
    Inventory_ResetClientState(client);
}

void Inventory_SaveAll()
{
    for (int client = 1; client <= MaxClients; client++)
    {
        if (Players_IsSurvivor(client, true) && !IsFakeClient(client))
        {
            Inventory_SaveClient(client);
        }
    }
}

void Inventory_SaveClient(int client)
{
    if (IsFakeClient(client) || Survival_IsInSurvivalMode())
    {
        return;
    }

    Inventory_ResetClientState(client);
    int activeWeapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        int weapon = GetPlayerWeaponSlot(client, slot);
        if (weapon == -1 || !IsValidEntity(weapon))
        {
            continue;
        }

        GetEntityClassname(weapon, g_savedWeaponClass[client][slot], AP_WEAPON_CLASS_LEN);

        if (StrEqual(g_savedWeaponClass[client][slot], "weapon_melee")
            && HasEntProp(weapon, Prop_Data, "m_strMapSetScriptName"))
        {
            GetEntPropString(weapon, Prop_Data, "m_strMapSetScriptName", g_savedMeleeType[client][slot], AP_NAME_LEN);
        }
        if (HasEntProp(weapon, Prop_Send, "m_iClip1"))
        {
            g_savedClip[client][slot] = GetEntProp(weapon, Prop_Send, "m_iClip1");
        }
        if (HasEntProp(weapon, Prop_Send, "m_upgradeBitVec"))
        {
            g_savedUpgradeBits[client][slot] = GetEntProp(weapon, Prop_Send, "m_upgradeBitVec");
        }
        if (HasEntProp(weapon, Prop_Send, "m_nUpgradedPrimaryAmmoLoaded"))
        {
            g_savedUpgradedAmmoLoaded[client][slot] = GetEntProp(weapon, Prop_Send, "m_nUpgradedPrimaryAmmoLoaded");
        }
        if (HasEntProp(weapon, Prop_Send, "m_isDualWielding"))
        {
            g_savedDualPistols[client][slot] = GetEntProp(weapon, Prop_Send, "m_isDualWielding") != 0;
        }
        if (HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType"))
        {
            int ammoType = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
            if (ammoType >= 0 && ammoType < 64 && HasEntProp(client, Prop_Send, "m_iAmmo"))
            {
                g_savedAmmoType[client][slot] = ammoType;
                g_savedReserveAmmo[client][slot] = GetEntProp(client, Prop_Send, "m_iAmmo", 4, ammoType);
            }
        }
        if (weapon == activeWeapon)
        {
            g_savedActiveSlot[client] = slot;
        }
    }

    g_inventoryRestorePending[client] = true;
    Inventory_PersistToDisk(client);
}

void Inventory_PersistToDisk(int client)
{
    char steamId[32];
    if (!GetClientAuthId(client, AuthId_Steam2, steamId, sizeof(steamId)) || strncmp(steamId, "STEAM_", 6) != 0)
    {
        return;
    }

    KeyValues kv = new KeyValues("APInventory");
    kv.ImportFromFile(g_inventoryFile);
    if (!kv.JumpToKey(steamId, true))
    {
        delete kv;
        return;
    }

    kv.SetNum("active_slot", g_savedActiveSlot[client]);

    char key[32];
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        FormatEx(key, sizeof(key), "class%d", slot);
        kv.SetString(key, g_savedWeaponClass[client][slot]);

        FormatEx(key, sizeof(key), "melee%d", slot);
        kv.SetString(key, g_savedMeleeType[client][slot]);

        FormatEx(key, sizeof(key), "clip%d", slot);
        kv.SetNum(key, g_savedClip[client][slot]);

        FormatEx(key, sizeof(key), "ammotype%d", slot);
        kv.SetNum(key, g_savedAmmoType[client][slot]);

        FormatEx(key, sizeof(key), "reserve%d", slot);
        kv.SetNum(key, g_savedReserveAmmo[client][slot]);

        FormatEx(key, sizeof(key), "upgrades%d", slot);
        kv.SetNum(key, g_savedUpgradeBits[client][slot]);

        FormatEx(key, sizeof(key), "upgradeammo%d", slot);
        kv.SetNum(key, g_savedUpgradedAmmoLoaded[client][slot]);

        FormatEx(key, sizeof(key), "dual%d", slot);
        kv.SetNum(key, g_savedDualPistols[client][slot] ? 1 : 0);
    }

    kv.Rewind();
    kv.ExportToFile(g_inventoryFile);
    delete kv;
}

void Inventory_ResetClientState(int client)
{
    if (client <= 0 || client > MaxClients)
    {
        return;
    }

    g_savedActiveSlot[client] = -1;
    g_inventoryRestorePending[client] = false;
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        g_savedWeaponClass[client][slot][0] = '\0';
        g_savedMeleeType[client][slot][0] = '\0';
        g_savedClip[client][slot] = -1;
        g_savedAmmoType[client][slot] = -1;
        g_savedReserveAmmo[client][slot] = 0;
        g_savedUpgradeBits[client][slot] = 0;
        g_savedUpgradedAmmoLoaded[client][slot] = 0;
        g_savedDualPistols[client][slot] = false;
    }
}

void Inventory_LoadFromDisk(int client)
{
    char steamId[32];
    if (!GetClientAuthId(client, AuthId_Steam2, steamId, sizeof(steamId)) || strncmp(steamId, "STEAM_", 6) != 0)
    {
        return;
    }

    KeyValues kv = new KeyValues("APInventory");
    if (!kv.ImportFromFile(g_inventoryFile) || !kv.JumpToKey(steamId))
    {
        delete kv;
        return;
    }

    g_savedActiveSlot[client] = kv.GetNum("active_slot", -1);

    char key[32];
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        FormatEx(key, sizeof(key), "class%d", slot);
        kv.GetString(key, g_savedWeaponClass[client][slot], AP_WEAPON_CLASS_LEN);

        FormatEx(key, sizeof(key), "melee%d", slot);
        kv.GetString(key, g_savedMeleeType[client][slot], AP_NAME_LEN);

        FormatEx(key, sizeof(key), "clip%d", slot);
        g_savedClip[client][slot] = kv.GetNum(key, -1);

        FormatEx(key, sizeof(key), "ammotype%d", slot);
        g_savedAmmoType[client][slot] = kv.GetNum(key, -1);

        FormatEx(key, sizeof(key), "reserve%d", slot);
        g_savedReserveAmmo[client][slot] = kv.GetNum(key, 0);

        FormatEx(key, sizeof(key), "upgrades%d", slot);
        g_savedUpgradeBits[client][slot] = kv.GetNum(key, 0);

        FormatEx(key, sizeof(key), "upgradeammo%d", slot);
        g_savedUpgradedAmmoLoaded[client][slot] = kv.GetNum(key, 0);

        FormatEx(key, sizeof(key), "dual%d", slot);
        g_savedDualPistols[client][slot] = kv.GetNum(key, 0) != 0;
    }
    delete kv;

    g_inventoryRestorePending[client] = true;
    Inventory_ScheduleRestore(client);
}

void Inventory_ScheduleRestore(int client)
{
    if (!g_inventoryRouteVerified || !g_inventoryRestorePending[client]
        || !IsClientInGame(client) || !IsPlayerAlive(client))
    {
        return;
    }

    RequestFrame(Frame_RestoreInventory, GetClientUserId(client));
}

public void Frame_RestoreInventory(any userId)
{
    int client = GetClientOfUserId(userId);
    if (client > 0 && client <= MaxClients)
    {
        Inventory_RestoreClient(client);
    }
}

// Saved gear only belongs to the AP run it was saved in; a different route or session discards it
void Inventory_CheckRouteFingerprint(int routeHash, const char[] sessionId)
{
    char currentHash[24];
    char storedRoute[24];
    char storedSession[128];
    IntToString(routeHash, currentHash, sizeof(currentHash));

    KeyValues kv = new KeyValues("APInventory");
    if (kv.ImportFromFile(g_inventoryFile))
    {
        kv.GetString("route", storedRoute, sizeof(storedRoute));
        kv.GetString("session", storedSession, sizeof(storedSession));
    }
    delete kv;

    bool sameRoute = StrEqual(storedRoute, currentHash);
    bool sameSession = sessionId[0] == '\0' || StrEqual(storedSession, sessionId);
    if (sameRoute && sameSession)
    {
        g_inventoryRouteVerified = true;
        Inventory_SchedulePendingRestores();
        return;
    }

    kv = new KeyValues("APInventory");
    kv.SetString("route", currentHash);
    if (sessionId[0] != '\0')
    {
        kv.SetString("session", sessionId);
    }
    kv.ExportToFile(g_inventoryFile);
    delete kv;

    for (int client = 1; client <= MaxClients; client++)
    {
        Inventory_ResetClientState(client);
    }
    g_inventoryRouteVerified = true;
}

void Inventory_SchedulePendingRestores()
{
    for (int client = 1; client <= MaxClients; client++)
    {
        if (g_inventoryRestorePending[client])
        {
            Inventory_ScheduleRestore(client);
        }
    }
}

bool Inventory_SpawnWeaponPickup(int client, int weapon)
{
    if (weapon == -1 || !IsValidEntity(weapon))
    {
        return false;
    }

    float origin[3];
    GetClientAbsOrigin(client, origin);
    TeleportEntity(weapon, origin, NULL_VECTOR, NULL_VECTOR);
    DispatchSpawn(weapon);
    if (!IsValidEntity(weapon))
    {
        return false;
    }

    AcceptEntityInput(weapon, "use", client);
    return true;
}

bool Inventory_RestoreDualPistolPair(int client, int firstPistol, int &outWeapon)
{
    if (!Inventory_SpawnWeaponPickup(client, firstPistol))
    {
        return false;
    }

    outWeapon = GetPlayerWeaponSlot(client, AP_SLOT_SECONDARY);
    if (outWeapon == -1 && IsValidEntity(firstPistol))
    {
        EquipPlayerWeapon(client, firstPistol);
        outWeapon = GetPlayerWeaponSlot(client, AP_SLOT_SECONDARY);
    }
    if (outWeapon == -1)
    {
        return false;
    }
    
    int secondPistol = CreateEntityByName("weapon_pistol");
    if (secondPistol == -1 || !Inventory_SpawnWeaponPickup(client, secondPistol))
    {
        if (secondPistol != -1 && IsValidEntity(secondPistol))
        {
            RemoveEntity(secondPistol);
        }
        return true;
    }

    int paired = GetPlayerWeaponSlot(client, AP_SLOT_SECONDARY);
    if (paired != -1)
    {
        outWeapon = paired;
    }
    return true;
}

void Inventory_RestoreClient(int client)
{
    if (!g_inventoryRouteVerified || !IsClientInGame(client) || !IsPlayerAlive(client)
        || !g_inventoryRestorePending[client])
    {
        return;
    }

    int restored[AP_WEAPON_SLOTS];
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        int current = GetPlayerWeaponSlot(client, slot);
        if (current != -1 && IsValidEntity(current))
        {
            RemovePlayerItem(client, current);
            RemoveEntity(current);
        }

        restored[slot] = -1;

        char classname[AP_WEAPON_CLASS_LEN];
        strcopy(classname, sizeof(classname), g_savedWeaponClass[client][slot]);
        // Skip empty slots and weapons above the current weapon tier
        if (classname[0] == '\0' || !WeaponTiers_IsClassAllowed(classname, g_savedMeleeType[client][slot]))
        {
            continue;
        }

        int weapon = StrEqual(classname, "weapon_melee")
            ? Melee_CreateWeapon(g_savedMeleeType[client][slot])
            : CreateEntityByName(classname);
        if (weapon == -1)
        {
            continue;
        }

        if (slot == AP_SLOT_SECONDARY && g_savedDualPistols[client][slot] && StrEqual(classname, "weapon_pistol"))
        {
            int restoredPistol = -1;
            if (!Inventory_RestoreDualPistolPair(client, weapon, restoredPistol) || restoredPistol == -1)
            {
                continue;
            }
            weapon = restoredPistol;
        }
        else
        {
            DispatchSpawn(weapon);
            if (!IsValidEntity(weapon))
            {
                continue;
            }
            EquipPlayerWeapon(client, weapon);
        }

        restored[slot] = weapon;

        if (g_savedClip[client][slot] >= 0 && HasEntProp(weapon, Prop_Send, "m_iClip1"))
        {
            SetEntProp(weapon, Prop_Send, "m_iClip1", g_savedClip[client][slot]);
        }
        if (HasEntProp(weapon, Prop_Send, "m_upgradeBitVec"))
        {
            SetEntProp(weapon, Prop_Send, "m_upgradeBitVec", g_savedUpgradeBits[client][slot]);
        }
        if (HasEntProp(weapon, Prop_Send, "m_nUpgradedPrimaryAmmoLoaded"))
        {
            SetEntProp(weapon, Prop_Send, "m_nUpgradedPrimaryAmmoLoaded", g_savedUpgradedAmmoLoaded[client][slot]);
        }

        int ammoType = g_savedAmmoType[client][slot];
        if (ammoType >= 0 && ammoType < 64 && HasEntProp(client, Prop_Send, "m_iAmmo"))
        {
            SetEntProp(client, Prop_Send, "m_iAmmo", g_savedReserveAmmo[client][slot], 4, ammoType);
        }
    }

    int activeSlot = g_savedActiveSlot[client];
    if (activeSlot >= 0 && activeSlot < AP_WEAPON_SLOTS && restored[activeSlot] != -1
        && IsValidEntity(restored[activeSlot]))
    {
        SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", restored[activeSlot]);
    }

    g_inventoryRestorePending[client] = false;
    Items_GiveMeleeIfUnarmed(client);
    WeaponTiers_EnforceAll();
}

public Action Timer_RestorePendingInventories(Handle timer)
{
    if (!g_inventoryRouteVerified)
    {
        return Plugin_Stop;
    }
    for (int client = 1; client <= MaxClients; client++)
    {
        if (g_inventoryRestorePending[client])
        {
            Inventory_RestoreClient(client);
        }
    }
    return Plugin_Stop;
}

public Action Timer_RestoreClientInventory(Handle timer, any userId)
{
    int client = GetClientOfUserId(userId);
    if (client > 0)
    {
        Inventory_RestoreClient(client);
    }
    return Plugin_Stop;
}
