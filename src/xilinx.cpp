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
#include "chip.h"
#include "prgfile.h"
#include <assert.h>

#define FAMILY_XC3S_ID			0x01400000	
#define FAMILY_XC3SE_ID			0x01C00000
#define FAMILY_XC3A_ID			0x02200000
#define FAMILY_XC3AN_ID		0x02600000
#define FAMILY_XC2VP_ID			0x01200000
#define FAMILY_XC4VLX_ID		0x01600000
#define FAMILY_XC4VSX_ID		0x02000000
#define FAMILY_XC4VFX_ID		0x01E00000
#define FAMILY_MASK				0x0FE00000

#define DUMMY					0xFFFFFFFF
#define FLUSH					0x00000000
#define SYNC					0x66AA9955
#define WRITE_CMD				0x8001000C
#define WRITE_COR				0x8004800C
#define CMD_AHIGH				0x10000000
#define CMD_START				0xA0000000
#define CMD_RCRC				0xE0000000

static char strISPEN[] = ".ISPEN";
static char strFADDR[] = ".FADDR";
static char strFERASE[] = ".FERASE";
static char strFDATA0[] = ".FDATA0";
static char strFVFY0[] = ".FVFY0";
static char strFVFY1[] = ".FVFY1";
static char strFPGM[] = ".FPGM";
static char strJPROGRAM[] = ".JPROGRAM";
static char strJSTART[] = ".JSTART";
static char strJSHUTDOWN[] = ".JSHUTDOWN";
static char strCFG_IN[] = ".CFG_IN";
static char strUSER[] = ".USER";

int xcf_erase(chip* dev, cable* prg)
{
	u16 d16;
	u8 ir_cap;
	u32 opISPEN, opFADDR, opFERASE;
    int rc = -1;

	if (dev->family->vars.get(strISPEN, &opISPEN) ||
		dev->family->vars.get(strFADDR, &opFADDR) ||
		dev->family->vars.get(strFERASE, &opFERASE))
		return -1;

	prg->reset_tap_state();

	// BYPASS
	prg->shift_ir(ALL_ONES, &ir_cap);
	if (ir_cap != 0x01)
    {
        //attribute INSTRUCTION_CAPTURE : entity is "XXXXX001";
        //-- IR[7:6] Internal Erase/Program Error (10=success; 01=fail; 00/11=N/A)
        //-- IR[5] Internal Erase/Program Status (1=ready; 0=busy)
        //-- IR[4] ISP mode (1=in-system programming mode; 0=normal download mode)
        //-- IR[3] JTAG read-protection (1=secured; 0=unsecured)
        msgf(STR_DEVICE_WRITE_PROTECTED);
        goto cleanup;
    }
	
	prg->shift_ir(&opISPEN);
    d16 = 0x34;
    prg->shift_dr(6, &d16);

	prg->shift_ir(&opFADDR);
	d16 = 1;
	prg->shift_dr(16, &d16);
	prg->tck_cycle(2);
  
	prg->shift_ir(&opFERASE);
	
	prg->tck_cycle(3000000);
	//prg->tck_cycle(15000000);
	
	//BYPASS
    prg->shift_ir(ALL_ONES);
	
    rc = 0;

cleanup:

	prg->reset_tap_state();

	return rc;
}

int xcf_program(chip* dev, cable* prg, program_file* file)
{
	u16 d16;
	int i, bits_len, cnt, rc = -1;
	u8 ones[512];
	u8 opISPEN, opFADDR, opFDATA0, opFPGM, op, ir_cap;

	if (dev->family->vars.get(strISPEN, &opISPEN) ||
		dev->family->vars.get(strFADDR, &opFADDR) ||
		dev->family->vars.get(strFDATA0, &opFDATA0) ||
		dev->family->vars.get(strFPGM, &opFPGM))
		return -1;

    bits_len = file->get_bit_length();
    if (bits_len == 0)
        return -1;

	memset(ones, 0xFF, sizeof(ones));

	prg->reset_tap_state();

	// BYPASS
	prg->shift_ir(ALL_ONES, &ir_cap);
	if (ir_cap != 0x01)
    {
        //attribute INSTRUCTION_CAPTURE : entity is "XXXXX001";
        //-- IR[7:6] Internal Erase/Program Error (10=success; 01=fail; 00/11=N/A)
        //-- IR[5] Internal Erase/Program Status (1=ready; 0=busy)
        //-- IR[4] ISP mode (1=in-system programming mode; 0=normal download mode)
        //-- IR[3] JTAG read-protection (1=secured; 0=unsecured)
        msgf(STR_DEVICE_WRITE_PROTECTED);
        goto cleanup;
    }

	prg->shift_ir(&opISPEN);
	op = 0x34;
	prg->shift_dr(6, &op);

    for(i = 0; i < bits_len; i += 4096)
    {
        cnt = bits_len - i > 4096 ? 4096 : bits_len - i;
        
		prg->shift_ir(&opFDATA0);
        if (cnt == 4096)
            prg->shift_dr(cnt, file->get_stream() + i/8);
        else
        {
            // Extra padding
            prg->shift_dr_start(cnt, file->get_stream() + i/8);
            prg->shift_dr_cont(4096 - cnt, ones, NULL, 1);
        }

		prg->shift_ir(&opFADDR);
		d16 = (u16)(i / 128);
		prg->shift_dr(16, &d16);

		prg->tck_cycle(2);

    	prg->shift_ir(&opFPGM);

		prg->tck_cycle(5000);
		//prg->tck_cycle(14000);
    } 

  	//BYPASS
    prg->shift_ir(ALL_ONES);

	rc = 0;

cleanup:
	
	prg->reset_tap_state();

    return rc;
}

