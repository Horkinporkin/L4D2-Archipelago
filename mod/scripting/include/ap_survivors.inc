#if defined _ap_survivors_included
    #endinput
#endif
#define _ap_survivors_included

// Handles Progressive Survivors + !ap character selection

#define AP_SURVIVOR_COMPANION_CAP 3
#define AP_SURVIVOR_SPAWN_CONFIRM_TIMEOUT 2.0
#define AP_SURVIVOR_SPAWN_RETRY_INTERVAL 1.0
#define AP_SURVIVOR_SPAWN_MAX_RETRIES 5
#define AP_SURVIVOR_TRANSITION_SETTLE_TIME 2.0

// L4D2_GetSurvivorSetMod() values
#define AP_SURVIVOR_SET_L4D1      1
#define AP_SURVIVOR_SET_L4D2      2
// m_survivorCharacter of the first L4D1 survivor (Bill); the L4D2 survivors are 0-3
#define AP_SURVIVOR_L4D1_FIRST_ID 4
#define AP_SURVIVOR_BOT_TYPES     4

// Indexed [set][character - first id of the set], set 0 = L4D2 survivors, 1 = L4D1 survivors
static const char AP_SURVIVOR_MODELS[2][AP_SURVIVOR_BOT_TYPES][] =
{
    {
        "models/survivors/survivor_gambler.mdl",    // Nick
        "models/survivors/survivor_producer.mdl",   // Rochelle
        "models/survivors/survivor_coach.mdl",      // Coach
        "models/survivors/survivor_mechanic.mdl"    // Ellis
    },
    {
        "models/survivors/survivor_namvet.mdl",     // Bill
        "models/survivors/survivor_teenangst.mdl",  // Zoey
        "models/survivors/survivor_biker.mdl",      // Francis
        "models/survivors/survivor_manager.mdl"     // Louis
    }
};

static const char AP_SURVIVOR_NAMES[2][AP_SURVIVOR_BOT_TYPES][] =
{
    { "Nick", "Rochelle", "Coach", "Ellis" },
    { "Bill", "Zoey", "Francis", "Louis" }
};

static const int AP_SURVIVOR_L4D1_SLOT_CHARACTERS[AP_SURVIVOR_BOT_TYPES] = { 4, 5, 7, 6 };

ConVar g_cvSurvivorLimit;
int g_originalSurvivorLimit;
bool g_haveOriginalSurvivorLimit;
bool g_survivorLimitWasChanged;
bool g_survivorProgressionEnabled;
bool g_survivorSyncPending;
bool g_survivorMapActive;
bool g_survivorTransitioning;
bool g_survivorRoundStarted;
bool g_survivorHumanSpawnedThisMap;
bool g_survivorAwaitingHumanSpawn;
bool g_survivorRestoreInProgress;
bool g_survivorReconcileQueued;
bool g_survivorTransitionReadyQueued;
bool g_survivorSpawnPending;
bool g_survivorKickPending[MAXPLAYERS + 1];
int g_survivorTargetCompanions;
int g_survivorMaxCompanions = AP_SURVIVOR_COMPANION_CAP;
int g_survivorSpawnBaseline;
int g_survivorSpawnRetries;
int g_survivorSpawnRequest;
int g_survivorTransitionReadyRequest;
int g_survivorCharacter[MAXPLAYERS + 1][2];
int g_survivorDefaultCharacter[MAXPLAYERS + 1][2];
bool g_survivorSetNativeAvailable;
bool g_survivorAddBotNativeAvailable;
char g_survivorCharacterFile[PLATFORM_MAX_PATH];

void Survivors_Init()
{
    BuildPath(Path_SM, g_survivorCharacterFile, sizeof(g_survivorCharacterFile),
        "data/l4d2_ap_characters.txt");

    g_cvSurvivorLimit = FindConVar("survivor_limit");
    if (g_cvSurvivorLimit != null)
    {
        g_originalSurvivorLimit = g_cvSurvivorLimit.IntValue;
        g_haveOriginalSurvivorLimit = true;
    }

    g_survivorSetNativeAvailable = GetFeatureStatus(FeatureType_Native,
        "L4D2_GetSurvivorSetMod") == FeatureStatus_Available;
    g_survivorAddBotNativeAvailable = GetFeatureStatus(FeatureType_Native,
        "L4D2Direct_AddSurvivorBot") == FeatureStatus_Available;

    for (int client = 1; client <= MAXPLAYERS; client++)
    {
        Survivors_ResetCharacters(client);
    }
}

