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

#ifndef _XILPRG_H_INCLUDED_
#define _XILPRG_H_INCLUDED_

#include "platform.h"
#include "chip.h"
#include "cable.h"
#include "vartable.h"
#include "utils.h"
#include "strtable.h"

#define VERSION_MAJOR   0
#define VERSION_MINOR   5
#define VERSION_STR     "0.5"   

typedef struct 
{
    chip_database chips;
    device_chain dev_chain;
    int sel_dev;
	variable_table vars;
} globals;
extern globals g;

cable* open_cable(int detect);
int close_cable(cable*);
chip* select_device_in_chain(int index);

int xc_user(chip *dev, cable *cbl, int user, uint8_t *in, uint8_t *out, int len);

int spi_readback(chip *dev, cable *prg, u8 **data);
int spi_program(chip* dev, cable* prg, program_file* file);

extern const char* strNAME;
extern const char* strDESC;
extern const char* strTYPE;
extern const char* strFPGA;
extern const char* strCPLD;
extern const char* strPROM;
extern const char* strIRLEN;
extern const char* strERASE;
extern const char* strPROGRAM;
extern const char* strREADBACK;
extern const char* strCABLE;

#endif