int xcf_readback(chip* dev, cable* prg, u8** data)
{
	u16 d16;
	int i, bits_len, rc = -1;
    int left;
	u8 opISPEN, opFADDR, opFVFY0, op, ir_cap;
    int prom_size;

    *data = NULL;

	if (dev->family->vars.get(strISPEN, &opISPEN) ||
		dev->family->vars.get(strFADDR, &opFADDR) ||
		dev->family->vars.get(strFVFY0, &opFVFY0))
		return -1;

    prom_size = (dev->id & 0x0000F000) >> 12;
    
    prg->reset_tap_state();

	// BYPASS
	prg->shift_ir(ALL_ONES, &ir_cap);
	if (ir_cap != 0x01)
    {
        //attribute INSTRUCTION_CAPTURE : entity is "XXXXX001";
        //-- IR[7:6] Internal Erase/Program Error (10=success; 01=fail; 00/11=N/A)
        //-- IR[5] Internal Erase/Program Status (1=ready; 0=busy)
        //-- IR[4] ISP mode (1=in-system programming mode; 0=normal download mode)
        //-- IR[3] JTAG read-protection (1=secured; 0=unsecured)
        msgf(STR_DEVICE_READ_PROTECTED);
        goto cleanup;
    }

	prg->shift_ir(&opISPEN);
	op = 0x34;
	prg->shift_dr(6, &op);

    bits_len = 4096 << (prom_size + 4);
    *data = (u8*)malloc(bits_len/8);

    left = bits_len;
	for(i = 0; i < bits_len; i += 8192)
    {
        prg->shift_ir(&opFADDR);
        d16 = (u16)(i / 128);
        prg->shift_dr(16, &d16);
		prg->tck_cycle(2);
        prg->shift_ir(&opFVFY0);
		prg->tck_cycle(51);
        prg->shift_dr(8192, NULL, (*data) + i/8);
    } 

	rc = bits_len/8;

cleanup:

	prg->reset_tap_state();

	if (rc < 0)
		if (*data)
		{
			free(*data);
			*data = NULL;
		}

    return rc;
}

