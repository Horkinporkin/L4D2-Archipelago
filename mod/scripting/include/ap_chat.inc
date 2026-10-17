#if defined _ap_chat_included
    #endinput
#endif
#define _ap_chat_included

// Script responsible for handling AP messages -> L4D2 chat (items sent/retrieved, locations checked, etc.)

#define AP_CHAT_PRINT_LEN 200
#define AP_CHAT_LINE_LEN 1024

// Colour markers the client uses (§1 .. §5) are rewritten to the game's
#define AP_CHAT_COLOR_MIN 1
#define AP_CHAT_COLOR_MAX 5

void Chat_PrintLine(const char[] message)
{
    if (message[0] == '\0')
    {
        return;
    }

    char line[AP_CHAT_LINE_LEN];
    strcopy(line, sizeof(line), message);

    char marker[8];
    char colorByte[2];
    for (int color = AP_CHAT_COLOR_MIN; color <= AP_CHAT_COLOR_MAX; color++)
    {
        Format(marker, sizeof(marker), "§%d", color);
        Format(colorByte, sizeof(colorByte), "%c", color);
        ReplaceString(line, sizeof(line), marker, colorByte);
    }

    Chat_PrintWrapped(line);
}

void Chat_PrintWrapped(char[] line)
{
    int length = strlen(line);
    int start = 0;
    while (start < length)
    {
        int end = start + AP_CHAT_PRINT_LEN;
        if (end >= length)
        {
            PrintToChatAll("%s", line[start]);
            return;
        }

        int split = end;
        while (split > start && line[split] != ' ')
        {
            split--;
        }
        if (split == start)
        {
            split = end;
            while (split > start && (line[split] & 0xC0) == 0x80)
            {
                split--;
            }
        }

        char cut = line[split];
        line[split] = '\0';
        PrintToChatAll("%s", line[start]);
        line[split] = cut;

        start = split;
        while (start < length && line[start] == ' ')
        {
            start++;
        }
    }
}

void Chat_HandleLine(const char[] message)
{
    char line[AP_CHAT_LINE_LEN];
    if (!JSON_GetField(message, "line", line, sizeof(line)))
    {
        LogError("[AP] dropping a chat line that does not fit the %d-byte buffer.",
            sizeof(line));
        return;
    }

    Chat_PrintLine(line);
}
