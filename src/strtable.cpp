/*
xixilprg is covered by the LGPL:

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
#include "strtable.h"

const char* res_str_table[STR_MAX] = 
{
    //STR_LOGO
    "Xilinx Programmer " VERSION_STR " (" __DATE__ " " __TIME__ ")\n" 
    "By Zoltan Csizmadia (zoltan_csizmadia@yahoo.com)",
    //STR_DONE_BIT_ERROR
    "DONE bit didn\'t go high!",
    //STR_INIT_BIT_ERROR
    "INIT bit didn\'t go high!",
    //STR_INVALID_PARAMETERS
    "Invalid parameter(s)!",
    //STR_INVALID_CABLE_DEF
    "Invalid cable definition. Use cable command!",
    //STR_SELECTED_CABLE
    "Selected cable is \'%s\'",
    //STR_OPERATION_NOT_SUPPORTED
    "Selected operation is not supported on this device!",
    //STR_IDENTIFY_CHAIN
    "Identifying chain contents ...",
    //STR_UNKNOWN_DEVICES
    "Unknown device(s) were found, operation failed!",
    //STR_OPENING
    "Opening \'%s\' ...",
    //STR_ERASING
    "\'%d\' Erasing %s ...",
    //STR_PROGRAMMING
    "\'%d\' Programming %s ...",
    //STR_READING
    "\'%d\' Reading %s ...",
	//STR_VERIFYING
    "\'%d\' Verifying %s ...",
	//STR_SUCCESS_COMPLETED
	"Successfully completed.",
    //STR_DONE
    "Done.",
	//STR_FAILED
    "Failed!",
    //STR_INVALID_CONFIG_FILE
    "Invalid configuration file (line %d)!",
    //STR_DEVICES_FOUND
    "%d device(s) found.",
    //STR_CHDIR_ERROR
    "Failed to change directory!",
    //STR_UNKNOWN_COMMAND
    "Unknown command!",
    //STR_ENABLE_USERMODE_IO_ERROR
#ifdef WIN32
    "Failed to enable user mode I/O access! Giveio.sys must be installed!"
    "http://www.bottledlight.com/tools/giveio.zip",
#else
    "Failed to enable user-mode I/O access!",
#endif
    //STR_DEVICE_WRITE_PROTECTED
    "Device is write protected!",
	//STR_DEVICE_READ_PROTECTED
    "Device is read protected!",
    //STR_FAILED_TO_OPEN_CONFIG_FILE
    "Failed to open config file!",
    //STR_UNABLE_TO_OPEN_DIGILENT_USB
    "Unable to open Digilent USB device!\n"
#ifdef WIN32
    "Please check if Digilent USB driver is installed.\n"
	"http://www.digilentinc.com/Data/Products/JTAG-USB/DASv1-6.msi.",
#else
	"Please check if libusb package is installed correctly.",
#endif
    //STR_UNKNOWN_DEVICE_IDCODE
    "\'%d\' UNKNOWN device (IDCODE: 0x%08X)",
    //STR_DEVICE_IDCODE
    "\'%d\' IDCODE: 0x%08X",
    //STR_INVALID_DEVICE
    "Device doesn't exists!",
    //STR_SUPPORTED_FAMILIES
    "Supported families [Detect/Erase/Program/Read]",
	//STR_UNABLE_TO_OPEN_FILE
	"Unable to open file!",
	//STR_INVALID_FILE_FORMAT
	"Invalid file format!",
    //STR_HELP
    "Help",

    //STR_CMD_EXIT
    "exit\0"
    "Exits program\0"
    "\0",
    //STR_CMD_DETECT
    "detect\0" 
    "Detects JTAG device chain\0"
    "detect\0",
    //STR_CMD_CABLE
    "cable\0" 
    "Sets programmer cable\0"
    "cable {xil3 [ioaddr]|dusb|znuhtag|amontec}\0",
    //STR_CMD_CHIPS
    "chips\0"
    "Prints supported devices\0" 
    "chips [all]\0",
    //STR_CMD_PROGRAM
    "program\0" 
    "Programs device\0" 
    "program [-e] [-bit|-mcs|-bin] position file\n"
	"    -e         Erase device before programming\n"
	"E.g.: program 1 download.bit\0",
    //STR_CMD_ERASE
    "erase\0" 
    "Erases device\0" 
    "erase position\nE.g.: erase 1\0",
    //STR_CMD_READ
    "read\0" 
    "Read device content and saves in a file\0" 
    "\0",
	//STR_CMD_VERIFY
    "verify\0" 
    "Verifies device\0" 
    "verify [-bit|-mcs|-bin] position file\n"
	"E.g.: verify 1 image.mcs\0",
    //STR_CMD_IDCODE
    "idcode\0" 
    "Shows device IDCODE\0" 
    "\0",
    //STR_CMD_CD
    "cd\0"
    "Shows/changes current directory\0"
    "cd [dir]\0",
    //STR_CMD_VERSION
    "version\0"
    "Shows version information\0"
    "version",
    //STR_CMD_HELP
    "help\0" 
    "Shows help\0" 
    "help [cmd]\0",
    //STR_UNABLE_TO_OPEN_AMONTEC_USB
    "Unable to open Amontec JTAG cable\0",
    //STR_CMD_USER
	"user\0" "user\0" "user\0",
    //STR_CMD_SERVER
	"server\0" "server\0" "server\0",
    //STR_CMD_WRITE
	"write\0" "write\0" "write\0",

};

const char* res_str(unsigned int res_id)
{
    return res_id < STR_MAX ? res_str_table[res_id] : "!@#$%^&*?";
}

int msgf(unsigned int id, ...)
{
    va_list args;
    int rc;
    
    va_start(args, id);
    rc = vprintf(res_str(id), args);
    rc += printf("\n");
    va_end(args);

    return rc;
}