int xcv_program(chip* dev, cable* prg, program_file* file)
{
    s32 cfg_in_data[11];
    u16 opJSTART, opCFG_IN;
    u16 ir_cap;
    int rc = -1;

	if (dev->family->vars.get(strJSTART, &opJSTART) ||
		dev->family->vars.get(strCFG_IN, &opCFG_IN))
		return -1;

    prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = FLUSH;
    cfg_in_data[1] = SYNC;
    cfg_in_data[2] = WRITE_COR;
    cfg_in_data[3] = 0xBCFD0500;    // COR data sets SHUTDOWN = 1
    cfg_in_data[4] = WRITE_CMD;
    cfg_in_data[5] = CMD_START;
	cfg_in_data[6] = WRITE_CMD;
	cfg_in_data[7] = CMD_RCRC;
	cfg_in_data[8] = FLUSH;
    prg->shift_dr(288, &cfg_in_data, NULL, 32);

	prg->reset_tap_state();

	prg->shift_ir(&opJSTART);
	prg->shift_dr(13, ALL_ZEROS);

	prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = FLUSH;
    cfg_in_data[1] = SYNC;
    cfg_in_data[2] = WRITE_CMD;
    cfg_in_data[3] = CMD_AHIGH;
    cfg_in_data[4] = WRITE_COR;
    cfg_in_data[5] = 0xFFFC0500;	// COR data sets SHUTDOWN = 0
	cfg_in_data[6] = WRITE_CMD;
	cfg_in_data[7] = CMD_START;
	cfg_in_data[8] = WRITE_CMD;
	cfg_in_data[9] = CMD_RCRC;
	cfg_in_data[10] = FLUSH;
    prg->shift_dr(352, &cfg_in_data, NULL, 32);

	prg->reset_tap_state();
	
	prg->shift_ir(&opJSTART);
	prg->shift_dr(13, ALL_ZEROS);

	prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = FLUSH;			// Flush
    cfg_in_data[1] = SYNC;			// Synchronization Word
    cfg_in_data[2] = WRITE_COR;		// Write to COR
    cfg_in_data[3] = 0xB4FD0500;	// COR data sets SHUTDOWN = 1
    cfg_in_data[4] = WRITE_CMD;		// Write to CMD
    cfg_in_data[5] = CMD_START;		// START command
	cfg_in_data[6] = WRITE_CMD;		// Write to CMD
	cfg_in_data[7] = CMD_RCRC;		// RCRC command
	cfg_in_data[8] = FLUSH;			// Flush
    prg->shift_dr(288, &cfg_in_data, NULL, 32);

	prg->reset_tap_state();
	
	prg->shift_ir(&opJSTART);
	prg->shift_dr(13, ALL_ZEROS);

	prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = FLUSH;			// Flush
    prg->shift_dr_start(32, &cfg_in_data, NULL, 32);
    prg->shift_dr_cont(file->get_bit_length(), file->get_stream(), NULL, 1);
    
    prg->reset_tap_state();
    
    prg->shift_ir(&opJSTART);
	prg->shift_dr(13, ALL_ZEROS);

    prg->reset_tap_state();
    
    //BYPASS
    prg->shift_ir(ALL_ONES, &ir_cap);
    if ((ir_cap & 4) == 0)
    {
        //attribute INSTRUCTION_CAPTURE : entity is "XXX01";
        //-- Bit 4 of instruction capture is PROGRAM. 
        //-- Bit 3 is INIT.  
        //-- Bit 2 is DONE.
        msgf(STR_DONE_BIT_ERROR);
        goto cleanup;
    }

    rc = 0;

cleanup:

	prg->reset_tap_state();
	
    return rc;
}

int xc2v_program(chip* dev, cable* prg, program_file* file)
{
    s32 cfg_in_data[6];
	u16 opJSTART, opJPROGRAM, opCFG_IN, opJSHUTDOWN;
    u16 ir_cap;
    int spartan3, virtex2p;
    int rc = -1;

	if (dev->family->vars.get(strJSTART, &opJSTART) ||
		dev->family->vars.get(strJSHUTDOWN, &opJSHUTDOWN) ||
		dev->family->vars.get(strCFG_IN, &opCFG_IN))
		return -1;

    spartan3 = ((dev->id & FAMILY_MASK) == FAMILY_XC3S_ID) ||
               ((dev->id & FAMILY_MASK) == FAMILY_XC3SE_ID) ||
		((dev->id & FAMILY_MASK) == FAMILY_XC3A_ID) ||
               ((dev->id & FAMILY_MASK) == FAMILY_XC3AN_ID);

	virtex2p = (dev->id & FAMILY_MASK) == FAMILY_XC2VP_ID;

	if (spartan3 &&	dev->family->vars.get(strJPROGRAM, &opJPROGRAM))
		return -1;

    prg->reset_tap_state();

    if (spartan3)
    {
        prg->shift_ir(&opJPROGRAM);
        prg->tck_cycle(10000);
    }

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = DUMMY;			// Dummy Word
    cfg_in_data[1] = SYNC;			// Synchronization Word
    cfg_in_data[2] = WRITE_CMD;		// CMD Write Packet Header
    cfg_in_data[3] = CMD_RCRC;		// RCRC
    cfg_in_data[4] = FLUSH;			// Flush
    cfg_in_data[5] = FLUSH;			// Flush
    prg->shift_dr(192, &cfg_in_data, NULL, 32);

    prg->shift_ir(&opJSHUTDOWN);

    prg->tck_cycle(12);

    prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[1] = FLUSH;     // Flush
    cfg_in_data[2] = FLUSH;		// Flush
    prg->shift_dr(64, &cfg_in_data, NULL, 32);

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = WRITE_CMD; // CMD Write Packet Header
    cfg_in_data[1] = CMD_AHIGH;	// AGHIGH
    cfg_in_data[2] = FLUSH;		// Flush
    cfg_in_data[3] = FLUSH;		// Flush

	if (virtex2p)
		prg->enddr_state = TAPSTATE_PAUSEDR;

    prg->shift_dr_start(128, &cfg_in_data, NULL, 32);
    prg->shift_dr_cont(file->get_bit_length(), file->get_stream(), NULL, 1);
    
    prg->reset_tap_state();
    prg->goto_tap_state(TAPSTATE_IDLE);

    prg->shift_ir(&opJSTART);

    prg->tck_cycle(12);

	if (virtex2p)
		prg->enddr_state = TAPSTATE_IDLE;
    
    //BYPASS
    prg->shift_ir(ALL_ONES, &ir_cap);
    if ((ir_cap & 0x20) == 0)
    {
        //attribute INSTRUCTION_CAPTURE : entity is "XXXX01";
        //-- Bit 5 is 1 when DONE is released (part of startup sequence)
        //-- Bit 4 is 1 if house-cleaning is complete
        //-- Bit 3 is ISC_Enabled
        //-- Bit 2 is ISC_Done
        msgf(STR_DONE_BIT_ERROR);
        goto cleanup;
    }

    rc = 0;

cleanup:

	prg->reset_tap_state();
	
    return rc;
}

