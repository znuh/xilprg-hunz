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
#include "digilent.h"
#include "utils.h"

digilent::digilent()
: tms(0), tdo(0), pending_shift_r_read(0), queued_wr_packet_count(0)
{
#ifdef WIN32
    handle_in = INVALID_HANDLE_VALUE;
    handle_out = INVALID_HANDLE_VALUE;
#else
    handle = NULL;
#endif
}

digilent::~digilent()
{
}

#ifdef WIN32
int digilent::find_digilent_device_name(char* device)
{
    char* sz;
    char* p;
    int len = 0xFFFF;
    int rc = -1;
    u32 vendor, product;

	sz = (char*)malloc(len);
	if (sz == NULL)
		return -1;

    // Find all DOS names
	if (QueryDosDevice(NULL, sz, len) <= 0)
		return -1;
    
    p = sz;
    while (*p)
    {
        if (sscanf(p, "USB#Vid_%x&Pid_%x#", &vendor, &product) == 2)
        {
            if (vendor == USB_VENDOR_ID && product == USB_PRODUCT_ID)
                // Found
                break;
        }
        p += strlen(p) + 1;
    }

    if (*p == 0)
        // Not found
        goto cleanup;

    // Copy result to target buffer
    strncpy(device, p, MAX_PATH);

    rc = 0;

cleanup:

    if (sz)
        free(sz);

    return rc;
}
#else
struct usb_device* digilent::find_digilent_device()
{
	struct usb_bus* bus;
	struct usb_device* dev;
    int i;

	usb_find_busses();
	usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next) 
	{
		if (bus->root_dev)
		{
			for (i = 0; i < bus->root_dev->num_children; i++)
		        if (bus->root_dev->children[i]->descriptor.idVendor == USB_VENDOR_ID &&
                    bus->root_dev->children[i]->descriptor.idProduct == USB_PRODUCT_ID)
					// Found it
					return bus->root_dev->children[i];
		}
		else
		{
			for (dev = bus->devices; dev; dev = dev->next)
                if (dev->descriptor.idVendor == USB_VENDOR_ID &&
                    dev->descriptor.idProduct == USB_PRODUCT_ID)
					// Found it
					return dev;
		}
    }

	return NULL;
}
#endif

int digilent::get_description(string& desc)
{
    desc = "Digilent USB";
    return 0;
}

