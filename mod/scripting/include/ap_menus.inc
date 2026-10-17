#if defined _ap_menus_included
    #endinput
#endif
#define _ap_menus_included

// AP menus backed by ap_routes.inc and ap_survivors.inc (!ap)

public Action Command_OpenMenu(int client, int args)
{
    if (client == 0)
    {
        return Plugin_Handled;
    }
    Menus_ShowMain(client);
    return Plugin_Handled;
}

void Menus_ShowMain(int client)
{
    Menu menu = new Menu(MenuHandler_APMain);
    menu.SetTitle("Archipelago");
    menu.AddItem("campaign", "Select Campaign Level");
    menu.AddItem("survival", "Select Survival Level");
    menu.AddItem("characters", "Select Character");
    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_APMain(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char selection[24];
        menu.GetItem(position, selection, sizeof(selection));
        if (StrEqual(selection, "campaign"))
        {
            Menus_ShowCampaign(client);
        }
        else if (StrEqual(selection, "survival"))
        {
            Menus_ShowSurvivalCampaign(client);
        }
        else if (StrEqual(selection, "characters"))
        {
            Menus_ShowCharacterSets(client);
        }
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

void Menus_ShowCharacterSets(int client)
{
    char l4d2Character[32];
    char l4d1Character[32];
    char title[128];
    Survivors_GetSelectedCharacterName(client, false, l4d2Character, sizeof(l4d2Character));
    Survivors_GetSelectedCharacterName(client, true, l4d1Character, sizeof(l4d1Character));
    Format(title, sizeof(title), "Select Character\nL4D2: %s | L4D1: %s", l4d2Character, l4d1Character);

    Menu menu = new Menu(MenuHandler_CharacterSets);
    menu.SetTitle(title);
    menu.AddItem("l4d2", "Select L4D2 Character");
    menu.AddItem("l4d1", "Select L4D1 Character");
    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_CharacterSets(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char selection[16];
        menu.GetItem(position, selection, sizeof(selection));
        if (StrEqual(selection, "l4d2"))
        {
            Menus_ShowL4D2Characters(client);
        }
        else if (StrEqual(selection, "l4d1"))
        {
            Menus_ShowL4D1Characters(client);
        }
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowMain(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

void Menus_AddCharacterItem(Menu menu, int client, bool l4d1Set, int character, const char[] name)
{
    char info[16];
    char display[48];
    if (character == -1)
    {
        strcopy(info, sizeof(info), "default");
    }
    else
    {
        IntToString(character, info, sizeof(info));
    }
    if (Survivors_IsCharacterSelected(client, l4d1Set, character))
    {
        Format(display, sizeof(display), "%s (selected)", name);
    }
    else
    {
        strcopy(display, sizeof(display), name);
    }
    menu.AddItem(info, display);
}

void Menus_ShowL4D2Characters(int client)
{
    Menu menu = new Menu(MenuHandler_L4D2Character);
    menu.SetTitle("Select L4D2 Character");
    Menus_AddCharacterItem(menu, client, false, 2, "Coach");
    Menus_AddCharacterItem(menu, client, false, 3, "Ellis");
    Menus_AddCharacterItem(menu, client, false, 0, "Nick");
    Menus_AddCharacterItem(menu, client, false, 1, "Rochelle");
    Menus_AddCharacterItem(menu, client, false, -1, "Game default");
    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

void Menus_ShowL4D1Characters(int client)
{
    Menu menu = new Menu(MenuHandler_L4D1Character);
    menu.SetTitle("Select L4D1 Character");
    Menus_AddCharacterItem(menu, client, true, 4, "Bill");
    Menus_AddCharacterItem(menu, client, true, 5, "Zoey");
    Menus_AddCharacterItem(menu, client, true, 6, "Francis");
    Menus_AddCharacterItem(menu, client, true, 7, "Louis");
    Menus_AddCharacterItem(menu, client, true, -1, "Game default");
    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

int MenuHandler_Character(Menu menu, MenuAction action, int client, int position, bool l4d1Set)
{
    if (action == MenuAction_Select)
    {
        char info[16];
        menu.GetItem(position, info, sizeof(info));
        int character = StrEqual(info, "default") ? -1 : StringToInt(info);
        Survivors_SelectCharacter(client, l4d1Set, character);
        Menus_ShowCharacterSets(client);
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowCharacterSets(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

public int MenuHandler_L4D2Character(Menu menu, MenuAction action, int client, int position)
{
    return MenuHandler_Character(menu, action, client, position, false);
}

public int MenuHandler_L4D1Character(Menu menu, MenuAction action, int client, int position)
{
    return MenuHandler_Character(menu, action, client, position, true);
}

#define AP_MAX_CHAPTERS_PER_CAMPAIGN 8

bool Menus_CampaignHasUnlockedChapter(const char[] campaign)
{
    for (int order = 0; order < AP_MAX_CHAPTERS_PER_CAMPAIGN; order++)
    {
        char chapter[AP_NAME_LEN];
        if (Route_GetVanillaChapterAtOrder(campaign, order, chapter, sizeof(chapter))
            && Route_IsChapterUnlocked(chapter))
        {
            return true;
        }
    }
    return false;
}

void Menus_ShowCampaign(int client)
{
    Menu menu = new Menu(MenuHandler_Campaign);
    menu.SetTitle("Select Campaign");

    int shown = 0;
    for (int i = 0; i < g_vanillaCampaignOrder.Length; i++)
    {
        char campaign[AP_NAME_LEN];
        g_vanillaCampaignOrder.GetString(i, campaign, sizeof(campaign));
        if (!Menus_CampaignHasUnlockedChapter(campaign))
        {
            continue;
        }
        menu.AddItem(campaign, campaign);
        shown++;
    }
    if (shown == 0)
    {
        menu.AddItem("", "No chapters unlocked yet", ITEMDRAW_DISABLED);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_Campaign(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char campaign[AP_NAME_LEN];
        menu.GetItem(position, campaign, sizeof(campaign));
        Menus_ShowChapter(client, campaign);
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowMain(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

void Menus_ShowChapter(int client, const char[] campaign)
{
    Menu menu = new Menu(MenuHandler_Chapter);
    menu.SetTitle(campaign);

    for (int order = 0; order < AP_MAX_CHAPTERS_PER_CAMPAIGN; order++)
    {
        char chapter[AP_NAME_LEN];
        if (!Route_GetVanillaChapterAtOrder(campaign, order, chapter, sizeof(chapter))
            || !Route_IsChapterUnlocked(chapter))
        {
            continue;
        }
        menu.AddItem(chapter, chapter);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_Chapter(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char chapter[AP_NAME_LEN];
        char map[AP_MAP_LEN];
        menu.GetItem(position, chapter, sizeof(chapter));
        if (g_chapterToMap.GetString(chapter, map, sizeof(map)))
        {
            Route_TravelToMap(map, false, true);
        }
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowCampaign(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

void Menus_ShowSurvivalCampaign(int client)
{
    Menu menu = new Menu(MenuHandler_SurvivalCampaign);
    menu.SetTitle("Select Survival Campaign");

    int shown = 0;
    for (int i = 0; i < g_vanillaCampaignOrder.Length; i++)
    {
        char campaign[AP_NAME_LEN];
        g_vanillaCampaignOrder.GetString(i, campaign, sizeof(campaign));
        if (!Survival_CampaignHasUnlockedLevel(campaign))
        {
            continue;
        }
        menu.AddItem(campaign, campaign);
        shown++;
    }
    if (shown == 0)
    {
        menu.AddItem("", "No Survival levels unlocked yet", ITEMDRAW_DISABLED);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_SurvivalCampaign(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char campaign[AP_NAME_LEN];
        menu.GetItem(position, campaign, sizeof(campaign));
        Menus_ShowSurvivalLevel(client, campaign);
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowMain(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}

void Menus_ShowSurvivalLevel(int client, const char[] campaign)
{
    Menu menu = new Menu(MenuHandler_SurvivalLevel);
    menu.SetTitle(campaign);

    char map[AP_MAP_LEN];
    char levelCampaign[AP_NAME_LEN];
    for (int i = 0; Survival_GetLevel(i, map, sizeof(map), levelCampaign, sizeof(levelCampaign)); i++)
    {
        if (!StrEqual(levelCampaign, campaign) || !Survival_IsLevelUnlocked(map))
        {
            continue;
        }
        char display[AP_NAME_LEN];
        if (!g_survivalLevelNames.GetString(map, display, sizeof(display)))
        {
            strcopy(display, sizeof(display), map);
        }
        menu.AddItem(map, display);
    }

    menu.ExitBackButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_SurvivalLevel(Menu menu, MenuAction action, int client, int position)
{
    if (action == MenuAction_Select)
    {
        char map[AP_MAP_LEN];
        menu.GetItem(position, map, sizeof(map));
        Survival_TravelToLevel(map);
    }
    else if (action == MenuAction_Cancel && position == MenuCancel_ExitBack)
    {
        Menus_ShowSurvivalCampaign(client);
    }
    else if (action == MenuAction_End)
    {
        delete menu;
    }
    return 0;
}
