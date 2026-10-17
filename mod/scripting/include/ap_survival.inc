#if defined _ap_survival_included
    #endinput
#endif
#define _ap_survival_included

// Survival mode: level access, medal checks, and travel.

#define AP_SURVIVAL_CLIENT_RETRY_TIME 1.0   // retry delay when a medal is due but no client is connected
#define AP_SURVIVAL_BRONZE_TIME 240.0
#define AP_SURVIVAL_SILVER_TIME 420.0
#define AP_SURVIVAL_GOLD_TIME 600.0
#define AP_SURVIVAL_MIN_TIME 10.0

bool g_survivalEnabled;
int g_survivalMedalRequirement;
ArrayList g_survivalLevelMaps;             // configured levels, in slot data order
StringMap g_survivalLevelNames;
StringMap g_survivalLevelCampaigns;
StringMap g_survivalUnlockedLevels;
StringMap g_survivalReportedMedals;
ArrayList g_survivalReceiptOrder;          // unlocked levels, most recently received first
bool g_survivalRoundRunning;
int g_survivalRoundSerial;                 // invalidates medal timers from a previous round
bool g_survivalNativeAvailable;

float g_survivalBronzeTime = AP_SURVIVAL_BRONZE_TIME;
float g_survivalSilverTime = AP_SURVIVAL_SILVER_TIME;
float g_survivalGoldTime = AP_SURVIVAL_GOLD_TIME;

void Survival_Init()
{
    g_survivalLevelMaps = new ArrayList(ByteCountToCells(AP_MAP_LEN));
    g_survivalLevelNames = new StringMap();
    g_survivalLevelCampaigns = new StringMap();
    g_survivalUnlockedLevels = new StringMap();
    g_survivalReportedMedals = new StringMap();
    g_survivalReceiptOrder = new ArrayList(ByteCountToCells(AP_MAP_LEN));
    g_survivalNativeAvailable = GetFeatureStatus(FeatureType_Native, "L4D_IsSurvivalMode") == FeatureStatus_Available;
}

void Survival_OnMapStart()
{
    g_survivalRoundRunning = false;

    if (Survival_IsInSurvivalMode())
    {
        g_mapSettled = true;
        Items_ScheduleFlush();
    }
}

void Survival_OnNewSession()
{
    g_survivalUnlockedLevels.Clear();
    g_survivalReportedMedals.Clear();
    g_survivalReceiptOrder.Clear();
    g_survivalRoundRunning = false;
    g_survivalRoundSerial++;
}

void Survival_Configure(const char[] payload)
{
    char buffer[16];
    g_survivalEnabled = JSON_GetField(payload, "enabled", buffer, sizeof(buffer)) && StrEqual(buffer, "true", false);

    g_survivalMedalRequirement = 0;
    if (JSON_GetField(payload, "medal", buffer, sizeof(buffer)))
    {
        g_survivalMedalRequirement = StringToInt(buffer);
    }

    g_survivalBronzeTime = Survival_ReadMedalTime(payload, "bronze_time", AP_SURVIVAL_BRONZE_TIME);
    g_survivalSilverTime = Survival_ReadMedalTime(payload, "silver_time", AP_SURVIVAL_SILVER_TIME);
    g_survivalGoldTime = Survival_ReadMedalTime(payload, "gold_time", AP_SURVIVAL_GOLD_TIME);

    g_survivalLevelMaps.Clear();
    g_survivalLevelNames.Clear();
    g_survivalLevelCampaigns.Clear();
    g_survivalUnlockedLevels.Clear();
    g_survivalReportedMedals.Clear();
    g_survivalReceiptOrder.Clear();

    static char levels[8192];
    if (!JSON_GetField(payload, "levels", levels, sizeof(levels)))
    {
        return;
    }

    ArrayList levelObjects = new ArrayList(ByteCountToCells(256));
    JSON_SplitArrayObjects(levels, levelObjects);
    for (int i = 0; i < levelObjects.Length; i++)
    {
        char levelObject[256];
        char map[AP_MAP_LEN];
        char campaign[AP_NAME_LEN];
        char name[AP_NAME_LEN];
        levelObjects.GetString(i, levelObject, sizeof(levelObject));
        if (!JSON_GetField(levelObject, "map", map, sizeof(map)))
        {
            continue;
        }
        JSON_GetField(levelObject, "campaign", campaign, sizeof(campaign));
        if (!JSON_GetField(levelObject, "name", name, sizeof(name)) || name[0] == '\0')
        {
            strcopy(name, sizeof(name), map);
        }

        g_survivalLevelMaps.PushString(map);
        g_survivalLevelNames.SetString(map, name);
        g_survivalLevelCampaigns.SetString(map, campaign);
    }
    delete levelObjects;

    LogMessage("[AP] survival: enabled=%d medal=%d times=%d/%d/%d levels=%d",
        g_survivalEnabled, g_survivalMedalRequirement,
        RoundToNearest(g_survivalBronzeTime), RoundToNearest(g_survivalSilverTime), RoundToNearest(g_survivalGoldTime),
        g_survivalLevelMaps.Length);
}

float Survival_ReadMedalTime(const char[] payload, const char[] field, float defaultTime)
{
    float time = defaultTime;
    char buffer[16];
    if (JSON_GetField(payload, field, buffer, sizeof(buffer)) && StringToInt(buffer) > 0)
    {
        time = float(StringToInt(buffer));
    }
    return time < AP_SURVIVAL_MIN_TIME ? AP_SURVIVAL_MIN_TIME : time;
}

