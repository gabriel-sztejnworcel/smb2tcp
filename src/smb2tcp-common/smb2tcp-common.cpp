#include "smb2tcp-common.h"
#include <iostream>

std::wstring str_to_wstr(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

void print_hex_dump(BYTE* buffer, DWORD length)
{
    DWORD i, count, index;
    CHAR rgb_digits[] = "0123456789abcdef";
    CHAR rgb_line[100];
    char cb_line;

    for (index = 0; length; length -= count, buffer += count, index += count)
    {
        count = (length > 16) ? 16 : length;

        sprintf_s(rgb_line, 100, "%4.4x  ", index);
        cb_line = 6;

        for (i = 0; i < count; i++)
        {
            rgb_line[cb_line++] = rgb_digits[buffer[i] >> 4];
            rgb_line[cb_line++] = rgb_digits[buffer[i] & 0x0f];
            if (i == 7)
            {
                rgb_line[cb_line++] = ':';
            }
            else
            {
                rgb_line[cb_line++] = ' ';
            }
        }
        for (; i < 16; i++)
        {
            rgb_line[cb_line++] = ' ';
            rgb_line[cb_line++] = ' ';
            rgb_line[cb_line++] = ' ';
        }

        rgb_line[cb_line++] = ' ';

        for (i = 0; i < count; i++)
        {
            if (buffer[i] < 32 || buffer[i] > 126)
            {
                rgb_line[cb_line++] = '.';
            }
            else
            {
                rgb_line[cb_line++] = buffer[i];
            }
        }

        rgb_line[cb_line++] = 0;
        std::cout << "        ";
        std::cout << rgb_line << std::endl;
    }
}
