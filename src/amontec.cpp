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

#include "xilprg.h"
#include "amontec.h"
#include "utils.h"
#include <time.h>
#include <assert.h>

amontec::amontec(){
	ftdic = NULL;
}

amontec::~amontec(){
}

int amontec::get_description(string& desc) {
    desc = "Amontec JTAG";
    return 0;
}

void amontec::bindump(unsigned char *d, int len) {
	int cnt;
	
	for(cnt=0; cnt<len; cnt++) {
		printf("%d",(d[cnt>>3]&(1<<(cnt&7))) ? 1 : 0);
	}
	printf("\n");
}

void amontec::hexdump(unsigned char *d, int len) {
	while(len--) {
		printf("%02x ",*d);
		d++;
	}
	printf("\n");
}

//#define DEBUG

int amontec::_ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size) {
	int rc = ftdi_read_data(ftdi, buf, size);
	
	while(rc <= 0)
		rc = ftdi_read_data(ftdi, buf, size);
#ifdef DEBUG
	printf("read(%d) ",rc);
	hexdump(buf,MIN(rc,10));
#endif
	return rc;
}

int amontec::_ftdi_write_data(struct ftdi_context *ftdi, unsigned char *buf, int size) {
	int rc = ftdi_write_data(ftdi, buf, size);;
#ifdef DEBUG
	printf("write(%d) ",rc);
	hexdump(buf,MIN(size,10));
#endif
	return rc;	
}

int amontec::open() {
	unsigned char buf[64];
    int rc = -1;
		
	assert( (ftdic = ftdi_new()) != NULL );

	assert(!(ftdi_init(ftdic)));
	
	rc = ftdi_usb_open(ftdic, USB_VENDOR_ID, USB_PRODUCT_ID);
	if(rc)
		goto cleanup;
	
	assert(!(ftdi_set_interface(ftdic, INTERFACE_A)));
	
	rc = ftdi_set_bitmode(ftdic, 0x1b, BITMODE_MPSSE); //0B
	if(rc)
		goto cleanup;
	
	buf[0]=0x80;
	buf[1]=0;
	buf[2]=0x1b;
	
	// flush
	buf[3]=0x87;
	
	// set speed to 1.5MHz
	buf[4]=0x86;
	buf[5]=3;
	buf[6]=0;
	
	ftdi_write_data(ftdic,buf,4+3);	
	ftdi_read_data(ftdic,buf,64);
	
	/*
	buf[0]=0x39;
	buf[1]=(rc-1)&0xff;
	buf[2]=(rc-1)/256;
	buf[3+rc]=0x87;
	
	printf("w: %d\n",ftdi_write_data(ftdic,buf,3+rc+1));
	printf("r: %d\n",(rc=ftdi_read_data(ftdic,buf,6400)));
	*/
	
	reset_tap_state();

	rc = 0;

cleanup:

    if (rc)	{
        close();
        msgf(STR_UNABLE_TO_OPEN_AMONTEC_USB);
	}

    return rc;
}

int amontec::close() {
		
	if(ftdic) {
	
		if(ftdic->usb_dev)
			ftdi_usb_close(ftdic);
		
		ftdi_free(ftdic);
		
		ftdic = NULL;
	}
	    
	return 0;
}

#define CHUNKSIZE 128

