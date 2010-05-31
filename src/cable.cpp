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
#include "cable.h"
#include "prgfile.h"
#include "utils.h"
#include "chip.h"
#include "cmdline.h"
#include "digilent.h"
#include "znuhtag.h"
#include "amontec.h"
#include "parport.h"

cable::cable()
: current_tap_state(TAPSTATE_UNKNOWN),
  endir_state(TAPSTATE_IDLE), enddr_state(TAPSTATE_IDLE)
{
}

cable::~cable()
{
}

int cable::get_description(string&)
{
    return -1;
}

void cable::set_tdi(int)
{
}

void cable::set_tck(int)
{
}

void cable::set_tms(int)
{
}

int cable::get_tdo()
{
    return 0;
}

void cable::tck_cycle(int n)
{
    while (n--)
    {
		set_tck(1);
        set_tck(0);
    }
}

void cable::tms_transition(u32 seq, int cnt)
{
    while (cnt--)
    {
        set_tms((seq & 1) ? 1 : 0);
        tck_cycle();
        seq >>= 1;
    }
}

int cable::tms_tap_state(int state_in, u32* tms_seq)
{
	u32 seq = 0;
	int cnt = 0;
    int tms;

    if (state_in == TAPSTATE_RESET)
    {
        // If RESET, always perform TMS reset sequence to reset/sync TAPs
		seq = 0xFF;
		cnt = 5;
        current_tap_state = TAPSTATE_RESET;
    }
    else 
    if ((state_in != current_tap_state) &&
        (((state_in == TAPSTATE_EXIT2DR) && (current_tap_state != TAPSTATE_PAUSEDR)) ||
        ((state_in == TAPSTATE_EXIT2IR ) && (current_tap_state != TAPSTATE_PAUSEIR))))
        // Illegal TAP state_in path
        return -1;
    else
    {
		// Already in target state_in.  Do nothing except when in DRPAUSE
		// or in IRPAUSE to comply with SVF standard
        if (state_in == current_tap_state &&
			(state_in == TAPSTATE_PAUSEDR || state_in == TAPSTATE_PAUSEIR))
        {
        	seq <<= 1;
			seq |= 1;
			cnt++;
			current_tap_state = state_in == TAPSTATE_PAUSEDR ? TAPSTATE_EXIT2DR : TAPSTATE_EXIT2IR;
        }

        // Perform TAP state_in transitions to get to the target state_in
        while (state_in != current_tap_state)
        {
			tms = 0;
            switch (current_tap_state)
            {
            case TAPSTATE_RESET:
                current_tap_state = TAPSTATE_IDLE;
                break;

            case TAPSTATE_IDLE:
            	tms = 1;
                current_tap_state = TAPSTATE_SELECTDR;
                break;

            case TAPSTATE_SELECTDR:
                if (state_in >= TAPSTATE_IRSTATES )
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_SELECTIR;
                }
                else
                {
                    current_tap_state = TAPSTATE_CAPTUREDR;
                }
                break;

            case TAPSTATE_CAPTUREDR:
                if (state_in == TAPSTATE_SHIFTDR )
                {
                    current_tap_state = TAPSTATE_SHIFTDR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_EXIT1DR;
                }
                break;

            case TAPSTATE_SHIFTDR:
                tms = 1;
                current_tap_state = TAPSTATE_EXIT1DR;
                break;

            case TAPSTATE_EXIT1DR:
                if (state_in == TAPSTATE_PAUSEDR )
                {
                    current_tap_state = TAPSTATE_PAUSEDR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_UPDATEDR;
                }
                break;

            case TAPSTATE_PAUSEDR:
                tms = 1;
                current_tap_state = TAPSTATE_EXIT2DR;
                break;

            case TAPSTATE_EXIT2DR:
                if (state_in == TAPSTATE_SHIFTDR )
                {
                    current_tap_state = TAPSTATE_SHIFTDR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_UPDATEDR;
                }
                break;

            case TAPSTATE_UPDATEDR:
                if (state_in == TAPSTATE_IDLE)
                {
                    current_tap_state = TAPSTATE_IDLE;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_SELECTDR;
                }
                break;

            case TAPSTATE_SELECTIR:
                current_tap_state = TAPSTATE_CAPTUREIR;
                break;

            case TAPSTATE_CAPTUREIR:
                if (state_in == TAPSTATE_SHIFTIR )
                {
                    current_tap_state = TAPSTATE_SHIFTIR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_EXIT1IR;
                }
                break;

            case TAPSTATE_SHIFTIR:
                tms = 1;
                current_tap_state = TAPSTATE_EXIT1IR;
                break;

            case TAPSTATE_EXIT1IR:
                if (state_in == TAPSTATE_PAUSEIR )
                {
                    current_tap_state = TAPSTATE_PAUSEIR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_UPDATEIR;
                }
                break;

            case TAPSTATE_PAUSEIR:
                tms = 1;
                current_tap_state = TAPSTATE_EXIT2IR;
                break;

            case TAPSTATE_EXIT2IR:
                if (state_in == TAPSTATE_SHIFTIR )
                {
                    current_tap_state = TAPSTATE_SHIFTIR;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_UPDATEIR;
                }
                break;

            case TAPSTATE_UPDATEIR:
                if (state_in == TAPSTATE_IDLE )
                {
                    current_tap_state = TAPSTATE_IDLE;
                }
                else
                {
                    tms = 1;
                    current_tap_state = TAPSTATE_SELECTDR;
                }
                break;

            default:
                current_tap_state = state_in;
                return -1;
            }

            seq |= (tms << cnt);
            cnt++;
        }
    }

    *tms_seq = seq;

    return cnt;
}

