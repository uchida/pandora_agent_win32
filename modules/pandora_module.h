/* Defines a parent class for a Pandora module.

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

#ifndef	__PANDORA_MODULE_H__
#define	__PANDORA_MODULE_H__

#include "../pandora.h"
#include "pandora_data.h"
#include "boost/regex.h"
#include <list>
#include <string>
#include <ctime>

using namespace Pandora;

/**
 * Definition of Pandora modules.
 */
namespace Pandora_Modules {

	/**
	 * Defines the type of the module.
	 *
	 * The type of a module is the value type the module can have.
	 */
	typedef enum {
		TYPE_0,                  /**< Invalid value               */
		TYPE_GENERIC_DATA,       /**< The value is an integer     */
		TYPE_GENERIC_DATA_INC,   /**< The value is an integer with
					  *  incremental diferences       */
		TYPE_GENERIC_PROC,       /**< The value is a 0 or a 1     */
		TYPE_GENERIC_DATA_STRING, /**< The value is a string       */
		TYPE_ASYNC_DATA, /**< Asynchronous generic_data */
		TYPE_ASYNC_PROC, /**< Asynchronous generic_proc */
		TYPE_ASYNC_STRING, /**< Asynchronous generic_data_string */
		TYPE_LOG /**< Log data */
	} Module_Type;

	const string module_generic_data_str        = "generic_data";
	const string module_generic_data_inc_str    = "generic_data_inc";
	const string module_generic_proc_str        = "generic_proc";
	const string module_generic_data_string_str = "generic_data_string";
	const string module_async_data_str          = "async_data";
	const string module_async_proc_str          = "async_proc";
	const string module_async_string_str        = "async_string";
	const string module_log_str                 = "log";

	/**
	 * Defines the kind of the module.
	 *
	 * The kind of a module is the work the module does.
	 */
	typedef enum {
		MODULE_0,         /**< Invalid kind                    */
		MODULE_EXEC,      /**< The module run a custom command */
		MODULE_PROC,      /**< The module checks for a running
				   *   process                         */
		MODULE_SERVICE,   /**< The module checks for a running
				   *   service                         */
		MODULE_FREEDISK,  /**< The module checks the free      */
		MODULE_FREEDISK_PERCENT,  /**< The module checks the free      */
		MODULE_CPUUSAGE,  /**< The module checks the CPU usage */
		MODULE_INVENTORY, /**< The module gets the inventory of the machine */
		MODULE_FREEMEMORY, /**< The module checks the percentage of 
				   *   freememory in the system        */
		MODULE_FREEMEMORY_PERCENT, /**< The module checks the amount of 
				   *   freememory in the system        */
		MODULE_ODBC,       /**< The module performs a SQL query via ODBC */
		MODULE_LOGEVENT,       /**< The module checks for log events */	
		MODULE_WMIQUERY,       /**< The module runs WQL queries */		
		MODULE_PERFCOUNTER,    /**< The module reads performance counters */
		MODULE_TCPCHECK,       /**< The module checks whether a tcp port is open */
		MODULE_REGEXP,         /**< The module searches a file for matches of a regular expression */
		MODULE_PLUGIN,          /**< Plugin */
		MODULE_PING,            /**< Ping module */
		MODULE_SNMPGET          /**< SNMP get module */
	} Module_Kind;
	
	/**
	 * Defines the structure that holds module conditions.
	 */
	typedef struct {
		double value_1;
		double value_2;
		string string_value;
		string operation;
		string command;
		regex_t regexp;
	} Condition;

	/**
	 * Defines the structure that holds the module cron.
	 */
	typedef struct {
		time_t utimestamp;
		int interval;
		int params[5][2];
	} Cron;

	const string module_exec_str       = "module_exec";
	const string module_proc_str       = "module_proc";
	const string module_service_str    = "module_service";
	const string module_freedisk_str   = "module_freedisk";
	const string module_freedisk_percent_str   = "module_freedisk_percent";
	const string module_freememory_str = "module_freememory";
	const string module_freememory_percent_str = "module_freememory_percent";
	const string module_cpuusage_str   = "module_cpuusage";
	const string module_inventory_str  = "module_inventory";
	const string module_odbc_str       = "module_odbc";
	const string module_logevent_str   = "module_logevent";	
	const string module_wmiquery_str   = "module_wmiquery";	
	const string module_perfcounter_str = "module_perfcounter";
	const string module_tcpcheck_str   = "module_tcpcheck";	
	const string module_regexp_str     = "module_regexp";	
	const string module_plugin_str     = "module_plugin";
	const string module_ping_str       = "module_ping";	
	const string module_snmpget_str    = "module_snmpget";	
	
	/**
	 * Pandora module super-class exception.
	 */
	class Module_Exception : public Pandora::Pandora_Exception    { };
	
