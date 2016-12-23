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
    DELIMITER = ';',
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
            if (i > 1)
                messages.push_back(text.substr(0, i));
            text = text.substr(i + 1);
        }
        else
            break;
    }
}
