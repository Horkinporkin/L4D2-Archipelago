#if defined _ap_protocol_included
    #endinput
#endif
#define _ap_protocol_included

// Handles bridge messages to various modules

#define PROGRESSIVE_WEAPON_TIER_ITEM  "Progressive Weapon Tier"
#define PROGRESSIVE_SURVIVOR_ITEM     "Progressive Survivor"
#define AP_HASH_SEED                  -2128831035   // 32-bit FNV-1a offset

char g_messagePayload[AP_RECV_BUFFER_SIZE];

StringMap g_processedItems;            // receipt keys handled during this AP session
StringMap g_deliveredItemReceipts;     // completed one-time item rewards
StringMap g_startingItemOccurrences;   // starting item name -> copies received during this sync
KeyValues g_itemReceiptStore;
char g_itemReceiptFile[PLATFORM_MAX_PATH];
bool g_hasItemRouteFingerprint;
int g_itemRouteFingerprint;
char g_apSessionFingerprint[128];      // last known AP session ID

void Protocol_Init()
{
    g_processedItems = new StringMap();
    g_deliveredItemReceipts = new StringMap();
    g_startingItemOccurrences = new StringMap();
}

void Protocol_HandleMessage(const char[] line)
{
    if (!Protocol_CheckVersion(line))
    {
        return;
    }

    char type[32];
    if (!JSON_GetField(line, "type", type, sizeof(type)))
    {
        Bridge_Send("ERROR", "{\"message\":\"Bridge message missing type.\"}");
        return;
    }
    if (!JSON_GetField(line, "payload", g_messagePayload, sizeof(g_messagePayload))
        || g_messagePayload[0] != '{')
    {
        Bridge_Send("ERROR", "{\"message\":\"Bridge message payload must be an object.\"}");
        return;
    }

    Protocol_Dispatch(type, g_messagePayload);
}

void Protocol_Dispatch(const char[] type, const char[] payload)
{
    if (StrEqual(type, "CONFIGURE_ROUTE", false))
    {
        Route_Configure(payload);
    }
    else if (StrEqual(type, "CONFIGURE_SURVIVAL", false))
    {
        Survival_Configure(payload);
    }
    else if (StrEqual(type, "SYNC_STATE", false))
    {
        Route_SyncCompletedChapters(payload);
        LogMessage("[AP] state sync received.");
    }
    else if (StrEqual(type, "SYNC_COMPLETE", false))
    {
        Protocol_HandleSyncComplete();
    }
    else if (StrEqual(type, "UNLOCK_CHAPTER", false))
    {
        Protocol_HandleUnlockChapter(payload);
    }
    else if (StrEqual(type, "UNLOCK_SURVIVAL_LEVEL", false))
    {
        Protocol_HandleUnlockSurvivalLevel(payload);
    }
    else if (StrEqual(type, "GIVE_ITEM", false))
    {
        Protocol_HandleGiveItem(payload);
    }
    else if (StrEqual(type, "ACTIVATE_TRAP", false))
    {
        Protocol_HandleActivateTrap(payload);
    }
    else if (StrEqual(type, "CONFIGURE_WEAPON_TIERS", false))
    {
        WeaponTiers_HandleConfigure(payload);
    }
    else if (StrEqual(type, "UNLOCK_WEAPON_TIER", false))
    {
        Protocol_HandleUnlockWeaponTier(payload);
    }
    else if (StrEqual(type, "CONFIGURE_SURVIVORS", false))
    {
        Protocol_HandleConfigureSurvivors(payload);
    }
    else if (StrEqual(type, "CONFIGURE_DEATHLINK", false))
    {
        Protocol_HandleConfigureDeathlink(payload);
    }
    else if (StrEqual(type, "RECEIVE_DEATHLINK", false))
    {
        Deathlink_Receive(payload);
    }
    else if (StrEqual(type, "UNLOCK_SURVIVOR", false))
    {
        Protocol_HandleUnlockSurvivor(payload);
    }
    else if (StrEqual(type, "SYNC_SURVIVORS", false))
    {
        Protocol_HandleSyncSurvivors(payload);
    }
    else if (StrEqual(type, "CHAT_LINE", false))
    {
        Chat_HandleLine(payload);
    }
    else
    {
        Bridge_Send("ERROR", "{\"message\":\"Unsupported bridge message.\"}");
    }
}