void Survival_Unlock(const char[] map)
{
    g_survivalUnlockedLevels.SetValue(map, 1);
    ArrayList_MoveToFront(g_survivalReceiptOrder, map);
}

bool Survival_IsLevelConfigured(const char[] map)
{
    return g_survivalLevelMaps.FindString(map) != -1;
}

bool Survival_IsLevelUnlocked(const char[] map)
{
    return g_survivalUnlockedLevels.ContainsKey(map);
}

bool Survival_IsInSurvivalMode()
{
    return g_survivalNativeAvailable && L4D_IsSurvivalMode();
}

bool Survival_IsActive()
{
    return g_survivalEnabled && Survival_IsInSurvivalMode();
}

bool Survival_GetLevel(int index, char[] map, int maxlen, char[] campaign, int campaignMaxlen)
{
    if (index >= g_survivalLevelMaps.Length)
    {
        return false;
    }
    g_survivalLevelMaps.GetString(index, map, maxlen);
    campaign[0] = '\0';
    g_survivalLevelCampaigns.GetString(map, campaign, campaignMaxlen);
    return true;
}

bool Survival_CampaignHasUnlockedLevel(const char[] campaign)
{
    char map[AP_MAP_LEN];
    char levelCampaign[AP_NAME_LEN];
    for (int i = 0; Survival_GetLevel(i, map, sizeof(map), levelCampaign, sizeof(levelCampaign)); i++)
    {
        if (StrEqual(levelCampaign, campaign) && Survival_IsLevelUnlocked(map))
        {
            return true;
        }
    }
    return false;
}

bool Survival_FindNextMap(char[] map, int maxlen)
{
    // Most recently unlocked first
    for (int i = 0; i < g_survivalReceiptOrder.Length; i++)
    {
        char candidate[AP_MAP_LEN];
        g_survivalReceiptOrder.GetString(i, candidate, sizeof(candidate));
        if (Survival_IsLevelUnlocked(candidate))
        {
            strcopy(map, maxlen, candidate);
            return true;
        }
    }
    // Or any unlocked level
    for (int i = 0; i < g_survivalLevelMaps.Length; i++)
    {
        char candidate[AP_MAP_LEN];
        g_survivalLevelMaps.GetString(i, candidate, sizeof(candidate));
        if (Survival_IsLevelUnlocked(candidate))
        {
            strcopy(map, maxlen, candidate);
            return true;
        }
    }
    return false;
}

void Survival_TravelToLevel(const char[] map)
{
    if (g_survivalEnabled && Survival_IsLevelUnlocked(map))
    {
        Route_TravelToMap(map, true, true);
    }
}

float Survival_MedalTime(int medal)
{
    switch (medal)
    {
        case 1: return g_survivalBronzeTime;
        case 2: return g_survivalSilverTime;
        case 3: return g_survivalGoldTime;
    }
    return 0.0;
}

// Each medal gets a timer that triggers when its time is reached
void Survival_OnRoundStart()
{
    if (!g_survivalEnabled || g_survivalMedalRequirement <= 0)
    {
        return;
    }

    g_survivalRoundRunning = true;
    g_survivalRoundSerial++;

    for (int medal = 1; medal <= g_survivalMedalRequirement; medal++)
    {
        CreateTimer(Survival_MedalTime(medal), Timer_SurvivalMedal,
            g_survivalRoundSerial * 32 + medal, TIMER_FLAG_NO_MAPCHANGE);
    }
}

// stops the timer when a survival round ends (loss)
void Survival_OnRoundEnd()
{
    g_survivalRoundRunning = false;
    g_survivalRoundSerial++;
}

public Action Timer_SurvivalMedal(Handle timer, any data)
{
    int medal = data % 32;
    if (data / 32 != g_survivalRoundSerial || !g_survivalRoundRunning || !g_survivalEnabled)
    {
        return Plugin_Stop;
    }

    float remaining = Survival_MedalTime(medal) - (GetGameTime() - GameRules_GetPropFloat("m_flRoundStartTime"));
    if (remaining > 0.0)
    {
        CreateTimer(remaining, Timer_SurvivalMedal, data, TIMER_FLAG_NO_MAPCHANGE);
        return Plugin_Stop;
    }

    if (g_client == null)
    {
        CreateTimer(AP_SURVIVAL_CLIENT_RETRY_TIME, Timer_SurvivalMedal, data, TIMER_FLAG_NO_MAPCHANGE);
        return Plugin_Stop;
    }

    Survival_TryAwardMedal(medal);
    return Plugin_Stop;
}

void Survival_TryAwardMedal(int medal)
{
    char map[AP_MAP_LEN];
    GetCurrentMap(map, sizeof(map));
    if (!Survival_IsLevelConfigured(map) || !Survival_IsLevelUnlocked(map))
    {
        return;
    }

    char key[AP_MAP_LEN + 8];
    FormatEx(key, sizeof(key), "%s|%d", map, medal);
    if (g_survivalReportedMedals.ContainsKey(key))
    {
        return;
    }
    g_survivalReportedMedals.SetValue(key, 1);

    char escapedMap[AP_MAP_LEN * 2];
    JSON_Escape(map, escapedMap, sizeof(escapedMap));
    char payload[192];
    FormatEx(payload, sizeof(payload), "{\"map\":\"%s\",\"medal\":%d}", escapedMap, medal);
    Bridge_Send("SURVIVAL_MEDAL", payload);
}
