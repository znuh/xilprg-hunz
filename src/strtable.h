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

#ifndef _STRTABLE_H_INCLUDED_
#define _STRTABLE_H_INCLUDED_

enum
{
    STR_LOGO,
    STR_DONE_BIT_ERROR,
    STR_INIT_BIT_ERROR,
    STR_INVALID_PARAMETERS,
    STR_INVALID_CABLE_DEF,
    STR_SELECTED_CABLE,
    STR_OPERATION_NOT_SUPPORTED,
    STR_IDENTIFY_CHAIN,
    STR_UNKNOWN_DEVICES,
    STR_OPENING,
    STR_ERASING,
    STR_PROGRAMMING,
    STR_READING,
	STR_VERIFYING,
	STR_SUCCESS_COMPLETED,
    STR_DONE,
	STR_FAILED,
    STR_INVALID_CONFIG_FILE,
    STR_DEVICES_FOUND,
    STR_CHDIR_ERROR,
    STR_UNKNOWN_COMMAND,
    STR_ENABLE_USERMODE_IO_ERROR,
    STR_DEVICE_WRITE_PROTECTED,
	STR_DEVICE_READ_PROTECTED,
    STR_FAILED_TO_OPEN_CONFIG_FILE,
    STR_UNABLE_TO_OPEN_DIGILENT_USB,
    STR_UNKNOWN_DEVICE_IDCODE,
    STR_DEVICE_IDCODE,
    STR_INVALID_DEVICE,
    STR_SUPPORTED_FAMILIES,
	STR_UNABLE_TO_OPEN_FILE,
	STR_INVALID_FILE_FORMAT,
    STR_HELP,
    STR_CMD_EXIT,
    STR_CMD_DETECT,
    STR_CMD_CABLE,
    STR_CMD_CHIPS,
    STR_CMD_PROGRAM,
    STR_CMD_ERASE,
    STR_CMD_READ,
	STR_CMD_VERIFY,
    STR_CMD_IDCODE,
    STR_CMD_CD,
    STR_CMD_VERSION,
    STR_CMD_HELP,
	STR_UNABLE_TO_OPEN_AMONTEC_USB,
	STR_CMD_USER,
	STR_CMD_SERVER,

    STR_MAX
};

const char* res_str(unsigned int);
int msgf(unsigned int, ...);

#endif