void Protocol_HandleSyncComplete()
{
    g_stateSynced = true;
    WeaponTiers_OnSyncComplete();
    Items_ScheduleFlush();
    Route_Enforce();
    LogMessage("[AP] initial item and route sync complete.");
}

void Protocol_HandleUnlockChapter(const char[] payload)
{
    char chapter[AP_NAME_LEN];
    char itemKey[AP_ITEM_KEY_LEN];
    if (JSON_GetField(payload, "chapter", chapter, sizeof(chapter))
        && Protocol_ClaimItem(payload, chapter, false, itemKey, sizeof(itemKey)))
    {
        Route_UnlockChapter(chapter);
    }
}

void Protocol_HandleUnlockSurvivalLevel(const char[] payload)
{
    char map[AP_MAP_LEN];
    char itemKey[AP_ITEM_KEY_LEN];
    if (JSON_GetField(payload, "map", map, sizeof(map))
        && Protocol_ClaimItem(payload, map, false, itemKey, sizeof(itemKey)))
    {
        Survival_Unlock(map);
    }
}

void Protocol_HandleGiveItem(const char[] payload)
{
    char item[AP_NAME_LEN];
    char itemKey[AP_ITEM_KEY_LEN];
    if (JSON_GetField(payload, "item", item, sizeof(item))
        && Protocol_ClaimItem(payload, item, true, itemKey, sizeof(itemKey)))
    {
        Items_GiveItem(item, itemKey);
    }
}

void Protocol_HandleActivateTrap(const char[] payload)
{
    char trap[AP_NAME_LEN];
    char itemKey[AP_ITEM_KEY_LEN];
    if (JSON_GetField(payload, "trap", trap, sizeof(trap))
        && Protocol_ClaimItem(payload, trap, true, itemKey, sizeof(itemKey)))
    {
        Traps_Activate(trap, itemKey);
    }
}

void Protocol_HandleUnlockWeaponTier(const char[] payload)
{
    char itemKey[AP_ITEM_KEY_LEN];
    if (Protocol_ClaimItem(payload, PROGRESSIVE_WEAPON_TIER_ITEM, false, itemKey, sizeof(itemKey)))
    {
        WeaponTiers_HandleUnlock();
    }
}

void Protocol_HandleUnlockSurvivor(const char[] payload)
{
    char itemKey[AP_ITEM_KEY_LEN];
    if (Protocol_ClaimItem(payload, PROGRESSIVE_SURVIVOR_ITEM, false, itemKey, sizeof(itemKey)))
    {
        Survivors_Unlock();
    }
}

void Protocol_HandleConfigureSurvivors(const char[] payload)
{
    char enabledText[16];
    char maxCompanionsText[16];
    bool enabled = JSON_GetField(payload, "enabled", enabledText, sizeof(enabledText))
        && StrEqual(enabledText, "true", false);
    int maxCompanions = AP_SURVIVOR_COMPANION_CAP;
    if (JSON_GetField(payload, "max_companions", maxCompanionsText, sizeof(maxCompanionsText)))
    {
        maxCompanions = StringToInt(maxCompanionsText);
    }
    Survivors_Configure(enabled, maxCompanions);
}

void Protocol_HandleConfigureDeathlink(const char[] payload)
{
    char enabledText[16];
    char triggerText[16];
    bool enabled = JSON_GetField(payload, "enabled", enabledText, sizeof(enabledText))
        && StrEqual(enabledText, "true", false);
    DeathlinkTrigger trigger = DeathlinkTrigger_Death;
    if (JSON_GetField(payload, "trigger", triggerText, sizeof(triggerText))
        && StrEqual(triggerText, "downed", false))
    {
        trigger = DeathlinkTrigger_Downed;
    }
    Deathlink_Configure(enabled, trigger);
}