	/**
	 * An error happened with the module output.
	 */
	class Output_Error : public Pandora_Modules::Module_Exception { };
	
	/**
	 * The module value is not correct, usually beacause of the limits.
	 */
	class Value_Error : public Pandora_Modules::Module_Exception  { };
	
	/**
	 * The module does not satisfy its interval.
	 */
	class Interval_Not_Fulfilled : public Pandora_Modules::Module_Exception { };
	
	/**
	 * Pandora module super-class.
	 *
	 * Every defined module must inherit of this class.
	 */
	class Pandora_Module {
	private:
		int                   module_interval;
		int                   module_timeout;
		int                   executions;
		int                   max, min;
		string                min_critical, max_critical, min_warning, max_warning;
		string                post_process, disabled, min_ff_event;
		bool                  has_limits, has_min, has_max;
		Module_Type           module_type;
		string                module_kind_str;
		Module_Kind           module_kind;
		string                save;
		list<Condition *>     *condition_list;
		list<Condition *>     *precondition_list;
		Cron				  *cron;
		list<Condition *>     *intensive_condition_list;
		time_t                timestamp;
		unsigned char         intensive_match;
		int                   intensive_interval;
		string                unit, custom_id, str_warning, str_critical;
		string 		      module_group, warning_inverse, critical_inverse, quiet, module_ff_interval;
		string                critical_instructions, warning_instructions, unknown_instructions, tags;

	protected:
		
		list<Pandora_Data *> *data_list;

		/**
		 * Indicates if the module generated output in
		 * his last execution.
		 */
		bool        has_output;
		/**
		 * The name of the module.
		 */
		string      module_name;
		/**
		 * The description of the module.
		 */
		string      module_description;
		
		/**
		 * Flag to set a module as asynchronous
		 */
		bool                  async;
		/**
		 * List of items of the inventory
		 */
        list<Pandora_Data *> *inventory_list;
        /**
         * Data passed to the latest setOutput
         */
		string                latest_output;        
		/**
		 * String containing the module type
		 */
		string                module_type_str;
		
		string getDataOutput (Pandora_Data *data);
		void   cleanDataList ();
	public:
		Pandora_Module                    (string name);
		virtual ~Pandora_Module           ();

		static Module_Type
			parseModuleTypeFromString (string type);
		
		static Module_Kind
			parseModuleKindFromString (string kind);
		
		void         setInterval   (int interval);
		void         setIntensiveInterval   (int intensive_interval);
		int          getInterval   ();
		int          getIntensiveInterval   ();
		void         setTimeout    (int timeout);
		int          getTimeout    ();
		string       getSave ();

		virtual string getXml      ();

		
		virtual void run           ();
		
		virtual void setOutput     (string output);
		virtual void setOutput     (string output,
					    SYSTEMTIME *system_time);
		virtual void setNoOutput   ();
		
		string      getName        () const;
		string      getDescription () const;
		string      getTypeString  () const;
		string      getLatestOutput () const;
		Module_Type getTypeInt     () const;
		Module_Type getModuleType  () const;
		Module_Kind getModuleKind  () const;

		void        setType        (string type);
		void        setKind        (string kind);
		void        setDescription (string description);
		void        setMax         (int value);
		void        setMin         (int value);
		void        setPostProcess (string value);
		void        setMinCritical (string value);
		void        setMaxCritical (string value);
		void        setMinWarning  (string value);
		void        setMaxWarning  (string value);
		void        setDisabled    (string value);
		void        setMinFFEvent  (string value);
		void        setUnit        (string value);
		void        setModuleGroup (string value);
		void        setCustomId    (string value);
		void        setStrWarning  (string value);
		void        setStrCritical (string value);
		void        setCriticalInstructions  (string value);
		void        setWarningInstructions  (string value);
		void        setUnknownInstructions  (string value);
		void        setTags        (string value);
		void        setCriticalInverse  (string value);
		void        setWarningInverse  (string value);
		void        setQuiet       (string value);
		void        setModuleFFInterval  (string value);
		
		void        setAsync       (bool async);
		void        setSave        (string save);

		void        exportDataOutput ();
		void        addGenericCondition (string condition, list<Condition *> **condition_list);
		void		addCondition    (string precondition);
		void		addPreCondition    (string precondition);
		void		addIntensiveCondition    (string intensivecondition);
		int 		evaluatePreconditions ();
		void		evaluateConditions ();
		int         checkCron ();
		void        setCron (string cron_string);
		void        setCronInterval (int interval);
		int         evaluateCondition (string string_value, double double_value, Condition *condition);
		int         evaluateIntensiveConditions ();
		int         hasOutput ();
		void        setTimestamp (time_t timestamp);
		time_t      getTimestamp ();
		void        setIntensiveMatch (unsigned char intensive_match);
		unsigned char getIntensiveMatch ();

	};
}

#endif
