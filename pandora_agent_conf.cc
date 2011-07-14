/* Library to manage a list of key-value options.

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

#include <fstream>
#include "pandora_agent_conf.h"
#include "pandora_strutils.h"
#include <iostream>
#include "pandora.h"

using namespace std;
using namespace Pandora;
using namespace Pandora_Strutils;

#define MAX_KEYS 100

Pandora::Pandora_Agent_Conf::Pandora_Agent_Conf () {
	this->key_values = NULL;
	this->collection_list = NULL;
}

/**
 * Destroy a Pandora_Agent_Conf object.
 */
Pandora::Pandora_Agent_Conf::~Pandora_Agent_Conf () {
	delete key_values; 
	delete collection_list;
}

Pandora_Agent_Conf *
Pandora::Pandora_Agent_Conf::getInstance () {
	static Pandora_Agent_Conf *conf = NULL;

	if (conf)
		return conf;
	conf = new Pandora_Agent_Conf ();
	return conf;
}

/**
 * Additional configuration file.
 */
void
Pandora::Pandora_Agent_Conf::parseFile(string path_file, Collection *aux){
	ifstream     file_conf (path_file.c_str ());
	string buffer;
	unsigned int pos;

	if (!file_conf.is_open ()) {
		return;
	}
	
	/* Read and set the file */
	while (!file_conf.eof ()) {
		/* Set the value from each line */
		getline (file_conf, buffer);
		
		/* Ignore blank or commented lines */
		if (buffer[0] != '#' && buffer[0] != '\n' && buffer[0] != '\0') {		
			/*Check if is a collection*/
			pos = buffer.find("file_collection");
			if(pos != string::npos) {
				string collection_name, trimmed_str;
				
				/*Add collection to collection_list*/
				/*The number 15 is the number of character of string file_collection*/
				collection_name = buffer.substr(pos+15);
				
				
				aux = new Collection();
				
				aux->name = trim (collection_name);
				
				/*Check for ".." substring for security issues*/
				if ( collection_name.find("..") == string::npos ) {
					aux->verify = 0;
					collection_list->push_back (*aux);
				}
				continue;
			}
		}
	}
	
	file_conf.close();
	return;
}

/**
 * Create configuration file for drone agents.
 * @param filename Configuration file to open.
 * @param path_broker Configuration file to write.
 */
void
writeBrokerConf(string path_broker, string filename, string name_broker){
	ifstream     file_conf (filename.c_str ());
	ofstream     file_broker (path_broker.c_str ());
	string       buffer;
	string		 comp;
	unsigned int pos;

	/* Read and set the file */
	while (!file_conf.eof ()) {
		/* Set the value from each line */
		getline (file_conf, buffer);
		
		pos = buffer.find("agent_name");
		if (pos != string::npos){
			comp = buffer.substr (0,12);
			if (comp == "# agent_name"){
				buffer = "agent_name "+name_broker+"\n";
			}	
		}
		
		pos = buffer.find("broker_agent");
		if (pos != string::npos){
			continue;
		} else {
			buffer = buffer + "\n";
		}
		file_broker << buffer;
		
	}
	file_conf.close ();
	file_broker.close();
}

void
Pandora::Pandora_Agent_Conf::setFile (string *all_conf){
	string       buffer, filename;
	unsigned int pos;
	Collection *aux;
	
	filename = Pandora::getPandoraInstallDir ();
	filename += "pandora_agent.conf";

	ifstream     file (filename.c_str ());

	if (this->key_values)
		delete this->key_values;
	this->key_values = new list<Key_Value> ();

	if (this->collection_list)
		delete this->collection_list;
	this->collection_list = new list<Collection> ();
	
	if (!file.is_open ()) {
		return;
	}
	
	/* Read and set the file */
	while (!file.eof ()) {
		/* Set the value from each line */
		getline (file, buffer);
		
		/* Ignore blank or commented lines */
		if (buffer[0] != '#' && buffer[0] != '\n' && buffer[0] != '\0') {
				/*Check if is a include*/
				pos = buffer.find("include");
				if (pos != string::npos){
					string path_file;
					unsigned pos_c;
				
					path_file = buffer.substr(pos+8);

					pos_c = path_file.find("\"");
					/* Remove " */
					while (pos_c != string::npos){
						path_file.replace(pos_c, 1, "");
						pos_c = path_file.find("\"",pos_c+1);
					}
					parseFile(path_file, aux);
			}
			/*Check if is a broker_agent*/
			pos = buffer.find("broker_agent");
				if (pos != string::npos){
					string path_broker, name_broker;
					unsigned pos_c;
					int position = 0;
				
					name_broker = buffer.substr(pos+13);
					path_broker = name_broker+".conf";

					all_conf[position] = Pandora::getPandoraInstallDir () + path_broker;	
					position += 1;

					ifstream     file_br (path_broker.c_str ());
					/* Check if already exists the configuration file*/
					if (!file_br){
						file_br.close();
						writeBrokerConf(path_broker, filename, name_broker);
					}	
				}

			/*Check if is a collection*/
			pos = buffer.find("file_collection");
			if(pos != string::npos) {
				string collection_name, trimmed_str;
				
				/*Add collection to collection_list*/
				/*The number 15 is the number of character of string file_collection*/
				collection_name = buffer.substr(pos+15);

				aux = new Collection();
				
				aux->name = trim (collection_name);
				
				/*Check for ".." substring for security issues*/
				if ( collection_name.find("..") == string::npos ) {
					aux->verify = 0;
					collection_list->push_back (*aux);
				}
				continue;
			}
			/*Check if is a module*/
			pos = buffer.find ("module_");
			if (pos == string::npos) {
				Key_Value kv;
				
				kv.parseLine (buffer);
				key_values->push_back (kv);
			}
		}
	}
	file.close ();
}