int xc4v_program(chip* dev, cable* prg, program_file* file)
{
    s32 cfg_in_data[8];
	u16 opJSTART, opJPROGRAM, opCFG_IN;
    u16 ir_cap;
    int virtex_fx, virtex_lx;
    int rc = -1;
	    
	if (dev->family->vars.get(strJSTART, &opJSTART) ||
		dev->family->vars.get(strJPROGRAM, &opJPROGRAM) ||
		dev->family->vars.get(strCFG_IN, &opCFG_IN))
		return -1;

	virtex_fx = (dev->id & FAMILY_MASK) == FAMILY_XC4VFX_ID;
	virtex_lx = (dev->id & FAMILY_MASK) == FAMILY_XC4VLX_ID;

    prg->reset_tap_state();

    prg->shift_ir(&opJPROGRAM);
	
	prg->shift_ir(&opCFG_IN);
	
	prg->tck_cycle(100000);

	// BYPASS, Check INIT complete
    prg->shift_ir(ALL_ONES, &ir_cap);
    if ((ir_cap & 0x10) == 0)
	{
		msgf(STR_INIT_BIT_ERROR);
        goto cleanup;
	}

    prg->reset_tap_state();

    prg->shift_ir(&opCFG_IN);
    cfg_in_data[0] = FLUSH; // Flush
	
	if (virtex_fx)
		prg->enddr_state = TAPSTATE_PAUSEDR;
    
	prg->shift_dr_start(32, &cfg_in_data, NULL, 32);
    prg->shift_dr_cont(file->get_bit_length(), file->get_stream(), NULL, 1);

	if (virtex_fx)
	{
		prg->reset_tap_state();
		prg->goto_tap_state(TAPSTATE_IDLE);
	}

    prg->shift_ir(&opJSTART);

    prg->tck_cycle(12);

	if (virtex_fx)
		prg->enddr_state = TAPSTATE_IDLE;

	if (virtex_lx)
	{
		prg->shift_ir(&opCFG_IN);
		cfg_in_data[0] = DUMMY;
		cfg_in_data[1] = SYNC;
		cfg_in_data[2] = 0x00000004;
		cfg_in_data[3] = WRITE_CMD;
		cfg_in_data[4] = 0x20000000;
		cfg_in_data[5] = FLUSH;
		cfg_in_data[6] = FLUSH;
		prg->shift_dr(224, &cfg_in_data, NULL, 32);
	}
    
    //BYPASS
    prg->shift_ir(ALL_ONES, &ir_cap);
    if ((ir_cap & 0x20) == 0)
    {
        // attribute INSTRUCTION_CAPTURE : entity is "XXXXXXXX01";
        //-- Bit 5 is 1 when DONE is released (part of startup sequence)
        //-- Bit 4 is 1 if house-cleaning is complete
        //-- Bit 3 is ISC_Enabled
        //-- Bit 2 is ISC_Done
        msgf(STR_DONE_BIT_ERROR);
        goto cleanup;
    }
    
    rc = 0;

cleanup:

	prg->reset_tap_state();
	
    return rc;
}

int xc_user(chip *dev, cable *cbl, int user, uint8_t *in, uint8_t *out, int len) {
	u8 opUSER; //TODO: virtex: 16bit
	char buf[16];
	
	sprintf(buf,"%s%d",strUSER,user);
	
	if(dev->family->vars.get(buf, &opUSER)) {
		printf("USER%d unknown for this device\n",user);
		return -1;
	}
	
	cbl->reset_tap_state();
	
	// bypass
	cbl->shift_ir(ALL_ONES, NULL);
	
	cbl->shift_ir(&opUSER);
	
	cbl->shift_dr(len,in,out);
	
	// bypass
	cbl->shift_ir(ALL_ONES);

	cbl->reset_tap_state();
	
	return 0;
}

