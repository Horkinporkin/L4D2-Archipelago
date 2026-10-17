#if defined _ap_items_included
    #endinput
#endif
#define _ap_items_included

// GIVE_ITEM rewards, maps AP item name -> in-game effect. Rewards are queued in case of transitions

#define AP_GIVE_FLUSH_DELAY       2.5
#define AP_ITEM_AMMO_PILE         "Ammo Pile"
#define AP_AMMO_PILE_MODEL        "models/props/terror/ammo_stack.mdl"
#define AP_SPAWN_MELEE_DELAY      2.0
#define AP_SPAWN_MELEE_TYPE       "baseball_bat"
#define AP_DROP_FORWARD_DISTANCE  48.0   // how far in front of the survivor a drop lands
#define AP_DROP_EYE_HEIGHT        20.0   // how far above their feet the ground trace starts

ArrayList g_pendingGiveItems;          // item names waiting to be given
ArrayList g_pendingGiveItemKeys;       // matching AP receipt keys
bool g_giveFlushPending;
StringMap g_upgradePickups;            // AP item name -> upgrade pickup classname

void Items_Init()
{
    g_pendingGiveItems = new ArrayList(ByteCountToCells(AP_NAME_LEN));
    g_pendingGiveItemKeys = new ArrayList(ByteCountToCells(AP_ITEM_KEY_LEN));
    g_weaponClassnames = new StringMap();

    // Healing items
    g_weaponClassnames.SetString("First Aid Kit", "weapon_first_aid_kit");
    g_weaponClassnames.SetString("Defibrillator", "weapon_defibrillator");
    g_weaponClassnames.SetString("Pain Pills", "weapon_pain_pills");
    g_weaponClassnames.SetString("Adrenaline", "weapon_adrenaline");

    // Tier 1 weapons
    g_weaponClassnames.SetString("Pistol", "weapon_pistol");
    g_weaponClassnames.SetString("Magnum", "weapon_pistol_magnum");
    g_weaponClassnames.SetString("Dual Pistols", "weapon_pistol");

    // Tier 2 weapons
    g_weaponClassnames.SetString("SMG", "weapon_smg");
    g_weaponClassnames.SetString("Silenced SMG", "weapon_smg_silenced");
    g_weaponClassnames.SetString("MP5", "weapon_smg_mp5");
    g_weaponClassnames.SetString("Pump Shotgun", "weapon_pumpshotgun");
    g_weaponClassnames.SetString("Chrome Shotgun", "weapon_shotgun_chrome");

    // Tier 3 weapons
    g_weaponClassnames.SetString("Tactical Shotgun", "weapon_autoshotgun");
    g_weaponClassnames.SetString("M16", "weapon_rifle");
    g_weaponClassnames.SetString("SG552", "weapon_rifle_sg552");
    g_weaponClassnames.SetString("AUG", "weapon_rifle_aug");
    g_weaponClassnames.SetString("Hunting Rifle", "weapon_hunting_rifle");
    g_weaponClassnames.SetString("SCAR", "weapon_rifle_desert");
    g_weaponClassnames.SetString("Scout", "weapon_sniper_scout");

    // Tier 4 weapons
    g_weaponClassnames.SetString("AK-47", "weapon_rifle_ak47");
    g_weaponClassnames.SetString("Spas-12", "weapon_shotgun_spas");
    g_weaponClassnames.SetString("Auto Sniper", "weapon_sniper_military");
    g_weaponClassnames.SetString("AWP", "weapon_sniper_awp");

    // Heavy weapons
    g_weaponClassnames.SetString("Grenade Launcher", "weapon_grenade_launcher");
    g_weaponClassnames.SetString("M60", "weapon_rifle_m60");

    g_weaponClassnames.SetString("Chainsaw", "weapon_chainsaw");

    // Grenades and carriable world items
    g_weaponClassnames.SetString("Molotov", "weapon_molotov");
    g_weaponClassnames.SetString("Pipe Bomb", "weapon_pipe_bomb");
    g_weaponClassnames.SetString("Bile Bomb", "weapon_vomitjar");
    g_weaponClassnames.SetString("Gas Can", "weapon_gascan");
    g_weaponClassnames.SetString("Oxygen Tank", "weapon_oxygentank");
    g_weaponClassnames.SetString("Propane Tank", "weapon_propanetank");
    g_weaponClassnames.SetString("Fireworks", "weapon_fireworkcrate");
    g_weaponClassnames.SetString("Gnome Chompski", "weapon_gnome");

    // Upgrades
    g_upgradePickups = new StringMap();
    g_upgradePickups.SetString("Laser Sights", "upgrade_laser_sight");
    g_upgradePickups.SetString("Incendiary Ammo", "weapon_upgradepack_incendiary");
    g_upgradePickups.SetString("Explosive Ammo", "weapon_upgradepack_explosive");

    // Melee weapons are registered by Melee_Init (ap_melee.inc)
}

void Items_OnMapStart()
{
    g_giveFlushPending = false;
    // Items_SpawnAmmoPile sets this model before spawning, which requires it to be precached
    PrecacheModel(AP_AMMO_PILE_MODEL, true);
}

