/* Class to manage the Windows Management Instrumentation(WMI).
   It depends on disphelper library (http://disphelper.sourceforge.net)

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
   Inc., 59 Temple Place - Suite 330, Boston, MAB02111-1307, USA.
*/

#include "pandora_wmi.h"
#include "../pandora_strutils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <ctime>
#include <winuser.h>

using namespace std;
using namespace Pandora_Wmi;

static LPWSTR
getWmiStr (LPCWSTR computer) {
	static WCHAR wmi_str[256];

	wcscpy (wmi_str, L"winmgmts:\\\\");
	
	if (computer) {
		wcsncat (wmi_str, computer, 128);
	} else {
		wcscat (wmi_str, L".");
	}

	wcscat (wmi_str, L"\\root\\cimv2");
	
	return wmi_str;
}

/** 
 * Check if a process is running.
 * 
 * @param process_name Name of the process with extension.
 * 
 * @return Number of instances of the process running.
 */
int
Pandora_Wmi::isProcessRunning (string process_name) {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	string        name;
	int           result = 0;
	string        query;

	query = "SELECT * FROM Win32_Process WHERE Name=\"" + process_name + "\"";
	
	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%T)",
				     query.c_str ()));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			result++;
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("isProcessRunning error. %s", errstr.c_str ());
	}

	return result;
}

/** 
 * Check if a Windows service is running.
 * 
 * @param service_name Internal name of the service to check.
 * 
 * @retval 1 The service is running
 * @retval 0 The service is stopped
 */
int
Pandora_Wmi::isServiceRunning (string service_name) {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	string        query;
	char         *state;
	string        str_state;
	int           retval;

	query = "SELECT * FROM Win32_Service WHERE Name = \"" + service_name + "\"";

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%T)",
				     query.c_str ()));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &state, quickfix,
				    L".State");
			str_state = state;
			retval = (str_state == "Running") ? 1 : 0;
			dhFreeString (state);

			return retval;
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("isServiceRunning error. %s", errstr.c_str ());
	}

	return 0;
}

/** 
 * Get the free space in a logical disk drive.
 * 
 * @param disk_id Disk drive letter (C: for example).
 * 
 * @return Free space amount in MB.
 *
 * @exception Pandora_Wmi_Exception Throwd if an error occured when reading
 *            from WMI database.
 */
unsigned long
Pandora_Wmi::getDiskFreeSpace (string disk_id) {
	CDhInitialize      init;
	CDispPtr           wmi_svc, quickfixes;
	unsigned long long space = 0;
	string             space_str;
	string             query;

	query = "SELECT FreeSpace FROM Win32_LogicalDisk WHERE DeviceID = \"" + disk_id + "\"";
	
	struct QFix {
		CDhStringA free_space; 	 
	};
	
	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%T)",
				     query.c_str ()));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			QFix fix = { 0 };
			dhGetValue (L"%s", &fix.free_space, quickfix,
				    L".FreeSpace");
			
			if (fix.free_space == NULL) 
				return 0;
			
			space_str = fix.free_space;
			dhFreeString (name);

			try {
	 			
				space = Pandora_Strutils::strtoulong (space_str);
			} catch (Pandora_Exception e) {
				throw Pandora_Wmi_Exception (); 	 
			}
			
			return space / 1024 / 1024;
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getDiskFreeSpace error. %s", errstr.c_str ());
	}

	throw Pandora_Wmi_Exception ();
}

/** 
 * Get the CPU usage percentage in the last minutes.
 * 
 * @param cpu_id CPU identifier.
 * 
 * @return The usage percentage of the CPU.
 *
 * @exception Pandora_Wmi_Exception Throwed if an error occured when reading
 *            from WMI database.
 */
int
Pandora_Wmi::getCpuUsagePercentage (int cpu_id) {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	string        query;
	long          load_percentage;
	std::ostringstream stm;

	stm << cpu_id;
	query = "SELECT * FROM Win32_Processor WHERE DeviceID = \"CPU" + stm.str () + "\"";

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%T)",
				     query.c_str ()));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%d", &load_percentage, quickfix,
				    L".LoadPercentage");
		
			return load_percentage;
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getCpuUsagePercentage error. %s", errstr.c_str ());
	}

	throw Pandora_Wmi_Exception ();
}

/** 
 * Get the amount of free memory in the system
 *
 * @return The amount of free memory in MB.
 * @exception Pandora_Wmi_Exception Throwd if an error occured when reading
 *            from WMI database.
 */
long
Pandora_Wmi::getFreememory () {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	long          free_memory;

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     L"SELECT * FROM Win32_PerfRawData_PerfOS_Memory "));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%d", &free_memory, quickfix,
				    L".AvailableMBytes");
		
			return free_memory;
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getFreememory error. %s", errstr.c_str ());
	}

	throw Pandora_Wmi_Exception ();	
}

/**
 * Get the name of the operating system.
 * 
 * @return The name of the operating system.
 */
string
Pandora_Wmi::getOSName () {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *name = NULL;
	string        ret;

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     L"SELECT * FROM Win32_OperatingSystem "));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &name, quickfix,
				    L".Caption");
			
			ret = name;
			dhFreeString (name);
		
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getOSName error. %s", errstr.c_str ());
	}

	return ret;
}