void Survivors_ResetCharacters(int client)
{
    for (int set = 0; set < 2; set++)
    {
        g_survivorCharacter[client][set] = -1;
        g_survivorDefaultCharacter[client][set] = -1;
    }
}

int Survivors_SetIndex(bool l4d1Set)
{
    return l4d1Set ? 1 : 0;
}

void Survivors_OnPluginEnd()
{
    Survivors_RestoreSurvivorLimit();
}

void Survivors_OnBridgeDisconnected()
{
    if (g_survivorProgressionEnabled)
    {
        g_survivorSyncPending = true;
        g_survivorSpawnPending = false;
        g_survivorSpawnRequest++;
    }
}

void Survivors_OnMapStart()
{
    bool awaitingHumanSpawn = g_survivorTransitioning;
    g_survivorMapActive = true;
    g_survivorTransitioning = false;
    g_survivorRoundStarted = false;
    g_survivorHumanSpawnedThisMap = false;
    g_survivorAwaitingHumanSpawn = awaitingHumanSpawn;
    g_survivorRestoreInProgress = false;
    g_survivorReconcileQueued = false;
    g_survivorTransitionReadyQueued = false;
    g_survivorSpawnPending = false;
    g_survivorSpawnRetries = 0;
    g_survivorSpawnRequest++;
    g_survivorTransitionReadyRequest++;
    for (int client = 1; client <= MAXPLAYERS; client++)
    {
        g_survivorKickPending[client] = false;
    }

    // Survivors_ApplySelectedCharacter switches models, must precache
    for (int set = 0; set < 2; set++)
    {
        for (int i = 0; i < AP_SURVIVOR_BOT_TYPES; i++)
        {
            PrecacheModel(AP_SURVIVOR_MODELS[set][i], true);
        }
    }

    if (Survival_IsInSurvivalMode())
    {
        Survivors_RestoreSurvivorLimit();
        return;
    }

    if (g_survivorProgressionEnabled)
    {
        Survivors_UpdateSurvivorLimit();
        Survivors_ScheduleTransitionReady();
        Survivors_ScheduleReconcile();
    }
}

void Survivors_OnMapEnd()
{
    Survivors_OnTransition();
    g_survivorMapActive = false;
    g_survivorRoundStarted = false;
    g_survivorRestoreInProgress = false;
    g_survivorReconcileQueued = false;
    g_survivorSpawnPending = false;
    g_survivorSpawnRequest++;
}

void Survivors_OnTransition()
{
    g_survivorTransitioning = true;
    g_survivorHumanSpawnedThisMap = false;
    g_survivorAwaitingHumanSpawn = false;
    g_survivorTransitionReadyQueued = false;
    g_survivorSpawnPending = false;
    g_survivorSpawnRequest++;
    g_survivorTransitionReadyRequest++;
    Survivors_RestoreSurvivorLimit();
}

void Survivors_OnRoundStart()
{
    g_survivorRoundStarted = true;
    if (Survival_IsInSurvivalMode())
    {
        return;
    }
    if (g_survivorProgressionEnabled)
    {
        Survivors_ScheduleTransitionReady();
        Survivors_ScheduleReconcile();
    }
}

void Survivors_Configure(bool enabled, int maxCompanions)
{
    if (!enabled)
    {
        bool wasEnabled = g_survivorProgressionEnabled;
        g_survivorProgressionEnabled = false;
        g_survivorSyncPending = false;
        g_survivorTargetCompanions = 0;
        g_survivorSpawnPending = false;
        g_survivorSpawnRequest++;
        g_survivorSpawnRetries = 0;
        g_survivorTransitionReadyQueued = false;
        g_survivorTransitionReadyRequest++;
        Survivors_RestoreSurvivorLimit();
        if (wasEnabled)
        {
            LogMessage("[AP] Progressive AI Survivors disabled.");
        }
        return;
    }

    if (maxCompanions < 0)
    {
        maxCompanions = 0;
    }
    if (maxCompanions > AP_SURVIVOR_COMPANION_CAP)
    {
        maxCompanions = AP_SURVIVOR_COMPANION_CAP;
    }

    bool wasEnabled = g_survivorProgressionEnabled;
    g_survivorProgressionEnabled = true;
    g_survivorSyncPending = true;
    g_survivorMaxCompanions = maxCompanions;
    if (!wasEnabled)
    {
        g_survivorTargetCompanions = 0;
        g_survivorSpawnRetries = 0;
    }

    Survivors_UpdateSurvivorLimit();
    Survivors_ScheduleTransitionReady();
    Survivors_ScheduleReconcile();
    LogMessage("[AP] Progressive AI Survivors enabled, syncing AP survivor count (cap %d).",
        g_survivorMaxCompanions);
}

