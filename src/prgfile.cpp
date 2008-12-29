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
#include <sys/stat.h>
#include "prgfile.h"
#include "utils.h"

///////////////////////////////////////////////////////////
//
// Base class 
//

program_file::program_file()
: data(NULL), length(0)
{
}

program_file::~program_file()
{
    clear();
}

int program_file::load(const char*)
{
	return -1;
}

void program_file::clear()
{
    if (data)
        free(data);

    data = NULL;
    length = 0;
}

int program_file::get_length()
{
    return length;
}

int program_file::get_bit_length()
{
    return length*8;
}

u8* program_file::get_stream()
{
    return data;
}

int program_file::save(const char*, void*, int)
{
    msgf(STR_INVALID_FILE_FORMAT);
	return -1;
}

///////////////////////////////////////////////////////////
//
// BIN file 
//

int bin_file::load(const char* name)
{
    FILE *f;
	struct stat st;
    int rc = -1;
    
    if (data || length)
        // Call clear() first
        return -1;

    if (stat(name, &st) < 0)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
        return -1;
	}
    
    f = fopen(name, "rb");
    if (f == NULL)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
        return -1;
	}

	// File length
	length = st.st_size;
	data = (u8*)malloc(length);
	if (data == NULL)
		goto cleanup;

	if (fread(data, 1, length, f) != length)
	{
		msgf(STR_INVALID_FILE_FORMAT);
		goto cleanup;
	}

	rc = 0;

cleanup:

	if (rc)
		clear();

	if (f)
		fclose(f);

    return rc;
}

int bin_file::save(const char* file, void* data, int len)
{
	FILE* f;
	int rc = -1;

	f = fopen(file, "wb");
	if (f == NULL)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
		return -1;
	}

	if (fwrite(data, len, 1, f) != (size_t)len)
		goto cleanup;

	rc = 0;

cleanup:

	fclose(f);

	return rc;
}

///////////////////////////////////////////////////////////
//
// BIT file 
//

int bit_file::load(const char* name)
{
    FILE *f;
    int rc = -1;
    u8 field;

    if (data || length)
        // Call clear() first
        return -1;
    
    f = fopen(name, "rb");
    if (f == NULL)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
	    return -1;
	}
	
    // Skip header
    fseek(f, 13, SEEK_SET);
    
    // Read fields
    while(!feof(f))
    {
        if (fread(&field, 1, 1, f) != 1)
            goto cleanup;
        
        switch (field)
        {
        case 'a': // NCD File Name
        case 'b': // Part Name
        case 'c': // Creation Date
        case 'd': // Creation Time
            if (read_field(f, NULL, 0) != 0)
			{
				msgf(STR_INVALID_FILE_FORMAT);
				goto cleanup;
			}
            break;

        case 'e': // Data
            if (alloc_read_data(f) != 0)
			{
				msgf(STR_INVALID_FILE_FORMAT);
				goto cleanup;
			}
            rc = 0;
            goto cleanup;
            break;

        default:
			msgf(STR_INVALID_FILE_FORMAT);
            goto cleanup;
        }
    }

cleanup:

	if (rc)
		clear();

    if (f)
        fclose(f);

    return rc;
}

int bit_file::read_field(FILE* f, char* field, size_t maxlen)
{
    u16 len;
    
    if (fread(&len, 1, 2, f) != 2)
        return -1;
    len = NTOH16(len);

    if (field == NULL)
    {
        fseek(f, len, SEEK_CUR);
        return 0;
    }

    maxlen = maxlen - 1 < len ? maxlen - 1 : len;
    if (fread(field, 1, maxlen, f) != maxlen)
        return -1;
    field[maxlen] = 0;

    if (len > maxlen)
        fseek(f, len - maxlen, SEEK_CUR);

    return 0;
}

int bit_file::alloc_read_data(FILE* f)
{
    unsigned int i;

    // Read length
    if (fread(&length, 4, 1, f) != 1)
        return -1;

    length = NTOH32(length);
    data = (u8*)malloc(length);
    if (data == NULL)
        return -1;

    if (fread(data, 1, length, f) != length)
    {
        clear();
        return -1;
    }

    // Reverse bytes
    for (i = 0; i < length; i++)
        data[i] = reverse8(data[i]);

    return 0;
}

///////////////////////////////////////////////////////////
//
// MCS file 
//