void cable::reset_tap_state()
{
    goto_tap_state(TAPSTATE_RESET);
}

int cable::goto_tap_state(int state_in)
{
    int tms_cnt;
    u32 tms_seq;

    tms_cnt = tms_tap_state(state_in, &tms_seq);
    if (tms_cnt <= 0)
        return tms_cnt;

    tms_transition(tms_seq, tms_cnt);

    return 0;
}

void cable::shift(int num_bits, void* ptdi, void* ptdo, int last)
{
    u8 tdi_byte, tdo_byte;
    int i;
	u8* _ptdi = (u8*)ptdi;
	u8* _ptdo = (u8*)ptdo;
	
    while (num_bits)
    {
        // Process on a byte-basis, LSB bytes and bits first
        if (_ptdi == ALL_ZEROS)
            tdi_byte = 0;
		else
        if (_ptdi == ALL_ONES)
            tdi_byte = 0xFF;
        else
            tdi_byte = *_ptdi++;

        tdo_byte = 0;
        for (i = 0; (num_bits && (i < 8)); i++)
        {
            num_bits--;
            if (last && !num_bits)
                // Exit Shift-DR state
                set_tms(1);

            // Set the new TDI value
            set_tdi(tdi_byte & 1);
            tdi_byte >>= 1;

			if (_ptdo)
                // Save TDO bit
                tdo_byte |= get_tdo() << i;

            // Clock cycle
            tck_cycle(1);
        }

        //* Save TDO byte
        if (_ptdo)
            *_ptdo++ = tdo_byte;
    }
}

int cable::detect_chain_ids(vector<u32>& ids)
{
	int cnt = 0;
	u32 id;

    g.sel_dev = -1;
    
	goto_tap_state(TAPSTATE_RESET);

    shift_dr_start(32, ALL_ZEROS, &id);
    while (id != 0 && id != (u32)-1)
    {
        ids.push_back(id);
        cnt++;

        id = 0;
        shift_dr_cont(32, ALL_ZEROS, &id, 0);
	}
	
    goto_tap_state(TAPSTATE_RESET);

	return cnt;
}

int cable::detect_chain()
{
    u32 id;
    vector<u32> ids;
	bool unknown = false;
	unsigned int i;
	chip* dev;
	string desc;

	g.dev_chain.clear();

	msgf(STR_IDENTIFY_CHAIN);

	detect_chain_ids(ids);
		
	for (i = 1; i <= ids.size(); i++)
    {
		id = ids[ids.size() - i];
		dev = g.chips.find_chip(id);
		if (dev && dev->family)
		{
			g.dev_chain.push_back(device_chain_item(id, dev));

            dev->family->vars.get(strDESC, desc);
			printf("\'%d\' %s %s\n", i, dev->name.c_str(), desc.c_str());
		}
		else
		{
            msgf(STR_UNKNOWN_DEVICE_IDCODE, i, id);
			unknown = true;
		}
    }

    if (unknown)
     {
		g.dev_chain.clear();
        msgf(STR_UNKNOWN_DEVICES);
        return -1;
	}

	if (g.sel_dev >= (int)g.dev_chain.size())
	    g.sel_dev = 0;

    msgf(STR_DEVICES_FOUND, g.dev_chain.size());
    
    return g.dev_chain.size();
}

