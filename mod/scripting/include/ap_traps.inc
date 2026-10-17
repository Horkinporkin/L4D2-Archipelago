#if defined _ap_traps_included
    #endinput
#endif
#define _ap_traps_included

// Handles AP trap items

#define AP_TRAP_MAP_SETTLE_TIME  5.0
#define AP_TRAP_RETRY_INTERVAL   1.0
#define AP_TRAP_SPAWN_ATTEMPTS   30

enum
{
    AP_TRAP_SMOKER = 1,
    AP_TRAP_BOOMER,
    AP_TRAP_HUNTER,
    AP_TRAP_SPITTER,
    AP_TRAP_JOCKEY,
    AP_TRAP_CHARGER,
    AP_TRAP_WITCH,
    AP_TRAP_TANK,
    AP_TRAP_CLASS_COUNT
};

char g_trapNames[AP_TRAP_CLASS_COUNT][] =
{
    "", "Smoker", "Boomer", "Hunter", "Spitter", "Jockey", "Charger", "Witch", "Tank"
};

ArrayList g_trapQueue;
ArrayList g_trapReceiptKeys;
Handle g_trapTimer;
bool g_trapMapActive;
bool g_trapTransitioning;
bool g_trapFinaleStageNativeAvailable;
float g_trapMapReadyAt;

void Traps_Init()
{
    g_trapQueue = new ArrayList();
    g_trapReceiptKeys = new ArrayList(ByteCountToCells(AP_ITEM_KEY_LEN));
    g_trapFinaleStageNativeAvailable = GetFeatureStatus(FeatureType_Native,
        "AP_GetCurrentFinaleStage") == FeatureStatus_Available;
}

void Traps_OnMapStart()
{
    g_trapMapActive = true;
    g_trapTransitioning = false;
    g_trapMapReadyAt = GetGameTime() + AP_TRAP_MAP_SETTLE_TIME;
    Traps_StartPump();
}

void Traps_OnMapEnd()
{
    g_trapMapActive = false;
    g_trapTransitioning = true;
    Traps_StopPump();
}

void Traps_OnNewSession()
{
    Traps_StopPump();
    g_trapQueue.Clear();
    g_trapReceiptKeys.Clear();
}

void Traps_OnTransition()
{
    g_trapTransitioning = true;
    Traps_StopPump();
}

void Traps_Activate(const char[] trapName, const char[] itemKey)
{
    int zombieClass = Traps_GetClass(trapName);
    if (zombieClass == 0)
    {
        LogMessage("[AP] ACTIVATE_TRAP for unknown trap '%s'.", trapName);
        return;
    }

    g_trapQueue.Push(zombieClass);
    g_trapReceiptKeys.PushString(itemKey);
    LogMessage("[AP] trap queued: %s (%d pending).", trapName, g_trapQueue.Length);

    // traps are queued until next valid spawn point (in the event of a loading screen/transition, etc.)
    if (g_trapQueue.Length == 1 && g_trapTimer == null && Traps_TrySpawnNext())
    {
        return;
    }
    Traps_StartPump();
}

void Traps_StartPump()
{
    if (g_trapTimer != null || g_trapQueue.Length == 0 || !g_trapMapActive || g_trapTransitioning)
    {
        return;
    }

    g_trapTimer = CreateTimer(AP_TRAP_RETRY_INTERVAL, Timer_TrapsPump,
        _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

// Never called from Timer_TrapsPump itself: deleting a timer inside its own callback is an error
void Traps_StopPump()
{
    delete g_trapTimer;
}

public Action Timer_TrapsPump(Handle timer)
{
    if (timer != g_trapTimer)
    {
        return Plugin_Stop;
    }

    if (g_trapQueue.Length == 0)
    {
        g_trapTimer = null;
        return Plugin_Stop;
    }

    if (Traps_TrySpawnNext() && g_trapQueue.Length == 0)
    {
        g_trapTimer = null;
        return Plugin_Stop;
    }

    return Plugin_Continue;
}

bool Traps_IsSpawnedEntity(int entity)
{
    if (entity <= 0)
    {
        return false;
    }
    return entity <= MaxClients ? IsClientInGame(entity) : IsValidEntity(entity);
}

bool Traps_UsesClientSlot(int zombieClass)
{
    return zombieClass != AP_TRAP_WITCH;
}

bool Traps_IsFinaleActive()
{
    return g_trapFinaleStageNativeAvailable
        && AP_GetCurrentFinaleStage() != FINALE_NONE;
}

bool Traps_IsFinaleMap()
{
    return FindEntityByClassname(-1, "trigger_finale") != -1;
}

bool Traps_CanSpawn(int zombieClass)
{
    if (zombieClass == AP_TRAP_TANK && (Traps_IsFinaleMap() || Traps_IsFinaleActive()))
    {
        return false;
    }
    if (!Traps_UsesClientSlot(zombieClass))
    {
        return true;
    }
    if (GetClientCount(false) >= MaxClients - 1)
    {
        return false;
    }
    return !Traps_IsFinaleActive();
}

bool Traps_TrySpawnNext()
{
    if (g_trapQueue.Length == 0 || !g_trapMapActive || g_trapTransitioning
        || GetGameTime() < g_trapMapReadyAt
        || !L4D_HasAnySurvivorLeftSafeArea())
    {
        return false;
    }

    int survivor = Players_GetFirstSurvivor();
    if (survivor == -1)
    {
        return false;
    }

    int zombieClass = g_trapQueue.Get(0);
    if (!Traps_CanSpawn(zombieClass))
    {
        return false;
    }

    float pos[3];
    float ang[3] = { 0.0, 0.0, 0.0 };
    ang[1] = GetRandomFloat(0.0, 360.0);

    // Use a Charger to find a spawn point for witch since witch is not a player-zombie class
    int positionClass = zombieClass == AP_TRAP_WITCH ? AP_TRAP_CHARGER : zombieClass;
    if (!L4D_GetRandomPZSpawnPosition(survivor, positionClass,
        AP_TRAP_SPAWN_ATTEMPTS, pos))
    {
        return false;
    }

    int infected;
    switch (zombieClass)
    {
        case AP_TRAP_WITCH:    infected = AP_SpawnWitch(pos, ang);
        case AP_TRAP_TANK:    infected = AP_SpawnTank(pos, ang);
        default:            infected = AP_SpawnSpecial(zombieClass, pos, ang);
    }

    if (!Traps_IsSpawnedEntity(infected))
    {
        return false;
    }

    char itemKey[AP_ITEM_KEY_LEN];
    g_trapReceiptKeys.GetString(0, itemKey, sizeof(itemKey));
    ItemReceipts_Commit(itemKey);
    g_trapQueue.Erase(0);
    g_trapReceiptKeys.Erase(0);
    char name[16];
    Traps_GetName(zombieClass, name, sizeof(name));
    LogMessage("[AP] trap spawned: %s (entity %d; %d pending).",
        name, infected, g_trapQueue.Length);
    return true;
}

int Traps_GetClass(const char[] trapName)
{
    for (int zombieClass = 1; zombieClass < AP_TRAP_CLASS_COUNT; zombieClass++)
    {
        if (StrEqual(trapName, g_trapNames[zombieClass], false))
        {
            return zombieClass;
        }
    }
    return 0;
}

void Traps_GetName(int zombieClass, char[] name, int maxlen)
{
    if (zombieClass > 0 && zombieClass < AP_TRAP_CLASS_COUNT)
    {
        strcopy(name, maxlen, g_trapNames[zombieClass]);
        return;
    }
    strcopy(name, maxlen, "Unknown");
}