void Survivors_SyncCount(int companions)
{
    if (companions < 0)
    {
        companions = 0;
    }
    if (companions > AP_SURVIVOR_COMPANION_CAP)
    {
        companions = AP_SURVIVOR_COMPANION_CAP;
    }

    if (!g_survivorProgressionEnabled)
    {
        g_survivorTargetCompanions = 0;
        g_survivorSyncPending = false;
        return;
    }

    if (companions > g_survivorMaxCompanions)
    {
        companions = g_survivorMaxCompanions;
    }
    g_survivorTargetCompanions = companions;
    g_survivorSyncPending = false;
    g_survivorSpawnRetries = 0;
    Survivors_UpdateSurvivorLimit();
    LogMessage("[AP] synchronized Progressive Survivor state: %d AI survivor(s).", companions);
    Survivors_ScheduleReconcile();
}

void Survivors_Unlock()
{
    if (!g_survivorProgressionEnabled)
    {
        LogMessage("[AP] ignored Progressive Survivor item because the world option is disabled.");
        return;
    }
    if (g_survivorSyncPending)
    {
        return;
    }
    if (g_survivorTargetCompanions >= g_survivorMaxCompanions)
    {
        LogMessage("[AP] Progressive Survivor received at the configured %d-survivor cap.",
            g_survivorMaxCompanions);
        return;
    }

    g_survivorTargetCompanions++;
    g_survivorSpawnRetries = 0;
    Survivors_UpdateSurvivorLimit();
    LogMessage("[AP] Progressive Survivor unlocked: %d/%d AI survivor(s).",
        g_survivorTargetCompanions, g_survivorMaxCompanions);
    Survivors_ScheduleReconcile();
}

void Survivors_OnClientAuthorized(int client, const char[] auth)
{
    if (client <= 0 || client > MaxClients)
    {
        return;
    }

    Survivors_ResetCharacters(client);
    if (IsFakeClient(client) || strncmp(auth, "STEAM_", 6) != 0)
    {
        return;
    }

    KeyValues kv = new KeyValues("APSurvivorCharacters");
    if (kv.ImportFromFile(g_survivorCharacterFile) && kv.JumpToKey(auth))
    {
        int l4d2Character = kv.GetNum("l4d2", -1);
        int l4d1Character = kv.GetNum("l4d1", -1);
        g_survivorCharacter[client][0] = Survivors_IsValidCharacter(false, l4d2Character) ? l4d2Character : -1;
        g_survivorCharacter[client][1] = Survivors_IsValidCharacter(true, l4d1Character) ? l4d1Character : -1;
    }
    delete kv;
}

void Survivors_OnClientPutInServer()
{
    if (Survival_IsInSurvivalMode())
    {
        return;
    }
    if (g_survivorProgressionEnabled)
    {
        Survivors_UpdateSurvivorLimit();
        Survivors_ScheduleReconcile();
    }
}

void Survivors_OnClientDisconnect(int client)
{
    if (client > 0 && client <= MAXPLAYERS)
    {
        Survivors_ResetCharacters(client);
        g_survivorKickPending[client] = false;
    }

    if (g_survivorProgressionEnabled)
    {
        if (!IsFakeClient(client) && Survivors_CountHumans(true, client) == 0)
        {
            g_survivorHumanSpawnedThisMap = false;
            g_survivorAwaitingHumanSpawn = false;
            g_survivorTransitionReadyQueued = false;
            g_survivorTransitionReadyRequest++;
            Survivors_RestoreSurvivorLimit();
        }
        Survivors_ScheduleReconcile();
    }
}

