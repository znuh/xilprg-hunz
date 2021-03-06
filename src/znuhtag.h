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
Copyright (c) 2008 Benedikt Heinz <Zn000h@googlemail.com>
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

#ifndef _ZNUHTAG_H_INCLUDED_
#define _ZNUHTAG_H_INCLUDED_

#include "cable.h"

#include <usb.h>

class znuhtag : public cable
{
public:
    enum
    {
        USB_VENDOR_ID         = 0x4711,
        USB_PRODUCT_ID        = 0x0023
    };

    enum
    {
		CMD_SEQ				= 0x80,
		CMD_JTAG				= 0x40,
		
		CMD_JTAG_TMS		= 0x20,
		CMD_JTAG_LAST		= 0x04,
		CMD_JTAG_TDO			= 0x02
    };
	
public:
	znuhtag(unsigned int _num);
	virtual ~znuhtag();

    virtual int open();
    virtual int close();

    virtual int get_description(string& desc);
    
    virtual void shift(int num_bits, void* ptdi, void* ptdo, int last);
	virtual void tck_cycle(int n = 1);
	virtual void tms_transition(u32 seq, int cnt);

protected:
	struct usb_device* find_device();

	void bindump(unsigned char *d, int len);
	void hexdump(unsigned char *d, int len);
	
    usb_dev_handle* handle;

	void usb_transfer(unsigned char *snd, unsigned char *rcv, int bitlen);
	void usb_command(int tms, void *in, void *out, int bitlen, int last);

    unsigned int num;
};

#endif
