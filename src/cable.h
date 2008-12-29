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

#ifndef _PROGRAMMER_H_INCLUDED_
#define _PROGRAMMER_H_INCLUDED_

// TAP states
#define TAPSTATE_RESET      0x00
#define TAPSTATE_IDLE       0x01
#define TAPSTATE_SELECTDR   0x02
#define TAPSTATE_CAPTUREDR  0x03
#define TAPSTATE_SHIFTDR    0x04
#define TAPSTATE_EXIT1DR    0x05
#define TAPSTATE_PAUSEDR    0x06
#define TAPSTATE_EXIT2DR    0x07
#define TAPSTATE_UPDATEDR   0x08
#define TAPSTATE_IRSTATES   0x09
#define TAPSTATE_SELECTIR   0x09
#define TAPSTATE_CAPTUREIR  0x0A
#define TAPSTATE_SHIFTIR    0x0B
#define TAPSTATE_EXIT1IR    0x0C
#define TAPSTATE_PAUSEIR    0x0D
#define TAPSTATE_EXIT2IR    0x0E
#define TAPSTATE_UPDATEIR   0x0F
#define TAPSTATE_UNKNOWN    0xFF

#define ALL_ZEROS           (u8*)NULL
#define ALL_ONES            (u8*)-1

class program_file;
class chip;

class device_chain_item
{
public:
	device_chain_item(u32 _id, chip* _dev)
	: id(_id), dev(_dev)
	{
	}

	u32 id;
	chip* dev;
};

class device_chain : public vector<device_chain_item>
{
};

class cable
{
public:
    cable();
    virtual ~cable();

    virtual void test() {}

    virtual int open() = 0;
    virtual int close() = 0;

    virtual int get_description(string& desc);

    virtual void set_tdi(int);
    virtual void set_tck(int);
    virtual void set_tms(int);
    virtual int get_tdo();

    virtual int detect_chain();
	virtual int detect_chain_ids(vector<u32>& ids);
    
    virtual int shift_dr(int num_bits, void* ptdi, void* ptdo = NULL, int align = 0);
    virtual int shift_ir(void* ptdi, void* ptdo = NULL);
    
    virtual int shift_dr_start(int num_bits, void* ptdi, void* ptdo = NULL, int align = 0);
    virtual int shift_dr_cont(int num_bits, void* ptdi, void* ptdo, int last);
    
	virtual void tck_cycle(int n = 1);

    virtual void tms_transition(u32 seq, int cnt);
    virtual void shift(int num_bits, void* tdi, void* tdo, int last);

    virtual int goto_tap_state(int state_in);
	virtual void reset_tap_state();

    int tms_tap_state(int state_in, u32* tms_seq);

protected:
    virtual int shift_r_start(int state, int num_bits, void* ptdi, void* ptdo, int align, int last);
    virtual int shift_r_cont(int num_bits, void* ptdi, void* ptdo, int last);

public:
    static cable* factory(const char* s);

protected:
    int current_tap_state;

public:
    int endir_state;
    int enddr_state;
};

#endif
