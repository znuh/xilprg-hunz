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

#ifndef _DIGILENT_H_INCLUDED_
#define _DIGILENT_H_INCLUDED_

#include "cable.h"

#ifndef WIN32
#include <usb.h>
#endif

class digilent : public cable
{
public:
    enum
    {
        USB_VENDOR_ID         = 0x1443,
        USB_PRODUCT_ID        = 0x0005
    };

    enum
    {
        CMD_NULL              = 0x00,
        CMD_ENABLE            = 0x01,
        CMD_SET               = 0x02,
        CMD_PUT_TDI_BITS      = 0x03,
        CMD_PUT_TMS_TDI_BITS  = 0x04,
        CMD_GET_TDO_BITS      = 0x05,
        CMD_RECEIVE           = 0x06,

        CMD_FLAG_TDI_HIGH     = 0x10,
        CMD_FLAG_TMS_HIGH     = 0x20,
        CMD_FLAG_RECEIVE      = 0x40
    };

	enum
	{
		BULK_PACKET_DATA_LEN  = 61,
		MAX_BULK_PACKET_BURST = 16,
	};

#pragma pack(1)
    typedef struct
    {
        u8 cmd;
        u16 cbit;
        u8 data[BULK_PACKET_DATA_LEN];
    } bulk_packet;
#pragma pack()

public:
	digilent();
	virtual ~digilent();

    virtual int open();
    virtual int close();

    virtual int get_description(string& desc);
    
    virtual void shift(int num_bits, void* ptdi, void* ptdo, int last);
	virtual void tck_cycle(int n = 1);
	virtual void tms_transition(u32 seq, int cnt);

	virtual void reset_tap_state();

protected:
    virtual int shift_r_start(int state, int num_bits, void* ptdi, void* ptdo, int align, int last);

protected:
    int enable_jtag(int enable);
    int set_tms_tdi_tck(u8 tms, u8 tdi, u8 tck);
    int put_tdi_bits(int cbit, void* snd, int tms, void* rcv);
    int put_tms_tdi_bits(int cbit, void* snd, void* rcv);
    int get_tdo_bits(int cbit, int tms, int tdi, void* rcv);

protected:
    void init_bulk_packet(bulk_packet* packet, int cmd, int cbit);
	int queue_wr_bulk(bulk_packet* packet, int flush = 0);
    int write_bulk(bulk_packet* packet, int n = 1);
    int read_bulk(bulk_packet* packet);
	int flush_wr_bulk();
	
protected:
#ifdef WIN32
    int find_digilent_device_name(char* device);
#else
	struct usb_device* find_digilent_device();
#endif

#ifdef WIN32
    HANDLE handle_in;
    HANDLE handle_out;
#else
    usb_dev_handle* handle;
#endif

    u8 tms;
    u8 tdo;

    int pending_shift_r_read;

	bulk_packet queued_wr_packets[MAX_BULK_PACKET_BURST];
	int queued_wr_packet_count;
};

#endif