void Items_OnNewSession()
{
    g_pendingGiveItems.Clear();
    g_pendingGiveItemKeys.Clear();
}

bool Items_IsKnownGiveItem(const char[] itemName)
{
    return g_weaponClassnames.ContainsKey(itemName)
        || g_meleeWeaponTypes.ContainsKey(itemName)
        || g_upgradePickups.ContainsKey(itemName)
        || StrEqual(itemName, AP_ITEM_AMMO_PILE, false);
}

void Items_GiveItem(const char[] itemName, const char[] itemKey)
{
    if (!Items_IsKnownGiveItem(itemName))
    {
        LogError("[AP] unknown GIVE_ITEM '%s'", itemName);
        return;
    }

    if (!g_mapSettled && !Survival_IsInSurvivalMode())
    {
        Items_QueueGiveItem(itemName, itemKey);
        LogMessage("[AP] map is not fully loaded, '%s' waiting to be received.", itemName);
        return;
    }

    int client = Players_GetFirstSurvivor();
    if (client == -1)
    {
        Items_QueueGiveItem(itemName, itemKey);
        Items_ScheduleFlush();
        LogMessage("[AP] no survivor available for '%s', waiting for spawn.", itemName);
        return;
    }

    if (Items_GiveItemTo(client, itemName))
    {
        ItemReceipts_Commit(itemKey);
        return;
    }

    Items_QueueGiveItem(itemName, itemKey);
    Items_ScheduleFlush();
}

void Items_QueueGiveItem(const char[] itemName, const char[] itemKey)
{
    g_pendingGiveItems.PushString(itemName);
    g_pendingGiveItemKeys.PushString(itemKey);
}

bool Items_GiveItemTo(int client, const char[] itemName)
{
    if (!Players_IsSurvivor(client, true))
    {
        return false;
    }
    if (StrEqual(itemName, AP_ITEM_AMMO_PILE, false))
    {
        return Items_SpawnAmmoPile(client);
    }

    char classname[AP_WEAPON_CLASS_LEN];
    if (g_upgradePickups.GetString(itemName, classname, sizeof(classname)))
    {
        return Items_SpawnWorldPickup(client, classname);
    }

    char meleeScript[AP_NAME_LEN];
    if (g_meleeWeaponTypes.GetString(itemName, meleeScript, sizeof(meleeScript)))
    {
        if (Items_IsInventorySlotEmpty(client, AP_SLOT_SECONDARY) && Items_GiveMeleeWeapon(client, meleeScript))
        {
            return true;
        }
        return Items_SpawnPickup(client, Melee_CreateWeapon(meleeScript));
    }

    if (!g_weaponClassnames.GetString(itemName, classname, sizeof(classname)))
    {
        LogError("[AP] no in-game handle for item '%s'.", itemName);
        return false;
    }

    if (Items_IsCarriableWorldItem(itemName) || !Items_IsInventorySlotEmpty(client, Items_GetInventorySlot(classname)))
    {
        return Items_SpawnWorldPickup(client, classname);
    }

    int entity = GivePlayerItem(client, classname);
    if (entity > MaxClients && IsValidEntity(entity))
    {
        return true;
    }
    return Items_SpawnWorldPickup(client, classname);
}

int Items_GetInventorySlot(const char[] classname)
{
    if (StrEqual(classname, "weapon_first_aid_kit")
        || StrEqual(classname, "weapon_defibrillator"))
    {
        return AP_SLOT_HEALTH;
    }
    if (StrEqual(classname, "weapon_pain_pills")
        || StrEqual(classname, "weapon_adrenaline"))
    {
        return AP_SLOT_TEMP_HEALTH;
    }
    if (StrEqual(classname, "weapon_molotov")
        || StrEqual(classname, "weapon_pipe_bomb")
        || StrEqual(classname, "weapon_vomitjar"))
    {
        return AP_SLOT_THROWABLE;
    }
    if (StrEqual(classname, "weapon_pistol")
        || StrEqual(classname, "weapon_pistol_magnum"))
    {
        return AP_SLOT_SECONDARY;
    }
    return AP_SLOT_PRIMARY;
}

bool Items_IsInventorySlotEmpty(int client, int slot)
{
    int item = GetPlayerWeaponSlot(client, slot);
    return item == -1 || !IsValidEntity(item);
}

// Items survivors carry in their hands instead of an inventory slot
bool Items_IsCarriableWorldItem(const char[] itemName)
{
    return StrEqual(itemName, "Gas Can", false)
        || StrEqual(itemName, "Oxygen Tank", false)
        || StrEqual(itemName, "Propane Tank", false)
        || StrEqual(itemName, "Fireworks", false)
        || StrEqual(itemName, "Gnome Chompski", false);
}

bool Items_GiveMeleeWeapon(int client, const char[] meleeScript)
{
    int weapon = Melee_CreateWeapon(meleeScript);
    if (weapon == -1 || !Items_DispatchSpawn(weapon))
    {
        return false;
    }
    EquipPlayerWeapon(client, weapon);
    return true;
}

