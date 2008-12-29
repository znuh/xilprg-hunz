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
#include "parport.h"

static unsigned long delay=0;

parport::parport(unsigned int _addr)
: addr(_addr)
{
}

parport::~parport()
{
}

int parport::open()
{
    if (enable_user_mode_io() < 0)
        return -1;
    
	if(getenv("PPDELAY"))
		delay=atoi(getenv("PPDELAY"));
	
    data = 0x10;

    reset_tap_state();

    return 0;
}

int parport::close()
{
    return 0;
}

int parport::get_description(string& desc)
{
    char s[256];
    sprintf(s, "Xilinx Parallel Cable III @ 0x%X", addr);
    desc = s;
    return 0;
}

void parport::set_tdi(int bit)
{
    if (bit) data |= 1; else data &= ~1;
    outb(data, addr + 0);
}

void parport::set_tck(int bit)
{
    if (bit) data |= 2; else data &= ~2;
    outb(data, addr + 0);
	usleep(delay);
}

void parport::set_tms(int bit)
{
    if (bit) data |= 4; else data &= ~4;
    outb(data, addr + 0);
}

int parport::get_tdo()
{
    return inb(addr + 1) & 0x10 ? 1 : 0;
}