void Protocol_HandleSyncSurvivors(const char[] payload)
{
    char companionsText[16];
    int companions = 0;
    if (JSON_GetField(payload, "companions", companionsText, sizeof(companionsText)))
    {
        companions = StringToInt(companionsText);
    }
    Survivors_SyncCount(companions);
}

bool Protocol_CheckVersion(const char[] line)
{
    char version[16];
    if (!JSON_GetField(line, "version", version, sizeof(version)))
    {
        Bridge_Send("ERROR", "{\"message\":\"Bridge message missing version.\"}");
        return false;
    }

    int received = StringToInt(version);
    if (received != AP_PROTOCOL_VERSION)
    {
        LogError("[AP] bridge protocol version mismatch: got %d, expected %d. "
            ... "Update the Archipelago client and this plugin together.",
            received, AP_PROTOCOL_VERSION);
        Bridge_Send("ERROR", "{\"message\":\"Bridge protocol version mismatch.\"}");
        return false;
    }
    return true;
}

bool Protocol_ClaimItem(const char[] payload, const char[] itemName, bool persistAcrossReload,
    char[] itemKey, int maxlen)
{
    char itemIndexText[16];
    return JSON_GetField(payload, "item_index", itemIndexText, sizeof(itemIndexText))
        && Protocol_MarkItemProcessed(itemIndexText, itemName, persistAcrossReload, itemKey, maxlen);
}

bool Protocol_MarkItemProcessed(const char[] itemIndexText, const char[] itemName,
    bool persistAcrossReload, char[] processedKey, int maxlen)
{
    if (itemIndexText[0] == '\0')
    {
        Bridge_Send("ERROR", "{\"message\":\"Item message missing item_index.\"}");
        return false;
    }

    if (StrEqual(itemIndexText, "-1"))
    {
        if (itemName[0] == '\0')
        {
            Bridge_Send("ERROR", "{\"message\":\"Starting item is missing its name.\"}");
            return false;
        }
        int occurrence = 0;
        g_startingItemOccurrences.GetValue(itemName, occurrence);
        occurrence++;
        g_startingItemOccurrences.SetValue(itemName, occurrence);
        Format(processedKey, maxlen, "start:%s:%d", itemName, occurrence);
    }
    else
    {
        strcopy(processedKey, maxlen, itemIndexText);
    }

    if (g_processedItems.ContainsKey(processedKey))
    {
        return false;
    }
    g_processedItems.SetValue(processedKey, 1);
    return !persistAcrossReload || !g_deliveredItemReceipts.ContainsKey(processedKey);
}

// Called once CONFIGURE_ROUTE has been handled. A different AP session or route means a new run.
void Protocol_OnRouteConfigured(const char[] sessionFingerprint, int routeHash)
{
    bool hasSessionFingerprint = sessionFingerprint[0] != '\0';
    bool routeChanged = g_hasItemRouteFingerprint && g_itemRouteFingerprint != routeHash;
    bool sessionChanged = hasSessionFingerprint && g_apSessionFingerprint[0] != '\0'
        && !StrEqual(g_apSessionFingerprint, sessionFingerprint);
    if (sessionChanged || routeChanged)
    {
        Protocol_ResetSessionState();
        LogMessage("[AP] new AP run detected, resetting progression.");
    }
    if (hasSessionFingerprint)
    {
        strcopy(g_apSessionFingerprint, sizeof(g_apSessionFingerprint), sessionFingerprint);
    }
    g_itemRouteFingerprint = routeHash;
    g_hasItemRouteFingerprint = true;

    ItemReceipts_Configure(sessionFingerprint, routeHash);
    g_startingItemOccurrences.Clear();
    Inventory_CheckRouteFingerprint(routeHash, sessionFingerprint);
}