void amontec::shift_tdi_tdo(int n, unsigned char *ptdi, unsigned char *ptdo) {
	unsigned char buf[CHUNKSIZE*2];
	
	buf[0] = ( MPSSE_LSB | MPSSE_WRITE_NEG | MPSSE_DO_WRITE | (ptdo ? MPSSE_DO_READ : 0) );
	//buf[0] = ( MPSSE_LSB | MPSSE_WRITE_NEG | (ptdi ? MPSSE_DO_WRITE : 0) | (ptdo ? MPSSE_DO_READ : 0) );
	
	if(ptdi == ALL_ZEROS)
		bzero(buf+3,(CHUNKSIZE*2)-3);
	
	else if(ptdi == ALL_ONES)
		memset(buf+3,0xff,(CHUNKSIZE*2)-3);
		
	// byte mode
	while(n>=8) {
		int write_chunk=0, chunk_bytes, chunk_bits = MIN(n,(((CHUNKSIZE*2)-4)*8));
		
		chunk_bits &= ~7;
		chunk_bytes = chunk_bits / 8;
		
		buf[1] = (chunk_bytes-1) & 0xff;
		buf[2] = (chunk_bytes-1) / 256;
		
//		if(ptdi) {
			write_chunk = chunk_bytes;
	
			if((ptdi != ALL_ZEROS) && (ptdi != ALL_ONES)) {
				memcpy(buf+3, ptdi, write_chunk);
				ptdi += write_chunk;
			}
//		}
		
		if(ptdo)
			buf[3+write_chunk] = SEND_IMMEDIATE;
		
		assert( _ftdi_write_data(ftdic, buf, write_chunk + (ptdo ? 4 : 3)) == (write_chunk + (ptdo ? 4 : 3)) );
		
		if(ptdo) {
			//usleep(10000);
			assert( _ftdi_read_data(ftdic, ptdo, chunk_bytes) == chunk_bytes );
			ptdo += chunk_bytes;
		}
		
		n -= chunk_bits;
	}
	
	//if((ptdi) && (ptdi != ALL_ONES))
		//printf("%d remaining (%x)\n",n,*ptdi);
		   
	// remaining bits
	if(n) {
		buf[0] |= MPSSE_BITMODE;
		buf[1] = n - 1;
		
//		if(ptdi) {
			
			if(ptdi == ALL_ZEROS)
				buf[2] = 0;
			else if (ptdi == ALL_ONES)
				buf[2] = 0xff;
			else
				buf[2] = *ptdi;
			
			if(ptdo)
				buf[3] = SEND_IMMEDIATE;
//		}
	//	else if (ptdo)
		//	buf[2] = SEND_IMMEDIATE;
		
		assert( _ftdi_write_data(ftdic, buf, 3 + (ptdo ? 1 : 0)) == (3 + (ptdo ? 1 : 0)) );
		
		if(ptdo)
			assert( _ftdi_read_data(ftdic, ptdo, 1) == 1);
	}
	
}

void amontec::shift_tdi_tms_tdo(int tdi, u32 tms, int len, unsigned char *tdo) {
	unsigned char buf[64], cmd;
	int rlen=len, idx=0;
	
	tdi = (tdi & 1) << 7;
	
	cmd = ( MPSSE_LSB | MPSSE_WRITE_NEG | MPSSE_BITMODE | MPSSE_WRITE_TMS );

	if(tdo)
		cmd |= MPSSE_DO_READ;
	
	while(rlen>0) {
		int chunk = MIN(rlen,7);
		
		buf[idx++] = cmd;
		buf[idx++] = chunk - 1;
		buf[idx++] = tdi | (tms&0x7f);
		
		rlen -= chunk;
		tms >>= chunk;
	}
	
	if(tdo)
		buf[idx++] = SEND_IMMEDIATE;
	
	assert( _ftdi_write_data(ftdic, buf, idx) == idx);
	
	idx = (idx-1) / 3;
	
	if(tdo) {
		assert( _ftdi_read_data(ftdic, buf, idx) == idx);
		/* BUGBUGBUG? */
		//printf("len %d val %x\n",len,*buf);
		*tdo=*buf;
	}
}

void amontec::shift(int num_bits, void* ptdi, void* ptdo, int last) {
	//printf("shift %d %p %p %d\n",num_bits,ptdi,ptdo,last);
	
	//last=0;
	
	//if((ptdo) && (num_bits == 6))
	//	num_bits--;
	
	if(ptdo)
		bzero((unsigned char *)ptdo, (num_bits+7)/8);
	
	//if(num_bits > 1000)
		//hexdump((unsigned char *)ptdi,100);
	
	shift_tdi_tdo(last ? num_bits-1 : num_bits, (unsigned char *) ptdi, (unsigned char *) ptdo);
	
	// last bit + TMS
	if(last) {
		unsigned char *tdip = (unsigned char *) ptdi;
		int idx = (num_bits - 1) / 8;
		int shift = (num_bits - 1) % 8;
		unsigned char tdi;
		unsigned char tdo;
		
		if((ptdi != ALL_ONES) && (ptdi != ALL_ZEROS))
			 tdi = tdip[idx] >> shift;
		
		shift_tdi_tms_tdo(tdi, 1, 1, (unsigned char *) &tdo);
		
		//printf("%d\n",shift);
		
		/* BUGBUGBUGBUG? */
		if(ptdo) {
			unsigned char *tdop = (unsigned char *) ptdo;
			
			tdop[idx] >>= 1;
			tdop[idx] |= (tdo&0x80);
			//printf("tdo %x\n",tdo);
			if(num_bits<8)
				tdop[idx] >>= (8 - num_bits); // I have NO idea! TODO: check with loopback
		}
	}
	//if(ptdo)
		//hexdump((unsigned char *)ptdo,(num_bits+7)/8);
}

void amontec::tck_cycle(int n) {
	//printf("tck_cycle %d\n",n);
	shift_tdi_tdo(n, ALL_ZEROS, NULL);
}

void amontec::tms_transition(u32 seq, int cnt) {
	//printf("tms %x %d\n",seq,cnt);
	shift_tdi_tms_tdo(0, seq, cnt, NULL);
}