void Survivors_OnPlayerTeamChanged()
{
    if (Survival_IsInSurvivalMode())
    {
        return;
    }
    if (g_survivorProgressionEnabled)
    {
        Survivors_UpdateSurvivorLimit();
        Survivors_ScheduleReconcile();
    }
}

void Survivors_OnPlayerSpawn(int client)
{
    if (!Players_IsSurvivor(client))
    {
        return;
    }

    if (!IsFakeClient(client))
    {
        if (IsPlayerAlive(client))
        {
            g_survivorHumanSpawnedThisMap = true;
            g_survivorRoundStarted = true;
        }
        Survivors_ScheduleTransitionReady();
        RequestFrame(Frame_ApplySelectedSurvivorCharacter, GetClientUserId(client));
    }

    if (!g_survivorProgressionEnabled)
    {
        return;
    }

    if (IsFakeClient(client) && g_survivorSpawnPending
        && Survivors_CountCompanionBots() > g_survivorSpawnBaseline)
    {
        g_survivorSpawnPending = false;
        g_survivorSpawnRequest++;
        g_survivorSpawnRetries = 0;
    }
    Survivors_ScheduleReconcile();
}

void Survivors_SelectCharacter(int client, bool l4d1Set, int character)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        return;
    }
    if (character != -1 && !Survivors_IsValidCharacter(l4d1Set, character))
    {
        return;
    }

    g_survivorCharacter[client][Survivors_SetIndex(l4d1Set)] = character;
    Survivors_PersistCharacterChoice(client);

    char name[32];
    Survivors_GetCharacterName(l4d1Set, character, name, sizeof(name));
    PrintToChat(client, "%s%s character saved: %s.", AP_CHAT_PREFIX, l4d1Set ? "L4D1" : "L4D2", name);

    if (Players_IsSurvivor(client, true))
    {
        RequestFrame(Frame_ApplySelectedSurvivorCharacter, GetClientUserId(client));
    }
}

void Survivors_GetSelectedCharacterName(int client, bool l4d1Set, char[] name, int maxlen)
{
    int character = -1;
    if (client > 0 && client <= MaxClients)
    {
        character = g_survivorCharacter[client][Survivors_SetIndex(l4d1Set)];
    }
    Survivors_GetCharacterName(l4d1Set, character, name, maxlen);
}

bool Survivors_IsCharacterSelected(int client, bool l4d1Set, int character)
{
    if (client <= 0 || client > MaxClients)
    {
        return false;
    }
    return g_survivorCharacter[client][Survivors_SetIndex(l4d1Set)] == character;
}

public Action L4D2_OnRestoreTransitionedSurvivorBots()
{
    if (g_survivorProgressionEnabled)
    {
        g_survivorRestoreInProgress = true;
    }
    return Plugin_Continue;
}

void Survivors_OnRestoreTransitionedSurvivorBotsComplete()
{
    if (g_survivorProgressionEnabled)
    {
        g_survivorRestoreInProgress = false;
        Survivors_ScheduleTransitionReady();
        Survivors_ScheduleReconcile();
    }
    Players_FixTransitionedSurvivors();
}

public void L4D2_OnRestoreTransitionedSurvivorBots_Post()
{
    Survivors_OnRestoreTransitionedSurvivorBotsComplete();
}

public void L4D2_OnRestoreTransitionedSurvivorBots_PostHandled()
{
    Survivors_OnRestoreTransitionedSurvivorBotsComplete();
}

public void L4D2_OnTransitionRestore_Post(int client, Address pkv)
{
    Players_OnTransitionRestore(client);
    Survivors_ScheduleTransitionReady();
}

public void Frame_ApplySelectedSurvivorCharacter(any userId)
{
    int client = GetClientOfUserId(userId);
    if (client > 0 && client <= MaxClients)
    {
        Survivors_ApplySelectedCharacter(client);
    }
}

public void Frame_SurvivorReconcile(any data)
{
    g_survivorReconcileQueued = false;
    Survivors_Reconcile();
}

