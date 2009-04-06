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
#include "chip.h"

///////////////////////////////////////////////////////////
//
// chip
//

chip::chip(char* _name, u32 _id, u32 _mask, int _ir_length, chip_family* _family)
: name(_name), id(_id), mask(_mask), ir_length(_ir_length), family(_family)
{
}

int chip::erase(cable* cbl)
{
    string fn_name;
    chip_database::fn_erase fn;

    if (family->vars.get(strERASE, fn_name))
    {
        msgf(STR_OPERATION_NOT_SUPPORTED);
		return -1;
	}

    fn = g.chips.find_erase_function(fn_name.c_str());
    if (fn == NULL)
    {
        msgf(STR_INVALID_CONFIG_FILE, 0);
		return -1;
	}

    return (fn)(this, cbl);
}

int xc_user(chip *dev, cable *cbl, int user, uint8_t *in, uint8_t *out, int len);

int chip::user(cable *cbl, int user, uint8_t *in, uint8_t *out, int len) {
	
	return xc_user(this,cbl,user,in,out,len);
}

int chip::program(cable* cbl, program_file* stream, int flash)
{
	string fn_name;
    chip_database::fn_program fn;

    if (family->vars.get(flash ? strFLASHPROG : strPROGRAM, fn_name))
    {
		msgf(STR_OPERATION_NOT_SUPPORTED);
        return -1;
	}

    fn = g.chips.find_program_function(fn_name.c_str());
    if (fn == NULL)
    {
		msgf(STR_INVALID_CONFIG_FILE, 0);
        return -1;
	}

    return (fn)(this, cbl, stream);
}

int chip::readback(cable* cbl, u8** data)
{
	string fn_name;
    chip_database::fn_readback fn;

    if (family->vars.get(strREADBACK, fn_name))
	{
		msgf(STR_OPERATION_NOT_SUPPORTED);
        return -1;
	}

    fn = g.chips.find_readback_function(fn_name.c_str());
    if (fn == NULL)
	{
		msgf(STR_INVALID_CONFIG_FILE, 0);
        return -1;
	}
    
    return (fn)(this, cbl, data);
}

///////////////////////////////////////////////////////////
//
// chip_family
//

chip_family::chip_family()
{
}

chip* chip_family::find_chip(u32 id)
{
    chip* dev;

    for (iterator iter = begin(); iter != end(); iter++)
    {
        dev = &(*iter);
        if (dev->id == (id & dev->mask))
            // Found it
            return dev;
    }
    return NULL;
}

///////////////////////////////////////////////////////////
//
// chip_database
//

chip_database::chip_database()
{
}

chip_database::~chip_database()
{
    destroy();
}

void chip_database::destroy()
{
    for (iterator iter = begin(); iter != end(); iter++)
        delete (*iter);
    clear();
}

chip* chip_database::find_chip(u32 id)
{
    chip* dev;
    for(iterator iter = begin(); iter != end(); iter++)
    {
        dev = (*iter)->find_chip(id);
        if (dev)
            // Found it
            return dev;
    }
    return NULL;
}

int chip_database::register_erase_function(const char* name, fn_erase fn)
{
    erase_functions[name] = fn;
    return 0;
}
int chip_database::register_program_function(const char* name, fn_program fn)
{
    program_functions[name] = fn;
    return 0;
}
int chip_database::register_readback_function(const char* name, fn_readback fn)
{
    readback_functions[name] = fn;
    return 0;
}

chip_database::fn_erase chip_database::find_erase_function(const char* name)
{
    map<string, fn_erase>::iterator iter = erase_functions.find(name);
    return iter == erase_functions.end() ? NULL : (*iter).second;
}
chip_database::fn_program chip_database::find_program_function(const char* name)
{
    map<string, fn_program>::iterator iter = program_functions.find(name);
    return iter == program_functions.end() ? NULL : (*iter).second;
}
chip_database::fn_readback chip_database::find_readback_function(const char* name)
{
    map<string, fn_readback>::iterator iter = readback_functions.find(name);
    return iter == readback_functions.end() ? NULL : (*iter).second;
}