int digilent::open()
{
#ifdef WIN32
    int rc = -1;
    char device_name[MAX_PATH];
    char name[MAX_PATH];
    
    if (handle_in != INVALID_HANDLE_VALUE || 
        handle_out != INVALID_HANDLE_VALUE)
        // Already open
        return -1;

    // Find digilent USB device name
    if (find_digilent_device_name(device_name) < 0)
		goto cleanup;
	
    sprintf(name, "\\\\.\\%s\\PIPE00", device_name);
    handle_in = CreateFile(
                    name,
                    GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
    sprintf(name, "\\\\.\\%s\\PIPE01", device_name);
    handle_out = CreateFile(
                    name,
                    GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
    if (handle_in == INVALID_HANDLE_VALUE || 
        handle_out == INVALID_HANDLE_VALUE)
        goto cleanup;
#else
    int rc = -1;
	struct usb_device* dev;

	if (handle)
		// Already open
        return -1;

	dev = find_digilent_device();
	if (dev == NULL)
		goto cleanup;
        
	handle = usb_open(dev);
    if (handle == NULL)
        goto cleanup;

    if (usb_set_configuration(handle, 1) < 0)
        goto cleanup;
    if (usb_claim_interface(handle, 0) < 0)
        goto cleanup;

#endif

    if (enable_jtag(1))
		goto cleanup;

	reset_tap_state();

	rc = 0;

cleanup:

    if (rc)
	{
        close();
        msgf(STR_UNABLE_TO_OPEN_DIGILENT_USB);
	}

    return rc;
}

int digilent::close()
{
    enable_jtag(0);

#ifdef WIN32
    if (handle_in != INVALID_HANDLE_VALUE)
        CloseHandle(handle_in);

    if (handle_out != INVALID_HANDLE_VALUE)
        CloseHandle(handle_out);

    handle_in = INVALID_HANDLE_VALUE;
    handle_out = INVALID_HANDLE_VALUE;
#else
	if (handle == NULL)
        return -1;

	usb_close(handle);
	//usb_reset(handle);
	handle = NULL;
#endif

    return 0;
}

void digilent::reset_tap_state()
{
	cable::reset_tap_state();
	flush_wr_bulk();
}

int digilent::shift_r_start(int state, int num_bits, void* ptdi, void* ptdo, int align, int last)
{
    if ((ptdi != ALL_ONES && ptdi != ALL_ZEROS) && ptdo != NULL)
        return -1;
    
    pending_shift_r_read = ptdo != NULL;
    
    return cable::shift_r_start(state, num_bits, ptdi, ptdo, align, last);
}

void digilent::shift(int num_bits, void* ptdi, void* ptdo, int last)
{
	u8 last_tdi = 0, last_tdo;
    int last_byte_pos, last_bit_pos;
	u8* _ptdi = (u8*)ptdi; 
	u8* _ptdo = (u8*)ptdo; 
    
    if (num_bits == 0)
		return;

    if (!last)
    {
        if ((_ptdi == ALL_ONES || _ptdi == ALL_ZEROS) && ptdo != NULL)
            get_tdo_bits(num_bits, tms, ptdi == ALL_ZEROS ? 0 : 1, ptdo);
        else
            put_tdi_bits(num_bits, ptdi, tms, ptdo);
    }
    else
    {
        if ((_ptdi == ALL_ONES || _ptdi == ALL_ZEROS) && _ptdo != NULL)
            get_tdo_bits(num_bits - 1, tms, _ptdi == ALL_ZEROS ? 0 : 1, _ptdo);
        else
            put_tdi_bits(num_bits - 1, ptdi, tms, _ptdo);

        last_byte_pos = (num_bits - 1) / 8;
        last_bit_pos = (num_bits - 1) % 8;
    
        if (_ptdi == ALL_ZEROS || _ptdi == ALL_ONES)
            last_tdi = (u8)(_ptdi == ALL_ZEROS ? 0 : 1);
        else
		    last_tdi = (u8)((_ptdi[last_byte_pos] >> last_bit_pos) & 1);
    
        // TMS transition
        tms = 1;
        last_tdi |= 2;

        put_tms_tdi_bits(1, &last_tdi, &last_tdo);

        // Add last TDO bit
        if (ptdo)
        {
            if (last_tdo)
                _ptdo[last_byte_pos] |= 1 << last_bit_pos;
            else
                _ptdo[last_byte_pos] &= ~(1 << last_bit_pos);
        }
    }
}

void digilent::tck_cycle(int n)
{
    put_tdi_bits(n, ALL_ZEROS, tms, NULL);
}

void digilent::tms_transition(u32 seq, int cnt)
{
    u64 tms_tdi = 0;
    int s, i;

    if (cnt == 0)
        return;

    for (i = 0, s = 0; i < cnt; i++, s += 2)
    {
        tms = (u8)((seq & 1) ? 1 : 0);
        tms_tdi |= tms << (s + 1);
        seq >>= 1;
    }

    if (pending_shift_r_read && 
        (current_tap_state == TAPSTATE_SHIFTDR ||current_tap_state == TAPSTATE_SHIFTIR))
    {
        pending_shift_r_read = 0;
        cnt--;
    }
    
    put_tms_tdi_bits(cnt, (u8*)&tms_tdi, NULL);
}

//////////////////////////////////////////////////////////////////////////

void digilent::init_bulk_packet(bulk_packet* packet, int cmd, int cbit)
{
    packet->cmd = (u8)cmd;
    packet->cbit = HTON16(cbit);
}

//#define DUMP_WR

int digilent::write_bulk(bulk_packet* packet, int n)
{
#ifdef WIN32
    DWORD written;
	if (handle_out == INVALID_HANDLE_VALUE)
		return -1;
    if (!WriteFile(handle_out, packet, sizeof(bulk_packet)*n, &written, NULL))
        return -1;
    return written == sizeof(bulk_packet)*n ? 0 : -1;
#else
    int rc;
	if (handle == NULL)
		return -1;
    rc = usb_bulk_write(handle, 0x04, (char*)packet, sizeof(bulk_packet)*n, 1000); 
    if (rc < 0)
        return rc;
    return rc == (int)(sizeof(bulk_packet)*n) ? 0 : -1;
#endif
}

int digilent::read_bulk(bulk_packet* packet)
{
#ifdef WIN32
    DWORD written;
	if (handle_in == INVALID_HANDLE_VALUE)
		return -1;
    if (!ReadFile(handle_in, packet, sizeof(bulk_packet), &written, NULL))
        return -1;
    return written == sizeof(bulk_packet) ? 0 : -1;
#else
    int rc;
	if (handle == NULL)
		return -1;
    rc = usb_bulk_read(handle, 0x82, (char*)packet, sizeof(bulk_packet), 1000); 
    if (rc < 0)
        return rc;
    return rc == sizeof(bulk_packet) ? 0 : -1;
#endif
}

int digilent::enable_jtag(int enable)
{
    bulk_packet packet;
    init_bulk_packet(&packet, CMD_ENABLE, 1);
    packet.data[0] = (u8)(enable ? 0x01 : 0x00);
    return queue_wr_bulk(&packet);
}

int digilent::set_tms_tdi_tck(u8 tms, u8 tdi, u8 tck)
{
    bulk_packet packet;
    init_bulk_packet(&packet, CMD_SET, 1);
    packet.data[0] = (u8)((tck ? 4 : 0) + (tms ? 2 : 0) + (tdi ? 1 : 0));
    return queue_wr_bulk(&packet);
}

int digilent::put_tdi_bits(int cbit, void* snd, int tms, void* rcv)
{
    bulk_packet packet;
    int left = cbit;
    int cnt_bits, cnt_bytes;
	u8* _snd = (u8*)snd;
	u8* _rcv = (u8*)rcv;
    
    while (left > 0)
    {
        cnt_bits = left < BULK_PACKET_DATA_LEN*8 ? left : BULK_PACKET_DATA_LEN*8;
        cnt_bytes = (cnt_bits + 7) / 8;
        left -= cnt_bits;

        init_bulk_packet(
            &packet,
            CMD_PUT_TDI_BITS + 
            (rcv ? CMD_FLAG_RECEIVE : 0) +
            (tms ? CMD_FLAG_TMS_HIGH : 0),
            cnt_bits);

        if (_snd == ALL_ZEROS)
            memset(packet.data, 0, cnt_bytes);
        else
        if (_snd == ALL_ONES)
            memset(packet.data, 0xFF, cnt_bytes);
        else
        {
            memcpy(packet.data, _snd, cnt_bytes);
            _snd += cnt_bytes;
        }
        
        if (queue_wr_bulk(&packet, rcv ? 1 : 0))
            return -1;

        if (rcv)
        {
            init_bulk_packet(&packet, CMD_NULL, 0);
            if (read_bulk(&packet))
                return -1;
            if (packet.cmd != CMD_RECEIVE)
                return -1;

            memcpy(rcv, packet.data, cnt_bytes);
            _rcv += cnt_bytes;
        }
    }

    return 0;
}

int digilent::put_tms_tdi_bits(int cbit, void* snd, void* rcv)
{
    bulk_packet packet;
	int left = cbit;
    int cnt_bits, cnt_bytes, cnt_bytes2;
	u8* _snd = (u8*)snd;
	u8* _rcv = (u8*)rcv;
    
	while (left > 0)
    {
        cnt_bits = left < BULK_PACKET_DATA_LEN*4 ? left : BULK_PACKET_DATA_LEN*4;
		cnt_bytes = (cnt_bits + 3) / 4;
		cnt_bytes2 = (cnt_bits + 7) / 8;
        left -= cnt_bits;

        init_bulk_packet(
            &packet,
            CMD_PUT_TMS_TDI_BITS + (_rcv ? CMD_FLAG_RECEIVE : 0),
            cnt_bits);

        if (snd == ALL_ZEROS)
            memset(packet.data, 0, cnt_bytes);
        else
        if (snd == ALL_ONES)
            memset(packet.data, 0xFF, cnt_bytes);
        else
        {
            memcpy(packet.data, _snd, cnt_bytes);
            _snd += cnt_bytes;
        }
        
        if (queue_wr_bulk(&packet, rcv ? 1 : 0))
            return -1;

        if (_rcv)
        {
            init_bulk_packet(&packet, CMD_NULL, 0);
            if (read_bulk(&packet))
                return -1;
            if (packet.cmd != CMD_RECEIVE)
                return -1;

            memcpy(_rcv, packet.data, cnt_bytes2);
            _rcv += cnt_bytes2;
        }

        left -= cnt_bits;
    }

    return 0;
}

int digilent::get_tdo_bits(int cbit, int tms, int tdi, void* rcv)
{
    bulk_packet packet;
    int left = cbit;
    int cnt_bits, cnt_bytes;
	u8* _rcv = (u8*)rcv;

    while (left > 0)
    {
        cnt_bits = left < BULK_PACKET_DATA_LEN*8 ? left : BULK_PACKET_DATA_LEN*8;
        cnt_bytes = (cnt_bits + 7) / 8;

        init_bulk_packet(
            &packet,
            CMD_GET_TDO_BITS + 
            (tdi ? CMD_FLAG_TDI_HIGH : 0) +
            (tms ? CMD_FLAG_TMS_HIGH : 0),
            cnt_bits);

		if (queue_wr_bulk(&packet, 1))
            return -1;

        init_bulk_packet(&packet, CMD_NULL, 0);
        
        if (read_bulk(&packet))
            return -1;
        
        if (packet.cmd != CMD_RECEIVE)
            return -1;

        if (_rcv)
        {
            memcpy(_rcv, packet.data, cnt_bytes);
            _rcv += cnt_bytes;
        }

        left -= cnt_bits;
    }

    return 0;
}

int digilent::queue_wr_bulk(bulk_packet* packet, int flush)
{
    memcpy(queued_wr_packets + queued_wr_packet_count, packet, sizeof(bulk_packet));
	queued_wr_packet_count++;
	if (queued_wr_packet_count == MAX_BULK_PACKET_BURST)
		flush = 1;
	return flush ? flush_wr_bulk() : 0;
}

int digilent::flush_wr_bulk()
{
	int rc;
    if (queued_wr_packet_count == 0)
        return 0;
    rc = write_bulk(queued_wr_packets, queued_wr_packet_count);
	queued_wr_packet_count = 0;
	return rc;
}
