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

#ifndef _CHIP_H_INCLUDED_
#define _CHIP_H_INCLUDED_

#include "vartable.h"

class program_file;
class cable;
class chip_family;

class chip
{
public:
	chip(char* _name, u32 _id, u32 _mask, int _ir_length, chip_family* _family);
   
    int erase(cable* cbl);
	int program(cable* cbl, program_file* stream);
	int readback(cable* cbl, u8** data);

public:
	string name;
	u32 id;
	u32 mask;
	int ir_length;
	chip_family* family;
};

class chip_family : public vector<chip>
{
public:
    chip_family();
    chip* find_chip(u32 id);

public:
	variable_table vars;
};

class chip_database : public vector<chip_family*>
{
public:
    typedef int (*fn_erase)(chip*, cable*);
    typedef int (*fn_program)(chip*, cable*, program_file*);
    typedef int (*fn_readback)(chip*, cable*, u8**);

public:
	chip_database();
    virtual ~chip_database();

public:
	chip* find_chip(u32 id);
    void destroy();

public:
    int register_erase_function(const char* name, fn_erase fn);
    int register_program_function(const char* name, fn_program fn);
    int register_readback_function(const char* name, fn_readback fn);

    fn_erase find_erase_function(const char* name);
    fn_program find_program_function(const char* name);
    fn_readback find_readback_function(const char* name);

protected:
    map<string, fn_erase> erase_functions;
    map<string, fn_program> program_functions;
    map<string, fn_readback> readback_functions;
};

#endif
