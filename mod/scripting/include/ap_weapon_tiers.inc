#if defined _ap_weapon_tiers_included
    #endinput
#endif
#define _ap_weapon_tiers_included

// Progressive Weapon Tiers - blocks weapons above current tier.
// Survival mode bypasses all checks.

#define AP_TIER_DENIED_TEXT "You haven't unlocked this weapon tier yet."

bool g_weaponTiersEnabled;
int g_currentWeaponTier;
int g_maxWeaponTier;
StringMap g_classnameTier;
StringMap g_meleeTierByType;
Handle g_tierHudSync;

void WeaponTiers_Init()
{
    g_classnameTier = new StringMap();
    g_meleeTierByType = new StringMap();
    g_tierHudSync = CreateHudSynchronizer();
}

void WeaponTiers_OnMapStart()
{
    CreateTimer(1.0, Timer_EnforceAll, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

void WeaponTiers_OnSyncComplete()
{
    WeaponTiers_EnforceAll();
}

void WeaponTiers_OnNewSession()
{
    g_currentWeaponTier = 0;
}

void WeaponTiers_HandleConfigure(const char[] payload)
{
    g_classnameTier.Clear();
    g_meleeTierByType.Clear();

    char buffer[16];
    g_weaponTiersEnabled = JSON_GetField(payload, "enabled", buffer, sizeof(buffer)) && StrEqual(buffer, "true", false);
    if (JSON_GetField(payload, "max_tier", buffer, sizeof(buffer)))
    {
        g_maxWeaponTier = StringToInt(buffer);
    }

    static char tiers[4096];
    if (!JSON_GetField(payload, "tiers", tiers, sizeof(tiers)))
    {
        return;
    }

    ArrayList tierObjects = new ArrayList(ByteCountToCells(256));
    JSON_SplitArrayObjects(tiers, tierObjects);
    for (int i = 0; i < tierObjects.Length; i++)
    {
        char tierObject[256];
        char item[AP_NAME_LEN];
        char tierText[16];
        tierObjects.GetString(i, tierObject, sizeof(tierObject));
        if (!JSON_GetField(tierObject, "item", item, sizeof(item))
            || !JSON_GetField(tierObject, "tier", tierText, sizeof(tierText)))
        {
            continue;
        }

        int tier = StringToInt(tierText);
        char classname[AP_WEAPON_CLASS_LEN];
        char meleeScript[AP_NAME_LEN];
        if (g_weaponClassnames.GetString(item, classname, sizeof(classname)))
        {
            g_classnameTier.SetValue(classname, tier);
        }
        else if (g_meleeWeaponTypes.GetString(item, meleeScript, sizeof(meleeScript)))
        {
            g_meleeTierByType.SetValue(meleeScript, tier);
        }
    }
    delete tierObjects;
}

void WeaponTiers_HandleUnlock()
{
    if (g_currentWeaponTier < g_maxWeaponTier)
    {
        g_currentWeaponTier++;
    }
}

bool WeaponTiers_ShouldEnforce()
{
    return g_weaponTiersEnabled && g_stateSynced && !Survival_IsInSurvivalMode();
}

bool WeaponTiers_IsClassAllowed(const char[] classname, const char[] meleeScript = "")
{
    if (Survival_IsInSurvivalMode())
    {
        return true;
    }

    int tier;
    if (StrEqual(classname, "weapon_melee"))
    {
        if (meleeScript[0] == '\0' || !g_meleeTierByType.GetValue(meleeScript, tier))
        {
            return true;
        }
    }
    else if (!g_classnameTier.GetValue(classname, tier))
    {
        return true;
    }
    return tier <= g_currentWeaponTier;
}

bool WeaponTiers_IsWeaponAllowed(int weapon)
{
    if (weapon <= MaxClients || !IsValidEntity(weapon) || Survival_IsInSurvivalMode())
    {
        return true;
    }

    char classname[AP_WEAPON_CLASS_LEN];
    char meleeScript[AP_NAME_LEN];
    GetEntityClassname(weapon, classname, sizeof(classname));
    if (StrEqual(classname, "weapon_melee") && HasEntProp(weapon, Prop_Data, "m_strMapSetScriptName"))
    {
        GetEntPropString(weapon, Prop_Data, "m_strMapSetScriptName", meleeScript, sizeof(meleeScript));
    }
    return WeaponTiers_IsClassAllowed(classname, meleeScript);
}

void WeaponTiers_HookClient(int client)
{
    if (IsFakeClient(client))
    {
        return;
    }

    SDKHook(client, SDKHook_WeaponCanUse, Hook_TierCanUse);
    SDKHook(client, SDKHook_WeaponCanSwitchTo, Hook_TierCanUse);
    SDKHook(client, SDKHook_WeaponSwitch, Hook_TierSwitch);
}

void WeaponTiers_OnPlayerSpawn(int client)
{
    if (!WeaponTiers_ShouldEnforce() || !Players_IsSurvivor(client, true) || IsFakeClient(client))
    {
        return;
    }

    CreateTimer(0.2, Timer_EnforceClient, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
}

void WeaponTiers_EnforceAll()
{
    if (!WeaponTiers_ShouldEnforce())
    {
        return;
    }

    for (int client = 1; client <= MaxClients; client++)
    {
        if (Players_IsSurvivor(client, true) && !IsFakeClient(client))
        {
            WeaponTiers_EnforceClient(client);
        }
    }
}

void WeaponTiers_EnforceClient(int client)
{
    for (int slot = 0; slot < AP_WEAPON_SLOTS; slot++)
    {
        int weapon = GetPlayerWeaponSlot(client, slot);
        if (weapon == -1 || WeaponTiers_IsWeaponAllowed(weapon))
        {
            continue;
        }

        if (slot == AP_SLOT_SECONDARY && WeaponTiers_IsDualPistols(weapon))
        {
            RemovePlayerItem(client, weapon);
            RemoveEntity(weapon);
        }
        else
        {
            SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR, true);
        }
        WeaponTiers_ShowHint(client);
    }
}

bool WeaponTiers_IsDualPistols(int weapon)
{
    char classname[AP_WEAPON_CLASS_LEN];
    GetEntityClassname(weapon, classname, sizeof(classname));
    return StrEqual(classname, "weapon_pistol")
        && HasEntProp(weapon, Prop_Send, "m_isDualWielding")
        && GetEntProp(weapon, Prop_Send, "m_isDualWielding") != 0;
}

public Action Timer_EnforceClient(Handle timer, any userId)
{
    int client = GetClientOfUserId(userId);
    if (WeaponTiers_ShouldEnforce() && Players_IsSurvivor(client, true) && !IsFakeClient(client))
    {
        WeaponTiers_EnforceClient(client);
    }
    return Plugin_Stop;
}

public Action Timer_EnforceAll(Handle timer)
{
    WeaponTiers_EnforceAll();
    return Plugin_Continue;
}

public Action Hook_TierCanUse(int client, int weapon)
{
    if (!g_weaponTiersEnabled || Survival_IsInSurvivalMode() || WeaponTiers_IsWeaponAllowed(weapon))
    {
        return Plugin_Continue;
    }

    // If it's already equipped, drop it
    if (Players_IsSurvivor(client, true) && GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon") == weapon)
    {
        SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR, true);
    }

    WeaponTiers_ShowHint(client);
    return Plugin_Handled;
}

public Action Hook_TierSwitch(int client, int weapon)
{
    if (!g_weaponTiersEnabled || Survival_IsInSurvivalMode() || WeaponTiers_IsWeaponAllowed(weapon))
    {
        return Plugin_Continue;
    }

    SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR, true);
    WeaponTiers_ShowHint(client);
    return Plugin_Handled;
}

void WeaponTiers_ShowHint(int client)
{
    if (g_tierHudSync == null)
    {
        PrintHintText(client, AP_TIER_DENIED_TEXT);
        return;
    }

    SetHudTextParams(-1.0, 0.75, 2.5, 255, 255, 255, 255, 0, 0.0, 0.1, 0.2);
    ShowSyncHudText(client, g_tierHudSync, AP_TIER_DENIED_TEXT);
}
