#pragma once
#include <stdint.h>
#include <vector>
#include <string>

typedef uint32_t uint32;
typedef int32_t int32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#define Guard std::lock_guard<std::mutex>

enum eTimers
{
    IN_MILISECONDS = 1000,
    IN_MICROSECONDS = 1000,
    DIFF = 100,
    DISCONNECT_TIMEOUT = 30 * IN_MILISECONDS,
};

enum eSpecialChars
{
    DELIMITER = '\n',
    ESCAPE = '\\',
    TOKEN = ' ',
};


static void splitMessages(std::vector<std::string>& messages, std::string& text, char delimiter)
{
    while (!text.empty())
    {

        bool escaped = false;
        uint32 i = 1;
        for (i = 1; i < text.size(); i++)
        {
            if (text[i] == delimiter && !escaped)
                break;

            if (text[i] == ESCAPE && !escaped)
                escaped = true;
            else
                escaped = false;
        }

        if (i < text.size() || delimiter == TOKEN)
        {
            if (i >= 1)
                messages.push_back(text.substr(0, i));
            if (i + 1 <= text.length())
                text = text.substr(i + 1);
            else
                text.clear();
        }
        else
            break;
    }
}

static std::string escapeString(std::string str)
{
    std::string msg;
    std::vector<std::string> tokens;
    splitMessages(tokens, str, TOKEN);
    for (uint32 i = 0; i < tokens.size(); i++)
    {
        if (i != 0)
            msg += "\\ ";
        msg += tokens[i];
    }
    if (!str.empty())
        msg += "\\ " + str;
    return msg;
}