public Action Timer_SurvivorSpawnConfirm(Handle timer, any requestId)
{
    if (requestId != g_survivorSpawnRequest || !g_survivorSpawnPending)
    {
        return Plugin_Stop;
    }

    if (Survivors_CountCompanionBots() > g_survivorSpawnBaseline)
    {
        g_survivorSpawnPending = false;
        g_survivorSpawnRequest++;
        g_survivorSpawnRetries = 0;
        Survivors_ScheduleReconcile();
        return Plugin_Stop;
    }

    g_survivorSpawnPending = false;
    g_survivorSpawnRequest++;
    g_survivorSpawnRetries++;
    LogError("[AP] survivor-bot creation attempt %d/%d was not confirmed.",
        g_survivorSpawnRetries, AP_SURVIVOR_SPAWN_MAX_RETRIES);

    if (g_survivorSpawnRetries < AP_SURVIVOR_SPAWN_MAX_RETRIES
        && g_survivorProgressionEnabled && g_survivorMapActive && !g_survivorSyncPending)
    {
        CreateTimer(AP_SURVIVOR_SPAWN_RETRY_INTERVAL, Timer_SurvivorSpawnRetry,
            g_survivorSpawnRequest, TIMER_FLAG_NO_MAPCHANGE);
    }
    return Plugin_Stop;
}

public Action Timer_SurvivorSpawnRetry(Handle timer, any requestId)
{
    if (requestId == g_survivorSpawnRequest && g_survivorProgressionEnabled)
    {
        Survivors_ScheduleReconcile();
    }
    return Plugin_Stop;
}

void Survivors_ScheduleTransitionReady()
{
    if (!g_survivorProgressionEnabled || !g_survivorMapActive
        || !g_survivorAwaitingHumanSpawn || g_survivorTransitionReadyQueued)
    {
        return;
    }

    g_survivorTransitionReadyQueued = true;
    g_survivorTransitionReadyRequest++;
    CreateTimer(AP_SURVIVOR_TRANSITION_SETTLE_TIME, Timer_SurvivorTransitionReady,
        g_survivorTransitionReadyRequest, TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_SurvivorTransitionReady(Handle timer, any requestId)
{
    if (requestId != g_survivorTransitionReadyRequest || !g_survivorTransitionReadyQueued)
    {
        return Plugin_Stop;
    }

    g_survivorTransitionReadyQueued = false;
    if (!g_survivorProgressionEnabled || !g_survivorMapActive
        || g_survivorTransitioning || !g_survivorAwaitingHumanSpawn
        || g_survivorRestoreInProgress || Survivors_CountHumans(true) == 0)
    {
        return Plugin_Stop;
    }

    g_survivorAwaitingHumanSpawn = false;
    g_survivorHumanSpawnedThisMap = true;
    g_survivorRoundStarted = true;
    Survivors_UpdateSurvivorLimit();
    Survivors_ScheduleReconcile();
    return Plugin_Stop;
}

void Survivors_ScheduleReconcile()
{
    if (Survival_IsInSurvivalMode())
    {
        return;
    }
    if (!g_survivorProgressionEnabled || !g_survivorMapActive
        || g_survivorTransitioning || g_survivorReconcileQueued)
    {
        return;
    }
    g_survivorReconcileQueued = true;
    RequestFrame(Frame_SurvivorReconcile);
}

void Survivors_Reconcile()
{
    if (Survival_IsInSurvivalMode())
    {
        return;
    }
    if (!g_survivorProgressionEnabled || !g_survivorMapActive
        || g_survivorTransitioning || g_survivorRestoreInProgress || g_survivorSyncPending)
    {
        return;
    }

    if (!g_survivorRoundStarted || !g_survivorHumanSpawnedThisMap
        || g_survivorAwaitingHumanSpawn || Survivors_CountHumans(true) == 0
        || g_survivorSpawnPending)
    {
        return;
    }

    Survivors_UpdateSurvivorLimit();
    int bots = Survivors_CountCompanionBots();
    if (bots > g_survivorTargetCompanions)
    {
        int excess = bots - g_survivorTargetCompanions;
        for (int client = MaxClients; client >= 1 && excess > 0; client--)
        {
            if (!Players_IsSurvivor(client) || !IsFakeClient(client) || g_survivorKickPending[client])
            {
                continue;
            }
            g_survivorKickPending[client] = true;
            KickClient(client, "Progressive Survivor roster adjusted");
            excess--;
        }
        return;
    }

    if (bots >= g_survivorTargetCompanions)
    {
        return;
    }

    if (!g_survivorAddBotNativeAvailable)
    {
        LogError("[AP] cannot add Progressive Survivor as Left4DHooks: L4D2Direct_AddSurvivorBot is unavailable.");
        return;
    }
    if (GetClientCount(false) >= MaxClients)
    {
        LogError("[AP] cannot add Progressive Survivor due to no free client slot.");
        return;
    }

    int characterType = Survivors_ChooseBotCharacterType();
    g_survivorSpawnBaseline = bots;
    g_survivorSpawnPending = true;
    g_survivorSpawnRequest++;
    L4D2Direct_AddSurvivorBot(characterType);
    CreateTimer(AP_SURVIVOR_SPAWN_CONFIRM_TIMEOUT, Timer_SurvivorSpawnConfirm,
        g_survivorSpawnRequest, TIMER_FLAG_NO_MAPCHANGE);
}

int Survivors_CountCompanionBots()
{
    int count = 0;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (Players_IsSurvivor(client) && IsFakeClient(client) && !g_survivorKickPending[client])
        {
            count++;
        }
    }
    return count;
}

int Survivors_CountHumans(bool requireAlive, int excludedClient = 0)
{
    int count = 0;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (client != excludedClient && Players_IsSurvivor(client, requireAlive) && !IsFakeClient(client))
        {
            count++;
        }
    }
    return count;
}

