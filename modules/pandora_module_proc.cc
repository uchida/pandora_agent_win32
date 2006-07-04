/* Pandora proc module. These modules check if a program is alive in the system.

   Copyright (C) 2006 Artica ST.
   Written by Esteban Sanchez.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "pandora_module_proc.h"
#include "../windows/pandora_wmi.h"
#include "../pandora_strutils.h"
#include <algorithm>
#include <cctype>

using namespace Pandora;
using namespace Pandora_Modules;
using namespace Pandora_Strutils;

Pandora_Module_Proc::Pandora_Module_Proc (string name, string process_name)
	: Pandora_Module (name) {
                                 
        transform (process_name.begin (), process_name.end (),
                   this->process_name.begin (), (int (*) (int)) tolower);
	
        this->module_kind_str = module_proc_str;
        this->module_kind     = MODULE_PROC;
}

void
Pandora_Module_Proc::run () {
        pandoraDebug ("MODULE_PROC RUN");

	
	int res = Pandora_Wmi::isProcessRunning (this->process_name);
	pandoraDebug ("res: %d", res);
        output = "1";
}