/** 
 * Get the version of the operating system.
 * 
 * @return The version of the operaing system.
 */
string
Pandora_Wmi::getOSVersion () {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *version = NULL;
	string        ret;

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     L"SELECT * FROM Win32_OperatingSystem "));
	
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &version, quickfix,
				    L".CSDVersion");
		
			ret = version;
			dhFreeString (version);
		
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getOSVersion error. %s", errstr.c_str ());
	}

	return ret;
}

/** 
 * Get the build of the operating system.
 * 
 * @return The build of the operating system.
 */
string
Pandora_Wmi::getOSBuild () {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *build = NULL;
	string        ret;
	
	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     L"SELECT * FROM Win32_OperatingSystem "));

		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &build, quickfix,
				    L".Version");
			
			ret = build;
			dhFreeString (build);
			
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getOSBuild error. %s", errstr.c_str ());
	}
	
	return ret;
}

/** 
 * Get the system name of the operating system.
 * 
 * @return The system name of the operating system.
 */
string
Pandora_Wmi::getSystemName () {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *name = NULL;
	string        ret;
	
	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     L"SELECT * FROM Win32_OperatingSystem "));

		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &name, quickfix,
				    L".CSName");
			
			ret = name;
			dhFreeString (name);
			
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("getSystemName error. %s", errstr.c_str ());
	}
	
	return ret;
}

/** 
 * Get a list of events that match a given pattern.
 * 
 * @return The list of events.
 */
void
Pandora_Wmi::getEventList (string source, string type, string code,
			   string pattern, int interval,
			   list<string> &event_list) {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *value = NULL;
	WCHAR        *unicode_value;
	string        event, limit, message, query, timestamp;
	char         *encode;
	
	limit = getTimestampLimit (interval);    
	if (limit.empty()) {
		pandoraDebug ("Pandora_Wmi::getEventList: getTimestampLimit error");
		return;
	}
	
	// Build the WQL query
	query = "SELECT * FROM Win32_NTLogEvent WHERE TimeWritten >= '" + limit + "'";
	if (! source.empty()) {
		query += " AND Logfile = '" + source + "'";
	}    
	if (! type.empty()) {
		query += " AND Type = '" + type + "'";
	}
	if (! code.empty()) {
		query += " AND EventCode = '" + code + "'";
	}

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%s)",
				     query.c_str()));

		FOR_EACH (quickfix, quickfixes, NULL) {
			// Timestamp
			dhGetValue (L"%s", &value, quickfix,
				    L".TimeWritten");
			timestamp = value;
			dhFreeString (value);
			
			// Message
			dhGetValue (L"%S", &unicode_value, quickfix,
				    L".Message");
			value = Pandora_Strutils::strUnicodeToAnsi (unicode_value);
			message = Pandora_Strutils::trim (value);
			dhFreeString (value);
			
			// LIKE is not always available, we have to filter ourselves
			if (pattern.empty () || (message.find (pattern) != string::npos)) {
				event = timestamp + " " + message;
				event_list.push_back(event);
			}
			
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraDebug ("Pandora_Wmi::getEventList: error: %s", errstr.c_str ());
	}
}

/** 
 * Returns the limit date (WMI format) for event searches.
 * 
 * @return The timestamp in WMI format.
 */
string
Pandora_Wmi::getTimestampLimit (int interval) {
	char       limit_str[26], diff_sign;
	time_t     limit_time, limit_time_utc, limit_diff;
	struct tm *limit_tm = NULL, *limit_tm_utc = NULL;
	
	// Get current time
	limit_time = time(0);
	if (limit_time == (time_t)-1) {
		return "";
	}
	
	// Get UTC time
	limit_tm_utc = gmtime (&limit_time);
	limit_time_utc = mktime (limit_tm_utc);
	
	// Calculate the difference in minutes
	limit_diff = limit_time - limit_time_utc;
	if (limit_diff >= 0) {
		diff_sign = '+';
	}
	else {
		diff_sign = '-';
	}
	limit_diff = abs(limit_diff);
	limit_diff /= 60;
	
	// Substract the agent interval
	limit_time -= interval;
	
	limit_tm = localtime (&limit_time);
	if (limit_tm == NULL) {
		return "";
	}
	
	// WMI date format: yyyymmddHHMMSS.xxxxxx+UUU
	snprintf (limit_str, 26, "%.4d%.2d%.2d%.2d%.2d%.2d.000000%c%.3d",
		  limit_tm->tm_year + 1900, limit_tm->tm_mon + 1,
		  limit_tm->tm_mday, limit_tm->tm_hour,
		  limit_tm->tm_min, limit_tm->tm_sec, diff_sign, limit_diff);
	limit_str[25] = '\0';
	
	return string (limit_str);
}

/** 
 * Converts a date in WMI format to SYSTEMTIME format.
 * 
 * @param wmi_date Date in WMI format
 * @param system_time Output system time variable
 */
