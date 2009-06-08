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
#include "znuhtag.h"
#include "utils.h"
#include <time.h>
#include <assert.h>

#define MAX_BLOCKLEN		((64-2)*8)

znuhtag::znuhtag()
{
    handle = NULL;

}

znuhtag::~znuhtag()
{
}

struct usb_device* znuhtag::find_device()
{
	struct usb_bus *bus;
	struct usb_device *dev;

	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == USB_VENDOR_ID &&
			    dev->descriptor.idProduct == USB_PRODUCT_ID)
				// Found it
				return dev;
		}
	}

	return NULL;
}

int znuhtag::get_description(string& desc)
{
    desc = "Znuh's JTAG via USB";
    return 0;
}

int znuhtag::open()
{
    int rc = -1;
	struct usb_device* dev;
	unsigned char buf[64];

	if (handle)
		// Already open
        return -1;

	dev = find_device();
	if (dev == NULL)
		goto cleanup;
        
	handle = usb_open(dev);
    if (handle == NULL)
        goto cleanup;

    usb_detach_kernel_driver_np(handle, 0);
    
    if (usb_set_configuration(handle, 1) < 0)
        goto cleanup;

    if (usb_claim_interface(handle, 0) < 0)
        goto cleanup;
    
	reset_tap_state();

	/* GRMBL!! */
	do {
		buf[0]=CMD_JTAG_TDO;
		buf[1]=0;
		
		assert(usb_bulk_write(handle, 0x01, (char *) buf, 64, 1000) == 64);
		rc = usb_bulk_read(handle, 0x81, (char *)buf, 64, 100);
	} while(rc != 64);
    
	rc = 0;

cleanup:

    if (rc)
	{
        close();
        msgf(STR_UNABLE_TO_OPEN_DIGILENT_USB);
	}

    return rc;
}

int znuhtag::close()
{

	if (handle == NULL)
        return -1;

	usb_release_interface(handle, 0);
	usb_close(handle);

	handle = NULL;

    return 0;
}

void znuhtag::bindump(unsigned char *d, int len) {
	int cnt;
	
	for(cnt=0; cnt<len; cnt++) {
		printf("%d",(d[cnt>>3]&(1<<(cnt&7))) ? 1 : 0);
	}
	printf("\n");
}

void znuhtag::hexdump(unsigned char *d, int len) {
	while(len--) {
		printf("%02x ",*d);
		d++;
	}
	printf("\n");
}

void znuhtag::usb_transfer(unsigned char *snd, unsigned char *rcv, int bitlen) {
	unsigned char rcvbuf[64];
	int rc=0;
			
	while(rc!=64) {
		rc = usb_bulk_write(handle, 0x01, (char *) snd, 64, 1000);
		
		assert(rc==64);
		
		if(snd[0]&CMD_JTAG_TDO) {
			rc = usb_bulk_read(handle, 0x81, (char *) rcvbuf, 64, 1000);
		
			if(rc != 64)
				printf("%d\n",rc);
		}
		
	}
	
	if(rcv)
		memcpy(rcv, rcvbuf+2, (bitlen%8) ? ((bitlen/8) + 1) : (bitlen/8));
}

void znuhtag::usb_command(int tms, void *snd, void *rcv, int bitlen, int last) {
	unsigned char buf[64];
	u8* _snd = (u8*) snd;
	u8* _rcv = (u8*) rcv;
	time_t start, end;
	int initial_bitlen = bitlen;
	int last_perc=-1;
	
	assert(bitlen>0);
	//printf("tms: %d snd: %p rcv: %p len: %d last: %d\n",tms,snd,rcv,bitlen,last);
	
	if(snd==ALL_ONES)
		memset(buf+2,0xff,64-2);
	else if(snd==ALL_ZEROS)
		bzero(buf+2,64-2);
	
	start=time(NULL);
	
	while(bitlen>0) {
		u16 tlen = (bitlen > MAX_BLOCKLEN) ? MAX_BLOCKLEN : bitlen;
		u8 bytes = (tlen%8) ? ((tlen/8)+1) : (tlen/8);
		int perc;
		
		if((snd != ALL_ONES) && (snd != ALL_ZEROS))
			memcpy(buf+2, _snd, bytes);
		
		buf[0] = (tlen>>8);
		buf[1] = tlen&0xff;
		
		buf[0] |= CMD_JTAG;
		buf[0] |= tms ? CMD_JTAG_TMS : 0;
		buf[0] |= rcv ? CMD_JTAG_TDO : 0;
		
		// last block
		if((last) && (tlen == bitlen))
			buf[0] |= CMD_JTAG_LAST;
		
		//printf("transfer %d\n",tlen);
		
		usb_transfer(buf,_rcv,tlen);
		
		perc=((initial_bitlen-bitlen)*100)/initial_bitlen;
		
		if(((perc/5)!=(last_perc/5)) && (initial_bitlen>(MAX_BLOCKLEN*30))) {
			printf("\r%02d%%",perc);
			fflush(stdout);
		}
		
		last_perc=perc;
		
		bitlen-=tlen;
		
		if((snd != ALL_ONES) && (snd != ALL_ZEROS))
			_snd += bytes;
		
		if(_rcv)
			_rcv += bytes;
	}
	end = time(NULL);
	
	if(initial_bitlen>(MAX_BLOCKLEN*30))
		printf("\rbitrate: %ld bps (%ld seconds)\n",initial_bitlen/(end-start),end-start);
	
}

void znuhtag::shift(int num_bits, void* ptdi, void* ptdo, int last)
{
	//printf("tdi %p tdo %p ones %p num %d last %d\n",ptdi,ptdo,ALL_ONES,num_bits,last);
	usb_command(0,ptdi,ptdo,num_bits,last);
	//if(ptdo)
		//hexdump((unsigned char *)ptdo,4);
}

void znuhtag::tck_cycle(int n)
{
   //printf("tck %d\n",n);
   usb_command(0,ALL_ZEROS,NULL,n,0);
}

void znuhtag::tms_transition(u32 seq, int cnt)
{
	//printf("tms %d\n",cnt);
	usb_command(1,(u8*)&seq,NULL,cnt,0);
}
