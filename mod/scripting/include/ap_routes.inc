#if defined _ap_routes_included
    #endinput
#endif
#define _ap_routes_included

// Handles route progression (chapter unlocks/competions, checks, travel)

#define AP_FINALE_COMPLETION_HOLD 8.0   // seconds to stay on a won finale before advancing. "let it sink in"
#define AP_MAP_CHANGE_DELAY       2.0

// Route data, copied from CONFIGURE_ROUTE
StringMap g_campaignChapters;          // AP campaign -> chapters in route order
StringMap g_mapToCampaign;             // map codename -> AP campaign label
StringMap g_vanillaChapterByOrder;     // "vanilla campaign|vanilla order" -> chapter name
StringMap g_routeNextVanillaKey;       // AP chapter name -> vanilla order key of the chapter that follows it
char g_routeBuffer[AP_RECV_BUFFER_SIZE];

// Progression
StringMap g_reportedCompletions;       // interactable/carrier location reported -> map it was earned on
ArrayList g_chapterReceiptOrder;       // unlocked chapters, most recently received first

// Enforcement and travel
char g_routeNoticeMap[AP_MAP_LEN];     // last map a redirect notice was printed for
char g_pendingMapChange[AP_MAP_LEN];
int g_mapChangeRequestId;              // invalidates atuomatic map changes
bool g_routeAdvancePending;            // a completed chapter needs a destination
bool g_routeAwaitingNativeTransition;  // do not redirect until map_transition has completed normally
char g_routeTransitionTarget[AP_MAP_LEN];
float g_advanceHoldUntil;              // GetTickedTime() gate after a finale win before advancing
bool g_routeHideScoreboardAvailable;
bool g_routeCleanupScoreboardOnMapStart;

void Route_Init()
{
    g_reportedCompletions = new StringMap();
    g_routeCampaigns = new ArrayList(ByteCountToCells(AP_NAME_LEN));
    g_campaignChapters = new StringMap();
    g_mapToChapter = new StringMap();
    g_mapToCampaign = new StringMap();
    g_chapterToMap = new StringMap();
    g_vanillaCampaignOrder = new ArrayList(ByteCountToCells(AP_NAME_LEN));
    g_vanillaChapterByOrder = new StringMap();
    g_routeNextVanillaKey = new StringMap();
    g_unlockedChapters = new StringMap();
    g_chapterReceiptOrder = new ArrayList(ByteCountToCells(AP_NAME_LEN));
    g_completedChapters = new StringMap();

    g_routeHideScoreboardAvailable = GetFeatureStatus(FeatureType_Native,
        "L4D2_HideScoreboard") == FeatureStatus_Available;
}