void ItemReceipts_Configure(const char[] sessionFingerprint, int routeHash)
{
    if (sessionFingerprint[0] == '\0')
    {
        if (g_itemReceiptStore != null)
        {
            g_deliveredItemReceipts.Clear();
            delete g_itemReceiptStore;
            g_itemReceiptFile[0] = '\0';
        }
        return;
    }

    char routeHashText[24];
    IntToString(routeHash, routeHashText, sizeof(routeHashText));
    int fileHash = AP_HashString(AP_HASH_SEED, sessionFingerprint);
    fileHash = AP_HashString(fileHash, routeHashText);

    char receiptFile[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, receiptFile, sizeof(receiptFile),
        "data/l4d2_ap_item_receipts_%d.txt", fileHash);
    if (g_itemReceiptStore != null && StrEqual(receiptFile, g_itemReceiptFile))
    {
        return;
    }

    g_deliveredItemReceipts.Clear();
    delete g_itemReceiptStore;
    g_itemReceiptStore = new KeyValues("APItemReceipts");
    strcopy(g_itemReceiptFile, sizeof(g_itemReceiptFile), receiptFile);

    bool matchingStore = g_itemReceiptStore.ImportFromFile(g_itemReceiptFile);
    if (matchingStore)
    {
        char storedSession[128];
        g_itemReceiptStore.GetString("session_id", storedSession, sizeof(storedSession));
        matchingStore = StrEqual(storedSession, sessionFingerprint)
            && g_itemReceiptStore.GetNum("route_hash", 0) == routeHash;
    }

    if (matchingStore && g_itemReceiptStore.JumpToKey("items")
        && g_itemReceiptStore.GotoFirstSubKey())
    {
        do
        {
            char itemKey[AP_ITEM_KEY_LEN];
            if (g_itemReceiptStore.GetSectionName(itemKey, sizeof(itemKey)))
            {
                g_deliveredItemReceipts.SetValue(itemKey, 1);
            }
        }
        while (g_itemReceiptStore.GotoNextKey());
        g_itemReceiptStore.Rewind();
    }
    else if (!matchingStore)
    {
        delete g_itemReceiptStore;
        g_itemReceiptStore = new KeyValues("APItemReceipts");
        g_itemReceiptStore.SetString("session_id", sessionFingerprint);
        g_itemReceiptStore.SetNum("route_hash", routeHash);
    }

    if (g_deliveredItemReceipts.Size > 0)
    {
        LogMessage("[AP] restored %d completed one-shot item receipt(s).",
            g_deliveredItemReceipts.Size);
    }
}

void ItemReceipts_Commit(const char[] itemKey)
{
    if (g_itemReceiptStore == null || itemKey[0] == '\0'
        || g_deliveredItemReceipts.ContainsKey(itemKey))
    {
        return;
    }

    g_deliveredItemReceipts.SetValue(itemKey, 1);
    g_itemReceiptStore.Rewind();
    g_itemReceiptStore.JumpToKey("items", true);
    g_itemReceiptStore.JumpToKey(itemKey, true);
    g_itemReceiptStore.SetNum("delivered", 1);
    g_itemReceiptStore.Rewind();

    if (!g_itemReceiptStore.ExportToFile(g_itemReceiptFile))
    {
        LogError("[AP] failed to save item receipt '%s' to '%s'.",
            itemKey, g_itemReceiptFile);
    }
}

int AP_HashString(int hash, const char[] text)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        hash ^= text[i];
        hash *= 16777619;
    }
    return hash;
}

void Protocol_ResetSessionState()
{
    g_processedItems.Clear();
    g_deliveredItemReceipts.Clear();
    g_startingItemOccurrences.Clear();
    delete g_itemReceiptStore;
    g_itemReceiptFile[0] = '\0';

    Route_OnNewSession();
    Items_OnNewSession();
    Survivors_OnBridgeDisconnected();
    Survival_OnNewSession();
    WeaponTiers_OnNewSession();
    Pickups_OnNewSession();
    Traps_OnNewSession();
    Interactables_OnNewSession();
}