void Survivors_UpdateSurvivorLimit()
{
    if (Survival_IsInSurvivalMode())
    {
        Survivors_RestoreSurvivorLimit();
        return;
    }
    if (!g_survivorProgressionEnabled || g_cvSurvivorLimit == null || g_survivorTransitioning)
    {
        return;
    }

    if (!g_survivorHumanSpawnedThisMap)
    {
        if (g_survivorAwaitingHumanSpawn || Survivors_CountHumans(true) == 0)
        {
            return;
        }
        g_survivorHumanSpawnedThisMap = true;
        g_survivorRoundStarted = true;
    }

    int humans = Survivors_CountHumans(false);
    if (humans == 0)
    {
        return;
    }
    int desiredTotal = humans + g_survivorTargetCompanions;
    if (g_cvSurvivorLimit.IntValue != desiredTotal)
    {
        Survivors_SetSurvivorLimitSilently(desiredTotal);
        g_survivorLimitWasChanged = true;
    }
}

// Changes survivor_limit without the "server cvar changed" chat notification
void Survivors_SetSurvivorLimitSilently(int value)
{
    int flags = g_cvSurvivorLimit.Flags;
    bool hadNotifyFlag = (flags & FCVAR_NOTIFY) != 0;
    if (hadNotifyFlag)
    {
        g_cvSurvivorLimit.Flags = flags & ~FCVAR_NOTIFY;
    }

    g_cvSurvivorLimit.SetInt(value, true, false);

    if (hadNotifyFlag)
    {
        g_cvSurvivorLimit.Flags = flags;
    }
}

void Survivors_RestoreSurvivorLimit()
{
    if (g_survivorLimitWasChanged && g_haveOriginalSurvivorLimit && g_cvSurvivorLimit != null)
    {
        Survivors_SetSurvivorLimitSilently(g_originalSurvivorLimit);
    }
    g_survivorLimitWasChanged = false;
}

// First slot no survivor plays yet, for L4D2Direct_AddSurvivorBot
int Survivors_ChooseBotCharacterType()
{
    bool used[AP_SURVIVOR_BOT_TYPES];
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!Players_IsSurvivor(client) || !HasEntProp(client, Prop_Send, "m_survivorCharacter"))
        {
            continue;
        }
        int slot = GetEntProp(client, Prop_Send, "m_survivorCharacter");
        if (slot >= 0 && slot < AP_SURVIVOR_BOT_TYPES)
        {
            used[slot] = true;
        }
    }

    for (int slot = 0; slot < AP_SURVIVOR_BOT_TYPES; slot++)
    {
        if (!used[slot])
        {
            return slot;
        }
    }
    return GetRandomInt(0, AP_SURVIVOR_BOT_TYPES - 1);
}