void Route_OnMapStart()
{
    g_advanceHoldUntil = 0.0;
    g_routeAwaitingNativeTransition = false;
    g_mapSettled = false;
    g_routeNoticeMap[0] = '\0';
    if (g_routeCleanupScoreboardOnMapStart)
    {
        g_routeCleanupScoreboardOnMapStart = false;
        CreateTimer(0.2, Timer_RouteHideScoreboard, _, TIMER_FLAG_NO_MAPCHANGE);
    }

    CreateTimer(0.2, Timer_ReportInitialMap, _, TIMER_FLAG_NO_MAPCHANGE);
    // Retries travel that could not happen yet (finale hold, nothing unlocked, rejected change)
    CreateTimer(1.0, Timer_EnforceRoute, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

void Route_OnMapEnd()
{
    g_pendingMapChange[0] = '\0';
    g_mapChangeRequestId++;
}

// A new AP run: forget unlocks and completions (the route definition is replaced by Route_Configure)
void Route_OnNewSession()
{
    g_reportedCompletions.Clear();
    g_unlockedChapters.Clear();
    g_chapterReceiptOrder.Clear();
    g_completedChapters.Clear();
}

// ---------------------------------------------------------------------------
// Route definition
// ---------------------------------------------------------------------------

// Rebuild the route tables, then let the protocol detect whether this is a new AP run
void Route_Configure(const char[] payload)
{
    g_inventoryRouteVerified = false;
    Route_ResetState();

    char sessionFingerprint[128];
    JSON_GetField(payload, "session_id", sessionFingerprint, sizeof(sessionFingerprint));
    if (!JSON_GetField(payload, "route", g_routeBuffer, sizeof(g_routeBuffer)))
    {
        LogError("[AP] CONFIGURE_ROUTE missing 'route' field.");
        return;
    }

    // Menus list campaigns in this order
    char campaignOrder[2048];
    if (JSON_GetField(payload, "vanilla_campaign_order", campaignOrder, sizeof(campaignOrder)))
    {
        JSON_SplitArrayStrings(campaignOrder, g_vanillaCampaignOrder);
    }
    if (g_vanillaCampaignOrder.Length == 0)
    {
        LogError("[AP] CONFIGURE_ROUTE has no vanilla_campaign_order; menus will be empty.");
    }

    int routeHash = AP_HASH_SEED;
    ArrayList campaignObjects = new ArrayList(ByteCountToCells(2048));
    JSON_SplitArrayObjects(g_routeBuffer, campaignObjects);
    for (int i = 0; i < campaignObjects.Length; i++)
    {
        char campaignObject[2048];
        campaignObjects.GetString(i, campaignObject, sizeof(campaignObject));
        routeHash = Route_AddCampaign(campaignObject, routeHash);
    }
    delete campaignObjects;

    if (g_routeCampaigns.Length == 0)
    {
        LogError("[AP] CONFIGURE_ROUTE produced zero campaigns (route bytes=%d).", strlen(g_routeBuffer));
        return;
    }

    LogMessage("[AP] route configured: %d campaigns.", g_routeCampaigns.Length);
    Protocol_OnRouteConfigured(sessionFingerprint, routeHash);
}

int Route_AddCampaign(const char[] campaignObject, int routeHash)
{
    char campaign[AP_NAME_LEN];
    JSON_GetField(campaignObject, "campaign", campaign, sizeof(campaign));
    g_routeCampaigns.PushString(campaign);
    routeHash = AP_HashString(routeHash, campaign);

    char chaptersArray[2048];
    JSON_GetField(campaignObject, "chapters", chaptersArray, sizeof(chaptersArray));
    ArrayList chapterObjects = new ArrayList(ByteCountToCells(256));
    JSON_SplitArrayObjects(chaptersArray, chapterObjects);

    ArrayList chapterNames = new ArrayList(ByteCountToCells(AP_NAME_LEN));
    for (int i = 0; i < chapterObjects.Length; i++)
    {
        char chapterObject[256];
        char chapter[AP_NAME_LEN];
        char map[AP_MAP_LEN];
        chapterObjects.GetString(i, chapterObject, sizeof(chapterObject));
        JSON_GetField(chapterObject, "name", chapter, sizeof(chapter));
        JSON_GetField(chapterObject, "map", map, sizeof(map));
        if (chapter[0] == '\0' || !Route_IsValidMapName(map))
        {
            LogError("[AP] skipping route chapter with a bad name or map ('%s' / '%s').", chapter, map);
            continue;
        }

        g_mapToChapter.SetString(map, chapter);
        g_mapToCampaign.SetString(map, campaign);
        g_chapterToMap.SetString(chapter, map);
        Route_AddVanillaPosition(chapterObject, chapter);
        routeHash = AP_HashString(AP_HashString(routeHash, chapter), map);
        chapterNames.PushString(chapter);
    }
    delete chapterObjects;

    StringMap_SetArrayValue(g_campaignChapters, campaign, chapterNames);
    return routeHash;
}

// Where a chapter sits in its vanilla campaign for menus and automatic transitions
void Route_AddVanillaPosition(const char[] chapterObject, const char[] chapter)
{
    char campaign[AP_NAME_LEN];
    char orderText[16];
    JSON_GetField(chapterObject, "vanilla_campaign", campaign, sizeof(campaign));
    int order = JSON_GetField(chapterObject, "vanilla_order", orderText, sizeof(orderText)) ? StringToInt(orderText) : -1;
    if (campaign[0] == '\0' || order < 0)
    {
        LogError("[AP] route chapter '%s' has no vanilla campaign position.", chapter);
        return;
    }

    char key[AP_NAME_LEN + 16];
    Route_FormatVanillaKey(key, sizeof(key), campaign, order);
    g_vanillaChapterByOrder.SetString(key, chapter);
    Route_FormatVanillaKey(key, sizeof(key), campaign, order + 1);
    g_routeNextVanillaKey.SetString(chapter, key);
}

void Route_FormatVanillaKey(char[] key, int maxlen, const char[] campaign, int order)
{
    FormatEx(key, maxlen, "%s|%d", campaign, order);
}

bool Route_IsValidMapName(const char[] map)
{
    if (map[0] == '\0')
    {
        return false;
    }
    for (int i = 0; map[i] != '\0'; i++)
    {
        char c = map[i];
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == '_';
        if (!ok)
        {
            return false;
        }
    }
    return true;
}

void Route_ResetState()
{
    Route_CancelCompletionHold();
    StringMap_ClearArrayValues(g_campaignChapters);
    g_routeCampaigns.Clear();
    g_mapToChapter.Clear();
    g_mapToCampaign.Clear();
    g_chapterToMap.Clear();
    g_vanillaCampaignOrder.Clear();
    g_vanillaChapterByOrder.Clear();
    g_routeNextVanillaKey.Clear();
    g_stateSynced = false;
    g_mapSettled = false;
    g_pendingMapChange[0] = '\0';
    g_routeAdvancePending = false;
    g_routeTransitionTarget[0] = '\0';
    g_mapChangeRequestId++;
}

// ---------------------------------------------------------------------------
// Progression
// ---------------------------------------------------------------------------

void Route_UnlockChapter(const char[] chapter)
{
    g_unlockedChapters.SetValue(chapter, 1);
    ArrayList_MoveToFront(g_chapterReceiptOrder, chapter);
    LogMessage("[AP] chapter unlocked: %s", chapter);
    Route_Enforce();
}

bool Route_IsChapterUnlocked(const char[] chapter)
{
    return g_unlockedChapters.ContainsKey(chapter);
}

bool Route_IsMapAllowed(const char[] map)
{
    if (Survival_IsLevelConfigured(map) && Survival_IsLevelUnlocked(map))
    {
        return true;
    }
    char chapter[AP_NAME_LEN];
    return g_mapToChapter.GetString(map, chapter, sizeof(chapter))
        && Route_IsChapterUnlocked(chapter);
}

// SYNC_STATE: keep server and local items/checks in sync
void Route_SyncCompletedChapters(const char[] payload)
{
    StringMap confirmed = new StringMap();   // chapter and AP Item Carrier locations checked on the server

    char completedArray[4096];
    if (JSON_GetField(payload, "completed_chapters", completedArray, sizeof(completedArray)))
    {
        ArrayList chapters = new ArrayList(ByteCountToCells(AP_NAME_LEN));
        JSON_SplitArrayStrings(completedArray, chapters);
        for (int i = 0; i < chapters.Length; i++)
        {
            char chapter[AP_NAME_LEN];
            chapters.GetString(i, chapter, sizeof(chapter));
            if (g_chapterToMap.ContainsKey(chapter))
            {
                g_completedChapters.SetValue(chapter, 1);
                confirmed.SetValue(chapter, 1);
            }
        }
        delete chapters;
    }

    static char pickupArray[8192];
    if (JSON_GetField(payload, "checked_pickups", pickupArray, sizeof(pickupArray)))
    {
        ArrayList pickups = new ArrayList(ByteCountToCells(AP_NAME_LEN));
        JSON_SplitArrayStrings(pickupArray, pickups);
        for (int i = 0; i < pickups.Length; i++)
        {
            char location[AP_NAME_LEN];
            pickups.GetString(i, location, sizeof(location));
            g_reportedCompletions.SetString(location, "");
            confirmed.SetValue(location, 1);
        }
        delete pickups;
    }

    char location[AP_NAME_LEN];
    char map[AP_MAP_LEN];
    StringMapSnapshot chapters = g_completedChapters.Snapshot();
    for (int i = 0; i < chapters.Length; i++)
    {
        chapters.GetKey(i, location, sizeof(location));
        if (!confirmed.ContainsKey(location) && g_chapterToMap.GetString(location, map, sizeof(map)))
        {
            Route_ReportLocation(location, map);
        }
    }
    delete chapters;

    StringMapSnapshot completions = g_reportedCompletions.Snapshot();
    for (int i = 0; i < completions.Length; i++)
    {
        completions.GetKey(i, location, sizeof(location));
        if (!confirmed.ContainsKey(location) && g_reportedCompletions.GetString(location, map, sizeof(map)))
        {
            Route_ReportLocation(location, map);
        }
    }
    delete completions;
    delete confirmed;
}

void Route_OnMapTransition()
{
    g_mapSettled = false;

    char currentMap[AP_MAP_LEN];
    GetCurrentMap(currentMap, sizeof(currentMap));

    char vanillaNext[AP_MAP_LEN];
    if (Route_GetVanillaNextMap(currentMap, vanillaNext, sizeof(vanillaNext))
        && !Route_IsMapAllowed(vanillaNext))
    {
        LogMessage("[AP] blocking vanilla transition %s -> %s (not unlocked).", currentMap, vanillaNext);
        g_routeAwaitingNativeTransition = false;
        g_routeAdvancePending = true;
        Route_ReportChapterCompletion("CHAPTER_COMPLETED");
        Route_AdvanceToNewestMap(currentMap);
        return;
    }

    g_routeAwaitingNativeTransition = true;
    g_routeAdvancePending = true;
    Route_ReportChapterCompletion("CHAPTER_COMPLETED");
}

void Route_OnFinaleWin()
{
    Route_ReportChapterCompletion("FINALE_COMPLETED");
    g_routeAdvancePending = true;
    g_mapSettled = false;
    g_advanceHoldUntil = GetTickedTime() + AP_FINALE_COMPLETION_HOLD;
}

void Route_ReportChapterCompletion(const char[] messageType)
{
    char map[AP_MAP_LEN];
    GetCurrentMap(map, sizeof(map));

    char chapter[AP_NAME_LEN];
    if (!g_mapToChapter.GetString(map, chapter, sizeof(chapter)))
    {
        return;
    }

    g_mapSettled = false;
    if (g_completedChapters.ContainsKey(chapter))
    {
        return;
    }
    g_completedChapters.SetValue(chapter, 1);
    Route_ReportLocation(chapter, map, messageType);
    LogMessage("[AP] reported completed location '%s' on map '%s'.", chapter, map);
}

// Interactable and AP Item Carrier checks, reported once per location
void Route_ReportInteractableCompletion(const char[] location)
{
    if (g_reportedCompletions.ContainsKey(location))
    {
        return;
    }

    char map[AP_MAP_LEN];
    GetCurrentMap(map, sizeof(map));
    g_reportedCompletions.SetString(location, map);
    Route_ReportLocation(location, map, "INTERACTABLE_COMPLETED");
}

bool Route_IsLocationReported(const char[] location)
{
    return g_reportedCompletions.ContainsKey(location);
}

void Route_ReportLocation(const char[] location, const char[] map, const char[] messageType = "LOCATION_CHECKED")
{
    char escapedLocation[AP_NAME_LEN * 2];
    char escapedMap[AP_MAP_LEN * 2];
    if (!JSON_Escape(location, escapedLocation, sizeof(escapedLocation))
        || !JSON_Escape(map, escapedMap, sizeof(escapedMap)))
    {
        LogError("[AP] location '%s' is too long to report.", location);
        return;
    }

    char payload[512];
    FormatEx(payload, sizeof(payload), "{\"map\":\"%s\",\"location\":\"%s\"}", escapedMap, escapedLocation);
    Bridge_Send(messageType, payload);
}

void Route_ReportGameState()
{
    if (g_client == null)
    {
        return;
    }

    char map[AP_MAP_LEN];
    char chapter[AP_NAME_LEN];
    char campaign[AP_NAME_LEN];
    GetCurrentMap(map, sizeof(map));
    g_mapToChapter.GetString(map, chapter, sizeof(chapter));
    g_mapToCampaign.GetString(map, campaign, sizeof(campaign));

    int survivors = 0;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (Players_IsSurvivor(client))
        {
            survivors++;
        }
    }

    char escapedMap[AP_MAP_LEN * 2];
    char escapedChapter[AP_NAME_LEN * 2];
    char escapedCampaign[AP_NAME_LEN * 2];
    JSON_Escape(map, escapedMap, sizeof(escapedMap));
    JSON_Escape(chapter, escapedChapter, sizeof(escapedChapter));
    JSON_Escape(campaign, escapedCampaign, sizeof(escapedCampaign));

    char payload[sizeof(g_lastReportedState)];
    FormatEx(payload, sizeof(payload),
        "{\"map\":\"%s\",\"chapter\":\"%s\",\"campaign\":\"%s\",\"survivors\":%d}",
        escapedMap, escapedChapter, escapedCampaign, survivors);
    if (StrEqual(payload, g_lastReportedState))
    {
        return;
    }
    strcopy(g_lastReportedState, sizeof(g_lastReportedState), payload);
    Bridge_Send("GAME_STATE", payload);
}

public Action Timer_ReportInitialMap(Handle timer)
{
    Route_ReportGameState();
    return Plugin_Stop;
}

// ---------------------------------------------------------------------------
// Destinations
// ---------------------------------------------------------------------------

// The most recently unlocked, unfinished chapter. Helps reduce manual !ap travels
bool Route_FindNextChapter(char[] chapter, int maxlen, bool allowCompleted)
{
    for (int i = 0; i < g_chapterReceiptOrder.Length; i++)
    {
        char name[AP_NAME_LEN];
        g_chapterReceiptOrder.GetString(i, name, sizeof(name));
        if (Route_IsChapterUnlocked(name) && g_chapterToMap.ContainsKey(name)
            && (allowCompleted || !g_completedChapters.ContainsKey(name)))
        {
            strcopy(chapter, maxlen, name);
            return true;
        }
    }

    if (!allowCompleted)
    {
        return false;
    }

    for (int i = 0; i < g_routeCampaigns.Length; i++)
    {
        char campaign[AP_NAME_LEN];
        g_routeCampaigns.GetString(i, campaign, sizeof(campaign));
        ArrayList chapters;
        if (!g_campaignChapters.GetValue(campaign, chapters))
        {
            continue;
        }
        for (int j = 0; j < chapters.Length; j++)
        {
            char name[AP_NAME_LEN];
            chapters.GetString(j, name, sizeof(name));
            if (Route_IsChapterUnlocked(name) && g_chapterToMap.ContainsKey(name))
            {
                strcopy(chapter, maxlen, name);
                return true;
            }
        }
    }
    return false;
}

bool Route_FindNextMap(char[] map, int maxlen, bool allowCompleted)
{
    char chapter[AP_NAME_LEN];
    return Route_FindNextChapter(chapter, sizeof(chapter), allowCompleted)
        && g_chapterToMap.GetString(chapter, map, maxlen);
}

bool Route_FindNextDestination(char[] map, int maxlen, bool &isSurvival)
{
    isSurvival = false;
    if (Route_FindNextMap(map, maxlen, false))
    {
        return true;
    }
    if (Survival_FindNextMap(map, maxlen))
    {
        isSurvival = true;
        return true;
    }
    return Route_FindNextMap(map, maxlen, true);
}

bool Route_GetVanillaChapterAtOrder(const char[] campaign, int order, char[] chapter, int maxlen)
{
    char key[AP_NAME_LEN + 16];
    Route_FormatVanillaKey(key, sizeof(key), campaign, order);
    return g_vanillaChapterByOrder.GetString(key, chapter, maxlen);
}

// Check if vanilla transition is okay
bool Route_GetVanillaNextMap(const char[] currentMap, char[] nextMap, int maxlen)
{
    char chapter[AP_NAME_LEN];
    char nextKey[AP_NAME_LEN + 16];
    char nextChapter[AP_NAME_LEN];
    return g_mapToChapter.GetString(currentMap, chapter, sizeof(chapter))
        && g_routeNextVanillaKey.GetString(chapter, nextKey, sizeof(nextKey))
        && g_vanillaChapterByOrder.GetString(nextKey, nextChapter, sizeof(nextChapter))
        && g_chapterToMap.GetString(nextChapter, nextMap, maxlen);
}

// ---------------------------------------------------------------------------
// Enforcement and travel
// ---------------------------------------------------------------------------

public Action Timer_EnforceRoute(Handle timer)
{
    Route_Enforce();
    return Plugin_Continue;
}

void Route_Enforce()
{
    if (!g_stateSynced || g_routeCampaigns.Length == 0)
    {
        return;
    }
    if (Survival_IsActive())
    {
        if (!g_mapSettled)
        {
            g_mapSettled = true;
            Items_ScheduleFlush();
        }
        return;
    }
    if (GetTickedTime() < g_advanceHoldUntil || g_routeAwaitingNativeTransition)
    {
        return;
    }
    if (!g_routeAdvancePending && (g_pendingMapChange[0] != '\0' || g_mapSettled))
    {
        return;
    }

    char map[AP_MAP_LEN];
    GetCurrentMap(map, sizeof(map));

    if (g_routeAdvancePending)
    {
        Route_AdvanceToNewestMap(map);
        return;
    }

    if (Route_IsMapAllowed(map))
    {
        g_mapSettled = true;
        Items_ScheduleFlush();
        return;
    }

    char destination[AP_MAP_LEN];
    bool isSurvival;
    if (!Route_FindNextDestination(destination, sizeof(destination), isSurvival) || StrEqual(destination, map))
    {
        if (Route_NotifyOncePerMap(map))
        {
            LogMessage("[AP] no playable destination from %s.", map);
        }
        return;
    }

    if (Route_NotifyOncePerMap(map))
    {
        char chapter[AP_NAME_LEN];
        if (!g_mapToChapter.GetString(map, chapter, sizeof(chapter)))
        {
            PrintToChatAll("%s%s isn't part of this run.", AP_CHAT_PREFIX, map);
        }
        else if (!Route_IsChapterUnlocked(chapter))
        {
            PrintToChatAll("%s%s isn't unlocked yet.", AP_CHAT_PREFIX, chapter);
        }
    }

    LogMessage("[AP] redirecting %s -> %s (%s).", map, destination, isSurvival ? "survival" : "coop");
    Route_TravelToMap(destination, isSurvival, false);
}

void Route_AdvanceToNewestMap(const char[] currentMap)
{
    char destination[AP_MAP_LEN];
    bool isSurvival;
    if (!Route_FindNextDestination(destination, sizeof(destination), isSurvival))
    {
        g_routeTransitionTarget[0] = '\0';
        return;
    }

    if (StrEqual(currentMap, destination))
    {
        g_routeAdvancePending = false;
        g_routeTransitionTarget[0] = '\0';
        g_mapSettled = true;
        Route_CancelCompletionHold();
        Items_ScheduleFlush();
        return;
    }

    if (!StrEqual(g_routeTransitionTarget, destination))
    {
        strcopy(g_routeTransitionTarget, sizeof(g_routeTransitionTarget), destination);
        LogMessage("[AP] queue advance %s -> %s (%s).", currentMap, destination,
            isSurvival ? "survival" : "coop");
    }
    Route_TravelToMap(destination, isSurvival, false);
}

bool Route_NotifyOncePerMap(const char[] map)
{
    if (StrEqual(g_routeNoticeMap, map))
    {
        return false;
    }
    strcopy(g_routeNoticeMap, sizeof(g_routeNoticeMap), map);
    return true;
}

// Schedules travel to an allowed map
void Route_TravelToMap(const char[] map, bool survival, bool manualSelection)
{
    if (!Route_IsMapAllowed(map))
    {
        return;
    }
    if (manualSelection)
    {
        g_routeAdvancePending = false;
        g_routeTransitionTarget[0] = '\0';
        Route_CancelPendingMapChange();
    }
    if (g_pendingMapChange[0] != '\0')
    {
        return;
    }

    Route_SetGameMode(survival ? "survival" : "coop");
    Inventory_SaveAll();
    g_mapSettled = false;
    strcopy(g_pendingMapChange, sizeof(g_pendingMapChange), map);
    g_mapChangeRequestId++;
    CreateTimer(AP_MAP_CHANGE_DELAY, Timer_ChangeLevel, g_mapChangeRequestId, TIMER_FLAG_NO_MAPCHANGE);
}

void Route_SetGameMode(const char[] mode)
{
    ConVar gameMode = FindConVar("mp_gamemode");
    if (gameMode == null)
    {
        LogError("[AP] mp_gamemode ConVar missing; cannot set mode to %s", mode);
        return;
    }

    char current[32];
    gameMode.GetString(current, sizeof(current));
    if (StrEqual(current, mode, false))
    {
        return;
    }
    gameMode.SetString(mode, true);
    LogMessage("[AP] set mp_gamemode to %s", mode);
}

void Route_CancelPendingMapChange()
{
    if (g_pendingMapChange[0] == '\0')
    {
        return;
    }
    g_pendingMapChange[0] = '\0';
    g_mapChangeRequestId++;
}

void Route_CancelCompletionHold()
{
    g_advanceHoldUntil = 0.0;
}

public Action Timer_ChangeLevel(Handle timer, any requestId)
{
    if (requestId != g_mapChangeRequestId || g_pendingMapChange[0] == '\0'
        || g_routeAwaitingNativeTransition)
    {
        return Plugin_Stop;
    }

    char destination[AP_MAP_LEN];
    strcopy(destination, sizeof(destination), g_pendingMapChange);
    g_pendingMapChange[0] = '\0';
    if (Route_IsMapAllowed(destination))
    {
        Route_ForceTravelToMap(destination, "Archipelago route travel");
    }
    return Plugin_Stop;
}

void Route_ForceTravelToMap(const char[] map, const char[] reason)
{
    if (g_routeAwaitingNativeTransition)
    {
        return;
    }

    Route_ClearEndScreen();
    Survivors_OnTransition();
    Players_OnForcedTravel();
    ForceChangeLevel(map, reason);
}

void Route_ClearEndScreen()
{
    if (!g_routeHideScoreboardAvailable)
    {
        return;
    }
    L4D2_HideScoreboard();
    g_routeCleanupScoreboardOnMapStart = true;
}

public Action Timer_RouteHideScoreboard(Handle timer)
{
    if (g_routeHideScoreboardAvailable)
    {
        L4D2_HideScoreboard();
    }
    return Plugin_Stop;
}
