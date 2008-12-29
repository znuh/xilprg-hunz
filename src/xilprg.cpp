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
#include "utils.h"
#include "cmds.h"
#include "parport.h"
#include "digilent.h"
#include "chip.h"
#include "cmdline.h"

globals g;

const char* strNAME = "NAME";
const char* strDESC = "DESC";
const char* strTYPE = "TYPE";
const char* strFPGA = "FPGA";
const char* strCPLD = "CPLD";
const char* strPROM = "PROM";
const char* strIRLEN = "IRLEN";
const char* strERASE = "ERASE";
const char* strPROGRAM = "PROGRAM";
const char* strREADBACK = "READBACK";
const char* strCABLE = "CABLE";

int register_xilinx_functions();

int initialize()
{
    g.sel_dev = -1;

#ifndef WIN32
	// Initialize libusb library
	usb_init();
#endif

    register_xilinx_functions();

    return 0;
}

void uninitialize()
{
}

cable* open_cable(int detect)
{
    cable* cbl;
    string desc, cable_def;

	if (g.vars.get(strCABLE, cable_def))
	{
		msgf(STR_INVALID_CABLE_DEF);
		return NULL;
	}

    cbl = cable::factory(cable_def.c_str());
    if (cbl == NULL)
        return NULL;

    cbl->get_description(desc);
    msgf(STR_OPENING, desc.c_str());

    if (cbl->open())
    {
        delete cbl;
        return NULL;
    }

    if (detect)
        cbl->detect_chain();

    return cbl;
}

int close_cable(cable* cbl)
{
    if (cbl == NULL)
        return -1;
    cbl->close();
    delete cbl;
    return 0;
}

chip* select_device_in_chain(int index)
{
    if (index >= (int)g.dev_chain.size())
    {
        msgf(STR_INVALID_DEVICE);
        return NULL;
    }

    g.sel_dev = index;
	return  g.dev_chain[index].dev;
}

int load_config_file()
{
	int rc = -1, line_index = 0;
    FILE* f;
	char line[256];
    chip_family* family = NULL;
    char* name;
    char* param;
    int argc;
    char** argv = NULL;
    int id, mask;
    int ir_length;

	f = fopen("xilprg.conf", "rt");
	if (f == NULL)
	{
        msgf(STR_FAILED_TO_OPEN_CONFIG_FILE);
		return -1;
	}

	for (line_index = 1; !feof(f); line_index++)
	{
		if (fgets(line, sizeof(line), f) == NULL)
			break;

        strip_whitespaces(line);

        // Empty
        if (*line == 0)
            continue;

        // Comment
        if (line[0] == ';' || line[0] == '#')
            continue;

        if (stricmp(line, "BEGIN_FAMILY") == 0)
        {
            if (family != NULL)
                goto cleanup;
            family = new chip_family;
            if (family == NULL)
                goto cleanup;
        }
        else
        if (stricmp(line, "END_FAMILY") == 0)
        {
            if (family == NULL)
                goto cleanup;

            // Check if NAME, DESC, TYPE exist
            if (family->vars.exists(strNAME) < 0)
                goto cleanup;
            if (family->vars.exists(strDESC) < 0)
                goto cleanup;
            if (family->vars.exists(strTYPE) < 0)
                goto cleanup;

            // Add family to chip database
            g.chips.push_back(family);
            family = NULL;
        }
        else
        {
            name = line;
            param = strchr(line, '=');
            *param = 0;
            param++;

            strip_whitespaces(name);
            strip_whitespaces(param);

            if (*name == 0)
                goto cleanup;

            parse_cmdline_args(param, &argc, &argv);
    
            if (family)
            {
                if (stricmp(name, "CHIP") == 0)
                {
                    if (argc != 4)
                        goto cleanup;

                    if (str2num(argv[1], &id) ||
                        str2num(argv[2], &mask) ||
                        str2num(argv[3], &ir_length))
                        goto cleanup;
                    
                    family->push_back(chip(argv[0], id, mask, ir_length, family));
                }
                else
                if (argc > 0)
                    family->vars.add(name, argv[0]);
            }
            else
				g.vars.add(name, param);

            free(argv);
            argv = NULL;
        }
	}
    
    rc = 0;
	
cleanup:

	if (f)
		fclose(f);

    if (family)
        delete family;

    if (argv)
        free(argv);

    if (rc)
        msgf(STR_INVALID_CONFIG_FILE, line_index);

	return rc;
}

int main()
{
    char line[1024];
    int rc = -1;

    process_command_line("version");
    printf("\n");

    if (initialize())
        goto cleanup;

	if (load_config_file())
		goto cleanup;

    do
    {
        if (prompt_read_line("xilprg> ", line, sizeof(line)) < 0)
			break;
    }
    while (process_command_line(line) != CMDLINE_EXIT_PROGRAM);

    rc = 0;

cleanup:

    uninitialize();
   
    return rc;
}