int spi_xfer_user1(chip *dev, cable *cbl, uint8_t *in, uint8_t *out, int len, int oskip) {
	uint8_t *ibuf=(uint8_t*)malloc(len+4+2+1);
	uint8_t *obuf=(uint8_t*)malloc(len+4+2+1);
	int cnt,rc;
	
	assert(ibuf);
	assert(obuf);
	
	ibuf[0]=0x59;
	ibuf[1]=0xa6;
	ibuf[2]=0x59;
	ibuf[3]=0xa6;
	
	ibuf[4]=(len*8)>>8;
	ibuf[5]=(len*8)&0xff;
	
	for(cnt=0;cnt<6;cnt++)
		ibuf[cnt]=reverse8(ibuf[cnt]);
	
	for(cnt=0;cnt<len;cnt++)
		ibuf[cnt+4+2]=reverse8(in[cnt]);
	
	rc=xc_user(dev,cbl,1,ibuf,obuf,(len+4+2+1)*8);
	
	for(cnt=0;cnt<len-oskip;cnt++) {
		out[cnt]=reverse8(obuf[cnt+4+2+1+oskip]);
	}
	
	free(ibuf);
	free(obuf);
	
	return rc;
}

int spi_cfg[] = {
	// sreg[5..2], pagesize, pages
	3, 264, 512, // XC3S50AN
	7, 264, 2048, // XC3S200AN / XC3S400AN
	9, 264, 4096, // XC3S700AN
	11, 528, 4096, // XC3S1400AN
	-1, 0, 0
};

int spi_flashinfo(chip *dev, cable *prg, int *size, int *pages) {
	uint8_t buf[8];
	int idx;
	
	buf[0]=0xd7;
	spi_xfer_user1(dev,prg,buf,buf+4,2,1);
	printf("status: %02x\n",buf[4]);
	
	for(idx=0;spi_cfg[idx] != -1;idx+=3) {
		if(spi_cfg[idx] == ((buf[4]>>2)&0x0f))
			break;
	}
	
	if(spi_cfg[idx] == -1) {
		printf("don't know that flash or status b0rken!\n");
		return -1;
	}
	
	printf("%d bytes/page, %d pages = %d bytes total \n",spi_cfg[idx+1],spi_cfg[idx+2],spi_cfg[idx+1]*spi_cfg[idx+2]);
	
	*size=spi_cfg[idx+1];
	*pages=spi_cfg[idx+2];
	
	return 0;
}

int spi_readback(chip *dev, cable *prg, u8 **data) {
	uint8_t *buf;
	int pgsize,pages,page,rc=0;
	
	rc=spi_flashinfo(dev,prg,&pgsize,&pages);
	if(rc)
		goto cleanup;
	
	*data=(u8*)malloc(pgsize*pages);
	
	buf=(uint8_t*)malloc(pgsize+16);
	buf[0]=0x03;
	buf[3]=0;
	
	for(page=0;page<pages;page++) {
		uint16_t paddr=page<<1;
		int res;
		
		if(!(page&0x0f)) {
			printf("\rpage %d",page);
			fflush(stdout);
		}
		
		// see UG333 page 19
		if(pgsize>512)
			paddr<<=1;
		
		buf[1]=paddr>>8;
		buf[2]=paddr&0xff;
		
		res=spi_xfer_user1(dev,prg,buf,(*data)+(page*pgsize),pgsize+4,4);
		//TODO: check res
		
		rc+=pgsize;
	}
	
cleanup:
	free(buf);

	if (rc < 0)
		if (*data)
		{
			free(*data);
			*data = NULL;
		}
	
	printf("\r");
		
	return rc;
}

///////////////////////////////////////////////////////////

int register_xilinx_functions()
{
    // Erase functions
    g.chips.register_erase_function("xcf_erase", xcf_erase);

    // Program functions
    g.chips.register_program_function("xcf_program", xcf_program);
	g.chips.register_program_function("xcv_program", xcv_program);
    g.chips.register_program_function("xc2v_program", xc2v_program);
    g.chips.register_program_function("xc4v_program", xc4v_program);
    
    // Readback functions
    g.chips.register_readback_function("xcf_readback", xcf_readback);
    g.chips.register_readback_function("spi_readback", spi_readback);

    return 0;
}