void Survivors_ApplySelectedCharacter(int client)
{
    if (!Players_IsSurvivor(client, true) || IsFakeClient(client)
        || !g_survivorSetNativeAvailable
        || !HasEntProp(client, Prop_Send, "m_survivorCharacter"))
    {
        return;
    }

    int survivorSet = L4D2_GetSurvivorSetMod();
    if (survivorSet != AP_SURVIVOR_SET_L4D1 && survivorSet != AP_SURVIVOR_SET_L4D2)
    {
        return;
    }
    bool l4d1Set = survivorSet == AP_SURVIVOR_SET_L4D1;
    int set = Survivors_SetIndex(l4d1Set);

    // Remember the character the game picked, for the "Game default" choice
    int currentSlot = GetEntProp(client, Prop_Send, "m_survivorCharacter");
    int currentCharacter = Survivors_SlotCharacter(l4d1Set, currentSlot);
    if (g_survivorDefaultCharacter[client][set] == -1 && currentCharacter != -1)
    {
        g_survivorDefaultCharacter[client][set] = currentCharacter;
    }

    int character = g_survivorCharacter[client][set];
    if (character == -1)
    {
        character = g_survivorDefaultCharacter[client][set];
    }
    int slot = Survivors_CharacterSlot(l4d1Set, character);
    if (slot == -1)
    {
        return;
    }

    // prevent duplicate characters
    if (currentCharacter != -1 && slot != currentSlot)
    {
        for (int other = 1; other <= MaxClients; other++)
        {
            if (other != client && Players_IsSurvivor(other) && IsFakeClient(other)
                && GetEntProp(other, Prop_Send, "m_survivorCharacter") == slot)
            {
                Survivors_SetCharacterSlot(other, l4d1Set, currentSlot);
            }
        }
    }
    Survivors_SetCharacterSlot(client, l4d1Set, slot);
}

int Survivors_SlotCharacter(bool l4d1Set, int slot)
{
    if (slot < 0 || slot >= AP_SURVIVOR_BOT_TYPES)
    {
        return -1;
    }
    return l4d1Set ? AP_SURVIVOR_L4D1_SLOT_CHARACTERS[slot] : slot;
}

int Survivors_CharacterSlot(bool l4d1Set, int character)
{
    for (int slot = 0; slot < AP_SURVIVOR_BOT_TYPES; slot++)
    {
        if (Survivors_SlotCharacter(l4d1Set, slot) == character)
        {
            return slot;
        }
    }
    return -1;
}

void Survivors_SetCharacterSlot(int client, bool l4d1Set, int slot)
{
    int set = Survivors_SetIndex(l4d1Set);
    int index = Survivors_SlotCharacter(l4d1Set, slot) - Survivors_FirstCharacter(l4d1Set);
    SetEntProp(client, Prop_Send, "m_survivorCharacter", slot);
    SetEntityModel(client, AP_SURVIVOR_MODELS[set][index]);
    if (IsFakeClient(client))
    {
        SetClientInfo(client, "name", AP_SURVIVOR_NAMES[set][index]);
    }
}

void Survivors_PersistCharacterChoice(int client)
{
    char auth[32];
    if (!GetClientAuthId(client, AuthId_Steam2, auth, sizeof(auth))
        || strncmp(auth, "STEAM_", 6) != 0)
    {
        return;
    }

    KeyValues kv = new KeyValues("APSurvivorCharacters");
    kv.ImportFromFile(g_survivorCharacterFile);
    if (kv.JumpToKey(auth, true))
    {
        kv.SetNum("l4d2", g_survivorCharacter[client][0]);
        kv.SetNum("l4d1", g_survivorCharacter[client][1]);
        kv.Rewind();
        kv.ExportToFile(g_survivorCharacterFile);
    }
    delete kv;
}

int Survivors_FirstCharacter(bool l4d1Set)
{
    return l4d1Set ? AP_SURVIVOR_L4D1_FIRST_ID : 0;
}

bool Survivors_IsValidCharacter(bool l4d1Set, int character)
{
    int first = Survivors_FirstCharacter(l4d1Set);
    return character >= first && character < first + AP_SURVIVOR_BOT_TYPES;
}

void Survivors_GetCharacterName(bool l4d1Set, int character, char[] name, int maxlen)
{
    if (Survivors_IsValidCharacter(l4d1Set, character))
    {
        strcopy(name, maxlen, AP_SURVIVOR_NAMES[Survivors_SetIndex(l4d1Set)][character - Survivors_FirstCharacter(l4d1Set)]);
        return;
    }
    strcopy(name, maxlen, "Game default");
}
