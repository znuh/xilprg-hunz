/*
xilprg is covered by the LGPL:

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

Copyright (c) 2006 Zoltan Csizmadia <zoltan_csizmadia@yahoo.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "xilprg.h"
#include "utils.h"
#include "cmdline.h"
#include "prgfile.h"

#if (!defined(WIN32) || defined(__CYGWIN__))
#include <readline/readline.h>
#include <readline/history.h>
#endif

static u8 reverse_bits_table[256] = 
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

#if defined(WIN32) && !defined(__CYGWIN__)
u8 inb(unsigned int port)
{
    u8 rc;
    __asm mov edx,port
    __asm in al, dx
    __asm mov rc, al
    return rc;
}

void outb(u8 data, unsigned int port)
{
    __asm mov edx,port
    __asm mov al, data
    __asm out dx, al
}
#endif

int enable_user_mode_io()
{
#if defined(WIN32) && !defined(__CYGWIN__)
    HANDLE h;
    
	h = CreateFile(
            "\\\\.\\giveio",
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (h != INVALID_HANDLE_VALUE)
    {
        // Giveio activated successfully
        CloseHandle(h);
        return 0;
    }
#else
    if (iopl(3) == 0)
        // Success
        return 0;
#endif

    msgf(STR_ENABLE_USERMODE_IO_ERROR);
    return -1;
}

u8 reverse8(u8 d)
{
    return reverse_bits_table[d];
}

int set_current_directory(const char* sz)
{
#ifdef WIN32
    return  SetCurrentDirectory(sz) ? 0 : -1;
#else
    return chdir(sz);
#endif
}

int get_current_directory(char* sz, int len)
{
#ifdef WIN32
    return  GetCurrentDirectory(len, sz) ? 0 : -1;
#else
    return getcwd(sz, len) ? 0 : -1;
#endif
}

void readline_load_history(void) {
#ifndef WIN32
	char line[1024];
	
	strcpy(line,getenv("HOME"));
	strcat(line,"/.xilprg_history");
	read_history(line);
#endif
}

void readline_save_history(void) {
#ifndef WIN32
	char line[1024];
	
	strcpy(line,getenv("HOME"));
	strcat(line,"/.xilprg_history");
	write_history(line);
#endif	
}

int prompt_read_line(const char* prompt, char* line, int maxlen)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    // Change text color
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE), 
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    printf(prompt);

    // Restore text color
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbi.wAttributes);
	
    if (fgets(line, maxlen, stdin) == NULL)
        return -1;

    strip_whitespaces(line);
	return 0;
#else
	char* sz;
	
	sz = readline(prompt);
	if (sz == NULL)
		return -1;
    if (*sz)
		add_history(sz);
    strip_whitespaces(sz);
    strncpy(line, sz, maxlen);
	free(sz);
	return 0;
#endif
}

int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == 0x0D || c == 0x0A;
}

char* strip_whitespaces(char* s)
{
    int len, i, j;

    if (*s == 0)
        return s;

    // Back
    len = strlen(s) - 1;
    while (is_whitespace(s[len]))
        s[len--] = 0;

    // Front
    i = 0;
    while (is_whitespace(s[i]))
        i++;
    if (i)
    {
        for (j = 0; s[i + j]; j++)
            s[j] = s[i + j];
        s[j] = 0;
    }

    return s;
}

int hexdigit(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    else
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    else
        return 0;
}

int hex2int(const char* sz, int n, int* sum) 
{
    int v = 0, b;
    while (n--)
    {
        if (*sz == 0)
            return v;
		b = hexdigit(*sz++) << 4;
        if (*sz == 0)
            return v;
        b += hexdigit(*sz++);
        if (sum)
            *sum += b;
        v = (v << 8) + b;
    }
    return v;
}

int str2num(const char* str, int* v)
{
    int base, digit, neg = 0;

    if (str == NULL)
        return -1;

    base = 10;

    if (*str == '-')
    {
        neg = 1;
        str++;
    }

    // Hex or binary
    if (*str == '0')
        if (str[1] == 'x' || str[1] == 'X' || 
			str[1] == 'b' || str[1] == 'B')
        {
            base = str[1] == 'x' || str[1] == 'X' ? 16 : 2;
            str += 2;
        }

    *v = 0;
    while (*str)
    {
        if (*str >= '0' && *str <= '9')
            digit = *str - '0';
        else
        if (*str >= 'A' && *str <= 'Z')
            digit = *str - 'A' + 10;
        else
            digit = *str - 'a' + 10;
        
        if (digit < 0 || digit >= base)
            return -1;

        *v *= base;
        *v += digit;
        str++;
    }

    if (neg)
        *v *= -1;

    return 0;
}

program_file* create_program_file(chip* dev, int argc, const char** argv)
{
	if (cmdline_has_opt(argc, argv, "mcs"))
		return new mcs_file;
	else
	if (cmdline_has_opt(argc, argv, "bit"))
		return new bit_file;
	else
	if (cmdline_has_opt(argc, argv, "bin"))
		return new bin_file;
	else
	{
		string type;

		// Default based on device type
		dev->family->vars.get(strTYPE, type);
		if (type == strFPGA)
			return new bit_file;
		else
		if (type == strPROM)
			return new mcs_file;
	}

	return NULL;
}