int mcs_file::load(const char* name)
{
    FILE *f;
    int rc = -1;
    char line[256];
	struct stat st;
	int len, type, addr;
	unsigned char buff[64];
	int addr_offs = 0;
	u32 max_length;

    if (data || length)
        // Call clear() first
        return -1;

	if (stat(name, &st) < 0)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
        return -1;
	}
    
    f = fopen(name, "rt");
    if (f == NULL)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
        return -1;
	}

	// This size must be enough!
	max_length = st.st_size / 2;
	data = (u8*)malloc(max_length);
	if (data == NULL)
		goto cleanup;

	memset(data, 0xFF, max_length);
	length = 0;
  
    // Read lines
    while(!feof(f))
    {
        if (fgets(line, sizeof(line), f) == NULL)
			break;

		strip_whitespaces(line);

		if (parse_record(line, &len, &addr, &type, buff, sizeof(buff)))
		{
			msgf(STR_INVALID_FILE_FORMAT);
			goto cleanup;
		}

		if (type == RECORD_DATA)
		{
			if ((u32)(addr_offs + addr + len) > max_length)
			{
				msgf(STR_INVALID_FILE_FORMAT);
				goto cleanup;
			}
			
			memcpy(data + addr_offs + addr, buff, len);
			
			if ((u32)(addr_offs + addr + len) > length)
				length = (u32)(addr_offs + addr + len);
		}
		else
		if (type == RECORD_ADDRESS)
			addr_offs = (buff[0] << 24) + (buff[1] << 16);
		else
		if (type == RECORD_EOF)
			// EOF
			break;
    }

	rc = 0;

cleanup:

	if (rc)
		clear();

    if (f)
        fclose(f);

    return rc;
}

int mcs_file::save(const char* file, void* data, int len)
{
	FILE* f;
	int rc = -1;
    int addr, addr_offs, count;
    u8 buff[16];
    int buff_len = 16;

	f = fopen(file, "wt");
	if (f == NULL)
	{
		msgf(STR_UNABLE_TO_OPEN_FILE);
		return -1;
	}

    addr = 0;
    addr_offs = 0;
    
    *(u16*)buff = 0;
    if (write_record(f, 2, 0, RECORD_ADDRESS, buff))
        goto cleanup;
    
    while (addr_offs + addr < len)
    {
        if (addr_offs + addr + buff_len < len)
            count = buff_len;
        else
            count = len - addr_offs - addr;

        if (write_record(f, count, addr, RECORD_DATA, (u8*)data + addr_offs + addr))
            goto cleanup;

        addr += count;

        if (addr >= 0x10000 && addr_offs + addr < len)
        {
            addr_offs += 0x10000;
            addr &= 0xFFFF;
            *(u16*)buff = HTON16(addr_offs >> 16);
            if (write_record(f, 2, 0, RECORD_ADDRESS, buff))
                goto cleanup;
        }
    }

    if (write_record(f, 0, 0, RECORD_EOF, NULL))
        goto cleanup;
    
	rc = 0;

cleanup:

    fclose(f);

    return rc;
}

int mcs_file::write_record(
    FILE* f,
    int len, 
    int addr, 
    int type, 
    unsigned char* buff)
{
    int i;
    u8 chksum;
    
    if (len >= 256 || addr >= 0x10000)
        return -1;

    fprintf(f,
        ":%02X%02X%02X%02X",
        len & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        type & 0xFF);

    chksum = (u8)((len & 0xFF) + ((addr >> 8) & 0xFF) + (addr & 0xFF) + (type & 0xFF));

    for (i = 0; i < len; i++)
    {
        fprintf(f, "%02X", buff[i]);
        chksum = (u8)(chksum + buff[i]);
    }

    chksum = (u8)(0x100 - chksum);
    
    fprintf(f, "%02X\n", chksum);

    return 0;
}

int mcs_file::parse_record(
    const char* line,
    int* len, 
    int* addr, 
    int* type, 
    unsigned char* buff, 
    int bufflen)
{
    int linelen, i;
    int sum = 0;

    if (line == NULL)
        return -1;

    linelen = strlen(line);

    if (line == NULL || line[0] != ':' || linelen < 9)
        return -1;

    *len = hex2int(line + 1, 1, &sum);
    *addr = hex2int(line + 3, 2, &sum);
    *type = hex2int(line + 7, 1, &sum);

    if (linelen != 11 + (*len)*2)
        return -1;

    if (*len > bufflen)
        return -1;

    for (i = 0; i < *len; i++)
        buff[i] = (unsigned char)hex2int(line + 9 + i*2, 1, &sum);

    hex2int(line + (linelen - 2), 1, &sum);

    if ((sum &0xFF) != 0)
        // Invalid checksum
        return -1;

    return 0;
}
