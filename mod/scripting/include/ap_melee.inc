#if defined _ap_melee_included
    #endinput
#endif
#define _ap_melee_included

// Handles melee weapons, protects the player from the claws glitch

enum struct MeleeWeapon
{
    char item[32];      // AP item name
    char script[32];    // scripts/melee/<script>.txt
}

static const MeleeWeapon g_meleeWeapons[] =
{
    { "Fireaxe", "fireaxe" },
    { "Baseball Bat", "baseball_bat" },
    { "Cricket Bat", "cricket_bat" },
    { "Crowbar", "crowbar" },
    { "Frying Pan", "frying_pan" },
    { "Golf Club", "golfclub" },
    { "Guitar", "electric_guitar" },
    { "Katana", "katana" },
    { "Machete", "machete" },
    { "Nightstick", "tonfa" },
    { "Pitchfork", "pitchfork" },
    { "Shovel", "shovel" },
    { "Knife", "knife" }
};

bool g_meleeMapUsable[sizeof(g_meleeWeapons)];
bool g_meleeScanned;

void Melee_Init()
{
    g_meleeWeaponTypes = new StringMap();
    for (int i = 0; i < sizeof(g_meleeWeapons); i++)
    {
        g_meleeWeaponTypes.SetString(g_meleeWeapons[i].item, g_meleeWeapons[i].script);
    }
}

void Melee_OnMapStart()
{
    // The string table is complete after the map spawns its entities
    g_meleeScanned = false;
    RequestFrame(Frame_MeleeScan);
}

public void Frame_MeleeScan(any data)
{
    Melee_ScanMap();
}

void Melee_ScanMap()
{
    for (int i = 0; i < sizeof(g_meleeMapUsable); i++)
    {
        g_meleeMapUsable[i] = false;
    }

    int table = FindStringTable("MeleeWeapons");
    if (table == INVALID_STRING_TABLE)
    {
        table = FindStringTable("meleeweapons");
    }

    int count = (table != INVALID_STRING_TABLE) ? GetStringTableNumStrings(table) : 0;
    int usableCount = 0;
    char script[64];
    for (int i = 0; i < count; i++)
    {
        ReadStringTable(table, i, script, sizeof(script));
        int index = Melee_FindScript(script);
        if (index != -1 && !g_meleeMapUsable[index])
        {
            g_meleeMapUsable[index] = true;
            usableCount++;
        }
    }
    g_meleeScanned = true;
    LogMessage("[AP] %d melee script(s) usable on this map.", usableCount);
}

int Melee_FindScript(const char[] script)
{
    for (int i = 0; i < sizeof(g_meleeWeapons); i++)
    {
        if (StrEqual(script, g_meleeWeapons[i].script, false))
        {
            return i;
        }
    }
    return -1;
}

// Pick the melee script, otherwise random
bool Melee_Resolve(const char[] wanted, char[] script, int maxlen)
{
    if (!g_meleeScanned)
    {
        Melee_ScanMap();
    }

    int wantedIndex = Melee_FindScript(wanted);
    if (wantedIndex != -1 && g_meleeMapUsable[wantedIndex])
    {
        strcopy(script, maxlen, g_meleeWeapons[wantedIndex].script);
        return true;
    }

    int usable[sizeof(g_meleeWeapons)];
    int usableCount = 0;
    for (int i = 0; i < sizeof(g_meleeWeapons); i++)
    {
        if (g_meleeMapUsable[i])
        {
            usable[usableCount++] = i;
        }
    }

    if (usableCount == 0)
    {
        LogError("[AP] no usable melee on this map, skipping '%s'.", wanted);
        return false;
    }

    int pick = usable[GetRandomInt(0, usableCount - 1)];
    strcopy(script, maxlen, g_meleeWeapons[pick].script);
    LogMessage("[AP] melee '%s' isn't usable on this map; giving '%s' instead.", wanted, script);
    return true;
}

int Melee_CreateWeapon(const char[] wantedScript)
{
    char script[32];
    if (!Melee_Resolve(wantedScript, script, sizeof(script)))
    {
        return -1;
    }

    int weapon = CreateEntityByName("weapon_melee");
    if (weapon != -1)
    {
        DispatchKeyValue(weapon, "melee_script_name", script);
    }
    return weapon;
}