/**
 * Sets configuration file to Pandora_Agent_Conf object instance.
 * 
 * It parses the filename and initialize the internal structures
 * of configuration values. The configuration file consist of a number of
 * lines of some of these forms:
 *  - <code>name value</code>
 *  - <code>name "value with blankspaces"</code>
 *
 * @param filename Configuration file to open.
 */
void
Pandora::Pandora_Agent_Conf::setFile (string filename) {
	ifstream     file (filename.c_str ());
	string       buffer;
	unsigned int pos;
	Collection *aux;
	
	if (this->key_values)
		delete this->key_values;
	this->key_values = new list<Key_Value> ();

	if (this->collection_list)
		delete this->collection_list;
	this->collection_list = new list<Collection> ();
	
	if (!file.is_open ()) {
		return;
	}
	
	/* Read and set the file */
	while (!file.eof ()) {
		/* Set the value from each line */
		getline (file, buffer);
		
		/* Ignore blank or commented lines */
		if (buffer[0] != '#' && buffer[0] != '\n' && buffer[0] != '\0') {
				/*Check if is a include*/
				pos = buffer.find("include");
				if (pos != string::npos){
					string path_file;
					unsigned pos_c;
				
					path_file = buffer.substr(pos+8);

					pos_c = path_file.find("\"");
					/* Remove " */
					while (pos_c != string::npos){
						path_file.replace(pos_c, 1, "");
						pos_c = path_file.find("\"",pos_c+1);
					}
					parseFile(path_file, aux);
			}

			/*Check if is a collection*/
			pos = buffer.find("file_collection");
			if(pos != string::npos) {
				string collection_name, trimmed_str;
				
				/*Add collection to collection_list*/
				/*The number 15 is the number of character of string file_collection*/
				collection_name = buffer.substr(pos+15);
				
				
				aux = new Collection();
				
				aux->name = trim (collection_name);
				
				/*Check for ".." substring for security issues*/
				if ( collection_name.find("..") == string::npos ) {
					aux->verify = 0;
					collection_list->push_back (*aux);
				}
				continue;
			}
			/*Check if is a module*/
			pos = buffer.find ("module_");
			if (pos == string::npos) {
				Key_Value kv;
				
				kv.parseLine (buffer);
				key_values->push_back (kv);
			}
		}
	}
	file.close ();
}

/**
 * Queries for a configuration value.
 * 
 * This method search in the key_values attribute for a
 * configuration value which match the key supplied.
 *
 * @param key Key to look for.
 *
 * @return The value of the configuration key looked for.
 *         If it could not be found then an empty string is returned.
 */
string
Pandora::Pandora_Agent_Conf::getValue (const string key)
{
	std::list<Key_Value>::iterator i;
	
	for (i = key_values->begin (); i != key_values->end (); i++) {
		if ((*i).getKey () == key) {
			return (*i).getValue ();
		}
	}
	
	return "";
}

/**
 * Queries for a collection name.
 * 
 * This method returns the name of the current
 * collection pointed by an iterator.
 *
 * @return The name of the current colletion 
 * 
 */
string 
Pandora::Pandora_Agent_Conf::getCurrentCollectionName() {
	string aux;
	aux = collection_it->name;
	return aux;
}

/**
 * Queries for a collection check of added to PATH.
 * 
 * This method returns 1 if the collections is in the PATH
 *
 * @return 1 if the collections is added to PATH
 * 
 */
unsigned char
Pandora::Pandora_Agent_Conf::getCurrentCollectionVerify() {
	unsigned char aux;
	aux = collection_it->verify;
	return aux;
}

/**
 * Set check path add field to 1.
 * 
 */
void
Pandora::Pandora_Agent_Conf::setCurrentCollectionVerify() {
	collection_it->verify = 1;
}

/**
 * Check is there is a collection with the same name in the list
 * 
 * @param The name of the collection to check.
 * 
 * @return True if there is a collection with the same name.
 */
bool
Pandora::Pandora_Agent_Conf::isInCollectionList(string name) {
	list<Collection>::iterator p;
	string name_md5;
	for (p = collection_list->begin();p != collection_list->end();p++) {
			name_md5 = p->name+".md5";
			if ( (strcmp(p->name.c_str(), name.c_str()) == 0)  || 
				(strcmp(name_md5.c_str(), name.c_str()) == 0)){
				return true;
			}
	}
	
	return false;
}

/**
 * Set iterator pointing to the first collection of the list.
 * 
 * This method set the iterator pointing to the first collection of the list.
 *
 */
void
Pandora::Pandora_Agent_Conf::goFirstCollection() {
	collection_it = collection_list->begin();
}

/**
 * Move the collection iterator to the next item
 * 
 * This method move the iterator to the next item.
 *
 */
void             
Pandora::Pandora_Agent_Conf::goNextCollection() {
	this->collection_it++;
}

/**
 * Compare the iterator with the last collection.
 * 
 * This method return true if the iterator is pointing to the last collection
 *
 * @return True if the iterator is pointing to the last collection
 * 
 */
bool
Pandora::Pandora_Agent_Conf::isLastCollection() {
	return collection_it == collection_list->end();
}