int cable::shift_r_start(int state, int num_bits, void* ptdi, void* ptdo, int align, int last)
{
    int hr = 0, tr = 0;
    unsigned int i;

    if (state == TAPSTATE_SHIFTDR)
    {
	    // Calculate HEADER and TRAILER
        if (g.sel_dev >= 0)
        {
            hr = g.dev_chain.size() - g.sel_dev - 1;
            tr = g.sel_dev;
        }

        if (align)
	    {
		    hr -= tr;
		    while (hr <= 0)
			    hr += align;
	    }
    }
    else
    if (state == TAPSTATE_SHIFTIR)
    {
        if (g.sel_dev < 0)
            return -1;

        // Calculate HIR
        for (i = g.sel_dev + 1; i < g.dev_chain.size(); i++)
	        hr += g.dev_chain[i].dev->ir_length;
    }
    else
        return -1;

    // Goto Shift-DR or Shift-IR
    goto_tap_state(state);

    // Shift header
    if (hr)
        shift(hr, state == TAPSTATE_SHIFTIR ? ALL_ONES : ALL_ZEROS, NULL, 0);

    return shift_r_cont(num_bits, ptdi, ptdo, last);
}

int cable::shift_r_cont(int num_bits, void* ptdi, void* ptdo, int last)
{
    int tr = 0;
    int i;
    
    if (num_bits == 0)
        last = 1;

    if (g.sel_dev >= (int)g.dev_chain.size())
        return -1;

    // Calculate TRAILER
    if (current_tap_state == TAPSTATE_SHIFTDR)
        tr = g.sel_dev < 0 ? 0 : g.sel_dev;
    else
    if (current_tap_state == TAPSTATE_SHIFTIR)
    {
        for (i = 0; i < g.sel_dev; i++)
	        tr += g.dev_chain[i].dev->ir_length;
    }
    
    // Shift data
    shift(num_bits, ptdi, ptdo, tr == 0 && last);

    // Shift trailer
    if (last && tr)
        shift(tr, current_tap_state == TAPSTATE_SHIFTIR ? ALL_ONES : ALL_ZEROS, NULL, 1);

    if (last)
    {
        // Update TAP state:  Shift->Exit
        current_tap_state++;
        
        switch (current_tap_state)
        {
        case TAPSTATE_EXIT1IR:
            goto_tap_state(endir_state);
            break;

        case TAPSTATE_EXIT1DR:
            goto_tap_state(enddr_state);
            break;
        
        default:
            goto_tap_state(TAPSTATE_IDLE);
            break;
        }
    }

    return 0;
}

int cable::shift_dr_start(int num_bits, void* ptdi, void* ptdo, int align)
{
    return shift_r_start(TAPSTATE_SHIFTDR, num_bits, ptdi, ptdo, align, 0);
}

int cable::shift_dr_cont(int num_bits, void* ptdi, void* ptdo, int last)
{
    return shift_r_cont(num_bits, ptdi, ptdo, last);
}

int cable::shift_dr(int num_bits, void* ptdi, void* ptdo, int align)
{
    return shift_r_start(TAPSTATE_SHIFTDR, num_bits, ptdi, ptdo, align, 1);
}

int cable::shift_ir(void* ptdi, void* ptdo)
{
	if (g.sel_dev >= (int)g.dev_chain.size())
        return -1;

    return shift_r_start(
        TAPSTATE_SHIFTIR, 
        g.dev_chain[g.sel_dev].dev->ir_length, 
        ptdi, 
        ptdo, 
        0, 
        1);
}

cable* cable::factory(const char* s)
{
    cable* cbl = NULL;
    int argc;
    char** argv = NULL;
    int addr;

    if (*s == 0)
        return NULL;

    parse_cmdline_args(s, &argc, &argv);

    if (argc == 0)
        return NULL;

    if (stricmp(argv[0], "xil3") == 0) {
        if (argc == 1)
            addr = 0x378;
        else {
            if (str2num(argv[1], &addr))
                goto cleanup;
        }
        
        cbl = new parport((unsigned int)addr);
    }
    else if (stricmp(argv[0], "dusb") == 0)
        cbl = new digilent;
    else if (stricmp(argv[0],"znuhtag") == 0) {
	    int num=0;
	    if(argc > 1)
		    str2num(argv[1], &num);
	cbl = new znuhtag(num);
    }
    else if (stricmp(argv[0],"amontec") == 0) {
	    int speed = 0;
	    if (argc == 1)
		    str2num(argv[1], &speed);
	cbl = new amontec(speed);
    }
    else {
        msgf(STR_INVALID_CABLE_DEF);
        return NULL;
    }

cleanup:

    return cbl;
}
