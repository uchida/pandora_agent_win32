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

#ifndef	__PANDORA_MODULE_PROC_H__
#define	__PANDORA_MODULE_PROC_H__

#include "pandora_module.h"

#define	MIN_DELAY 2000 /* Minimum process start/retry delay */

namespace Pandora_Modules {
	/**
	 * Module to check that a process is running on the system.
	 */
	class Pandora_Module_Proc : public Pandora_Module {
	private:
		string process_name;
		HANDLE thread;
		bool   watchdog;
		string start_command;
		int retries;
		int start_delay;
		int stop_delay;
	public:
		Pandora_Module_Proc    (string name, string process_name);
		
		string getProcessName  () const;
		string getStartCommand () const;
		bool   isWatchdog      () const;
		int    getRetries      () const;
		int    getStartDelay   () const;
		void   getRetryDelay   () const;
		
		void   setWatchdog     (bool watchdog);
		void   setStartCommand (string command);
		void   setRetries      (int retries);
		void   setStartDelay   (int mseconds);
		void   setRetryDelay   (int mseconds);
		
		void   run             ();
	};
}

#endif
