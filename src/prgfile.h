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

#ifndef _PRGFILE_H_INCLUDED_
#define _PRGFILE_H_INCLUDED_

///////////////////////////////////////////////////////////
//
// Base class 
//

class program_file  
{
public:
	program_file();
	virtual ~program_file();

public:
    virtual int load(const char*);
	virtual int save(const char*, void*, int);
    virtual void clear();

public:
    virtual int get_length();
    virtual int get_bit_length();
    virtual u8* get_stream();

protected:
    u8* data;
    u32 length;
};

///////////////////////////////////////////////////////////
//
// BIN file 
//

class bin_file : public program_file  
{
public:
    virtual int load(const char*);
	virtual int save(const char* file, void* data, int len);
};

///////////////////////////////////////////////////////////
//
// BIT file 
//

class bit_file : public program_file  
{
public:
    virtual int load(const char*);

protected:
    int read_field(FILE* f, char* field, size_t maxlen);
    int alloc_read_data(FILE* f);
};

///////////////////////////////////////////////////////////
//
// MCS file 
//

class mcs_file : public program_file  
{
public:
	enum
	{
		RECORD_DATA		= 0,
		RECORD_EOF      = 1,
		RECORD_ADDRESS  = 4,
	};

public:
    virtual int load(const char*);
	virtual int save(const char* file, void* data, int len);

protected:
    int write_record(
            FILE* f,
            int len, 
            int addr, 
            int type, 
            unsigned char* buff);
	int parse_record(
			const char* line,
			int* len, 
			int* addr, 
			int* type, 
			unsigned char* buff, 
			int bufflen);
};

#endif