// Gives empty-handed survivors a melee weapon
void Items_GiveMeleeIfUnarmed(int client)
{
    if (!Players_IsSurvivor(client, true)
        || !Items_IsInventorySlotEmpty(client, AP_SLOT_PRIMARY)
        || !Items_IsInventorySlotEmpty(client, AP_SLOT_SECONDARY))
    {
        return;
    }

    if (Items_GiveMeleeWeapon(client, AP_SPAWN_MELEE_TYPE))
    {
        LogMessage("[AP] gave %N a melee weapon since they had none.", client);
    }
}

public Action Timer_GiveSpawnMelee(Handle timer, any userId)
{
    Items_GiveMeleeIfUnarmed(GetClientOfUserId(userId));
    return Plugin_Stop;
}

public bool Items_TraceFilterIgnorePlayers(int entity, int contentsMask, any data)
{
    return entity == 0 || entity > MaxClients;
}

// In front of the survivor, dropped to the ground below that point
void Items_GetGroundDropPlacement(int client, float origin[3], float angles[3])
{
    Players_GetPointInFront(client, AP_DROP_FORWARD_DISTANCE, AP_DROP_EYE_HEIGHT, origin, angles);

    float start[3], end[3];
    start[0] = end[0] = origin[0];
    start[1] = end[1] = origin[1];
    start[2] = origin[2] + 32.0;
    end[2] = origin[2] - 2048.0;

    Handle trace = TR_TraceRayFilterEx(start, end, MASK_SOLID, RayType_EndPoint,
        Items_TraceFilterIgnorePlayers, 0);
    if (TR_DidHit(trace) && !TR_StartSolid(trace))
    {
        TR_GetEndPosition(origin, trace);
        origin[2] += 2.0;
    }
    else
    {
        origin[2] -= 20.0;
    }
    delete trace;
}

// Spawns an entity, removing it again if it does not survive spawning
bool Items_DispatchSpawn(int entity)
{
    if (DispatchSpawn(entity) && IsValidEntity(entity))
    {
        return true;
    }
    if (IsValidEntity(entity))
    {
        RemoveEntity(entity);
    }
    return false;
}

// Spawns a created (not yet spawned) entity on the ground in front of the player
bool Items_SpawnPickup(int client, int entity)
{
    if (entity == -1)
    {
        return false;
    }

    char classname[AP_WEAPON_CLASS_LEN];
    GetEntityClassname(entity, classname, sizeof(classname));

    float origin[3], angles[3];
    Items_GetGroundDropPlacement(client, origin, angles);
    DispatchKeyValueVector(entity, "origin", origin);
    DispatchKeyValueVector(entity, "angles", angles);
    if (!Items_DispatchSpawn(entity))
    {
        LogError("[AP] entity %s failed to spawn.", classname);
        return false;
    }

    ActivateEntity(entity);
    TeleportEntity(entity, origin, angles, NULL_VECTOR);
    return true;
}

bool Items_SpawnWorldPickup(int client, const char[] classname)
{
    int entity = CreateEntityByName(classname);
    if (entity == -1)
    {
        LogError("[AP] failed to create entity %s", classname);
        return false;
    }
    return Items_SpawnPickup(client, entity);
}

bool Items_SpawnAmmoPile(int client)
{
    int ammoPile = CreateEntityByName("weapon_ammo_spawn");
    if (ammoPile == -1)
    {
        LogError("[AP] failed to create weapon_ammo_spawn.");
        return false;
    }
    DispatchKeyValue(ammoPile, "spawnflags", "2");
    SetEntityModel(ammoPile, AP_AMMO_PILE_MODEL);
    return Items_SpawnPickup(client, ammoPile);
}

void Items_ScheduleFlush()
{
    if (g_pendingGiveItems.Length == 0 || g_giveFlushPending || !g_mapSettled)
    {
        return;
    }
    g_giveFlushPending = true;
    CreateTimer(AP_GIVE_FLUSH_DELAY, Timer_FlushPendingItems, _, TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_FlushPendingItems(Handle timer)
{
    g_giveFlushPending = false;
    if (!g_mapSettled)
    {
        return Plugin_Stop;
    }
    int client = Players_GetFirstSurvivor();
    if (client == -1)
    {
        Items_ScheduleFlush();
        return Plugin_Stop;
    }

    while (g_pendingGiveItems.Length > 0)
    {
        char item[AP_NAME_LEN];
        char itemKey[AP_ITEM_KEY_LEN];
        g_pendingGiveItems.GetString(0, item, sizeof(item));
        g_pendingGiveItemKeys.GetString(0, itemKey, sizeof(itemKey));
        if (!Items_GiveItemTo(client, item))
        {
            Items_ScheduleFlush();
            return Plugin_Stop;
        }

        ItemReceipts_Commit(itemKey);
        g_pendingGiveItems.Erase(0);
        g_pendingGiveItemKeys.Erase(0);
    }
    return Plugin_Stop;
}