void
Pandora_Wmi::convertWMIDate (string wmi_date, SYSTEMTIME *system_time)
{
	system_time->wYear = atoi (wmi_date.substr (0, 4).c_str());
	system_time->wMonth = atoi (wmi_date.substr (4, 2).c_str());
	system_time->wDay = atoi (wmi_date.substr (6, 2).c_str());
	system_time->wHour = atoi (wmi_date.substr (8, 2).c_str());
	system_time->wMinute = atoi (wmi_date.substr (10, 2).c_str());
	system_time->wSecond = atoi (wmi_date.substr (12, 2).c_str());
}

/**
 * Runs a program in a new process.
 *
 * @param command Command to run, with parameters
 */
bool
Pandora_Wmi::runProgram (string command) {
	PROCESS_INFORMATION process_info;
	STARTUPINFO         startup_info;
	bool                success;
	char               *cmd;
	
	if (command == "")
		return false;
	
	ZeroMemory (&startup_info, sizeof (startup_info));
	startup_info.cb = sizeof (startup_info);
	ZeroMemory (&process_info, sizeof (process_info));
	
	pandoraDebug ("Start process \"%s\".", command.c_str ());
	cmd = strdup (command.c_str ());
	success = CreateProcess (NULL, cmd, NULL, NULL, FALSE, 0,
				 NULL, NULL, &startup_info, &process_info);
	pandoraFree (cmd);
	
	if (success) {
		pandoraDebug ("The process \"%s\" was started.", command.c_str ());
		return true;
	}
	pandoraLog ("Could not start process \"%s\". Error %d", command.c_str (),
		    GetLastError());
	return false;
}

/**
 * Start a Windows service.
 *
 * @param service_name Service internal name to start.
 *
 * @retval true If the service started.
 * @retval false If the service could not start. A log message is created.
 */
bool
Pandora_Wmi::startService (string service_name) {
	SC_HANDLE manager, service;
	bool      success;
	
	manager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (manager == NULL) {
		pandoraLog ("Could not access to service \"%s\" to start.",
			    service_name.c_str ());
		return false;
	}
	
	service = OpenService (manager, service_name.c_str (), GENERIC_EXECUTE);
	if (service == NULL) {
		pandoraLog ("Could not access to service \"%s\" to start.",
			    service_name.c_str ());
		CloseServiceHandle (manager);
		return false;
	}
	
	success = StartService (service, 0, NULL);
	
	CloseServiceHandle (service);
	CloseServiceHandle (manager);
	
	if (! success) {
		pandoraLog ("Could not start service \"%s\". (Error %d)",
			    service_name.c_str (), GetLastError ());
	}
	
	return success;
}

/**
 * Stop a Windows service.
 *
 * @param service_name Service internal name to stop.
 *
 * @retval true If the service started.
 * @retval false If the service could not stop. A log message is created.
 */
bool
Pandora_Wmi::stopService (string service_name) {
	SC_HANDLE manager, service;
	bool      success;
	SERVICE_STATUS ssStatus; 

	manager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (manager == NULL) {
		pandoraLog ("Could not access to service \"%s\" to stop.",
			    service_name.c_str ());
		return false;
	}
	
	service = OpenService (manager, service_name.c_str (), SERVICE_STOP);
	if (service == NULL) {
		pandoraLog ("Could not access to service \"%s\" to stop.",
			    service_name.c_str ());
		CloseServiceHandle (manager);
		return false;
	}
	
	success = ControlService (service, SERVICE_CONTROL_STOP, &ssStatus);
	
	CloseServiceHandle (service);
	CloseServiceHandle (manager);
	
	if (! success) {
		pandoraLog ("Could not stop service \"%s\". (Error %d)",
			    service_name.c_str (), GetLastError ());
	}
	
	return success;
}

/**
 * Runs a generic WQL query.
 * 
 * @param wmi_query WQL query.
 * @param column Column to retrieve from the query result.
 * @param rows List where the query result will be placed.
 */
void
Pandora_Wmi::runWMIQuery (string wmi_query, string column, list<string> &rows) {
	CDhInitialize init;
	CDispPtr      wmi_svc, quickfixes;
	char         *value = NULL;
    wstring column_w(column.length(), L' ');
    wstring wmi_query_w(wmi_query.length(), L' ');

    // Copy string to wstring.
    std::copy(column.begin(), column.end(), column_w.begin());
    std::copy(wmi_query.begin(), wmi_query.end(), wmi_query_w.begin());

	try {
		dhCheck (dhGetObject (getWmiStr (L"."), NULL, &wmi_svc));
		dhCheck (dhGetValue (L"%o", &quickfixes, wmi_svc,
				     L".ExecQuery(%S)",
				     wmi_query_w.c_str ()));
		FOR_EACH (quickfix, quickfixes, NULL) {
			dhGetValue (L"%s", &value, quickfix,
				    column_w.c_str ());
			if (value != NULL) {
		  	   rows.push_back (value);
		  	}
			dhFreeString (value);		
		} NEXT_THROW (quickfix);
	} catch (string errstr) {
		pandoraLog ("runWMIQuery error. %s", errstr.c_str ());
	}
}



