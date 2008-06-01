/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "tribe.h"

#include "editor_game_base.h"
#include "game.h"
#include "helper.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "profile.h"
#include "soldier.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"
#include "widelands_fileread.h"
#include "world.h"

#include "upcast.h"

#include <iostream>

using namespace std;

namespace Widelands {

//
// Tribe_Descr class
//
Tribe_Descr::Tribe_Descr(const std::string & tribename, const World & the_world)
: m_name(tribename), m_world(the_world)
{
	try {
		char directory[256];

		//  Grab the localization textdomain.
		snprintf(directory, sizeof(directory), "tribes/%s", tribename.c_str());
		i18n::Textdomain textdomain(directory);

		snprintf(directory, sizeof(directory), "tribes/%s", tribename.c_str());

		m_default_encdata.clear();
		parse_wares    (directory);
		parse_workers  (directory);
		parse_buildings(directory);
		parse_bobs     (directory);
		parse_root_conf(directory);
	}
	catch (std::exception &e)
	{throw wexception("Error loading tribe %s: %s", tribename.c_str(), e.what());}
}


/*
===============
Tribe_Descr::postload

Load all logic data
===============
*/
void Tribe_Descr::postload(Editor_Game_Base *) {
	// TODO: move more loads to postload
}

/*
===============
Tribe_Descr::load_graphics

Load tribe graphics
===============
*/
void Tribe_Descr::load_graphics()
{
	for (Ware_Index i = Ware_Index::First(); i < m_workers.get_nitems(); ++i)
		m_workers.get(i)->load_graphics();

	for (Ware_Index i = Ware_Index::First(); i < m_wares.get_nitems  (); ++i)
		m_wares.get(i)->load_graphics();

	for
		(Building_Index i = Building_Index::First();
		 i < m_buildings.get_nitems();
		 ++i)
		m_buildings.get(i)->load_graphics();
}


//
// down here: private read functions for loading
//

/*
===============
Tribe_Descr::parse_root_conf

Read and process the main conf file
===============
*/
void Tribe_Descr::parse_root_conf(const char *directory)
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	try
	{
		Profile prof(fname);

		{
			Section & tribe_s = prof.get_safe_section("tribe");
			tribe_s.get_string("author");
			tribe_s.get_string("name"); // descriptive name
			tribe_s.get_string("descr"); // long description
			m_bob_vision_range = tribe_s.get_int("bob_vision_range");
		}

		if (Section * const defaults_s = prof.get_section("defaults"))
			m_default_encdata.parse(*defaults_s);

		m_anim_frontier =
			g_anim.get
				(directory,
				 prof.get_safe_section("frontier"),
				 0,
				 &m_default_encdata);

		m_anim_flag =
			g_anim.get
				(directory,
				 prof.get_safe_section("flag"),
				 0,
				 &m_default_encdata);

		{
			Section & swa_s = prof.get_safe_section("startwares");
			while (Section::Value const * const value = swa_s.get_next_val(0)) {
				if (not m_wares.exists(value->get_name()))
					throw wexception
						("In section [startwares], ware %s is not know!",
						 value->get_name());
				m_startwares[value->get_name()] = value->get_int();
			}
		}

		{
			Section & swo_s = prof.get_safe_section("startworkers");
			while (Section::Value const * const value = swo_s.get_next_val(0)) {
				if (strcmp(value->get_name(), "soldier")) { // Ignore soldiers here
					if (not m_workers.exists(value->get_name()))
						throw wexception
							("In section [startworkers], worker %s is not know!",
							 value->get_name());
					m_startworkers[value->get_name()] = value->get_int();
				}
			}
		}

		{
			Section & sso_s = prof.get_safe_section("startsoldiers");
			while (Section::Value const * const value = sso_s.get_next_val(0))
				//  NOTE no check here; we do not know about max levels and so on
				m_startsoldiers[value->get_name()] = value->get_int();
		}
	} catch (const std::exception & e) {
		throw wexception("%s: %s", fname, e.what());
	}
}


/*
===============
Tribe_Descr::parse_buildings

Read all the building descriptions
===============
*/
void Tribe_Descr::parse_buildings(const char *rootdir)
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/buildings", rootdir);

	g_fs->FindFiles(subdir, "*", &dirs);

	for (filenameset_t::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		Building_Descr *descr = 0;

		try {
			descr = Building_Descr::create_from_dir
				(*this, it->c_str(), &m_default_encdata);
		} catch (std::exception &e) {
			log("Building %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch (...) {
			log("Building %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			m_buildings.add(descr);
	}

	//  Calculate recursive workarea info. For each building, add info to
	//  m_recursive_workarea_info from every building that can be reached through
	//  at least 1 sequence of enhancement operations (including the empty
	//  sequence).
	for
		(Building_Index i = Building_Index::First();
		 i < m_buildings.get_nitems();
		 ++i)
	{
		Workarea_Info & collected_info
			= get_building_descr(i)->m_recursive_workarea_info;
		std::set<Building_Index> to_consider, considered;
		to_consider.insert(i);
		while (not to_consider.empty()) {
			std::set<Building_Index>::iterator const consider_now_iterator
				= to_consider.begin();
			Building_Index const consider_now = *consider_now_iterator;
			const Building_Descr & considered_building_descr
				= *get_building_descr(consider_now);
			to_consider.erase(consider_now_iterator);
			considered.insert(consider_now);
			{  //  Enhancements from the considered building
				const std::vector<char*> & enhancements =
					considered_building_descr.enhances_to();
				const std::vector<char *>::const_iterator enhancements_end =
					enhancements.end();
				for
					(std::vector<char*>::const_iterator it = enhancements.begin();
					 it != enhancements_end;
					 ++it)
				{
					const Building_Index index = m_buildings.get_index(*it);
					if (not index) {
						log
							("        Warning: building %s (%u) does not exist\n",
							 *it, index.value());
					}
					else if (considered.find(index) == considered.end())
						//  The building index has not been considered. Add it to
						//  to_consider.
						to_consider.insert(index);
				}
			}
			{
				//  Merge collected info.
				const Workarea_Info & ci = considered_building_descr.m_workarea_info;
				const Workarea_Info::const_iterator ci_end = ci.end();
				for
					(Workarea_Info::const_iterator it = ci.begin();
					 it != ci_end;
					 ++it)
				{
					const int32_t radius = it->first;
					const std::set<std::string> & descriptions = it->second;
					for
						(std::set<std::string>::const_iterator di =
						 descriptions.begin();
						 di != descriptions.end();
						 ++di)
						collected_info[radius].insert(*di);
				}
			}
		}
	}

}


/*
===============
Tribe_Descr::parse_workers

Read all worker descriptions
===============
*/
void Tribe_Descr::parse_workers(const char *directory)
{
	filenameset_t dirs;
	{
		char subdir[256];
		snprintf(subdir, sizeof(subdir), "%s/workers", directory);
		g_fs->FindFiles(subdir, "*", &dirs);
	}
	Worker_Descr::becomes_map_t becomes_map;

	for (filenameset_t::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		Worker_Descr *descr = 0;

		try {
			descr =
				Worker_Descr::create_from_dir
				(*this, becomes_map, it->c_str(), &m_default_encdata);
		} catch (std::exception &e) {
			log("Worker %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch (...) {
			log("Worker %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			m_workers.add(descr);
	}

	//  Resolve inter-worker-type dependencies.
	Worker_Descr::becomes_map_t::const_iterator const becomes_map_end =
		becomes_map.end();
	for
		(Worker_Descr::becomes_map_t::const_iterator it = becomes_map.begin();
		 it != becomes_map_end;
		 ++it)
		if (not (it->first->m_becomes = worker_index(it->second.c_str())))
			throw wexception
				("worker type %s becomes nonexistent worker type \"%s\"",
				 it->first->name().c_str(), it->second.c_str());
}

/*
===============
Tribe_Descr::parse_wares

Parse the wares belonging to this tribe, adding it to the games warelist. This is delayed until the game starts,
and is called by the Game class
===============
*/
void Tribe_Descr::parse_wares(const char* directory)
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/wares", directory);

	g_fs->FindFiles(subdir, "*", &dirs);

	for (filenameset_t::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", it->c_str());

		if (!g_fs->FileExists(fname))
			continue;

		const char *warename;
		const char *slash = strrchr(it->c_str(), '/');
		const char *backslash = strrchr(it->c_str(), '\\');

		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		if (slash)
			warename = slash+1;
		else
			warename = it->c_str();

		if (m_wares.exists(warename))
			log("Ware %s is already known in world init\n", it->c_str());

		Item_Ware_Descr* descr = 0;

		try
		{
			descr = Item_Ware_Descr::create_from_dir(warename, it->c_str());
		}
		catch (std::exception& e)
		{
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		}
		catch (...)
		{
			cerr << it->c_str() << ": Unknown exception" << endl;
		}

		if (descr)
			m_wares.add(descr);
	}
}

/*
 * Parse the player bobs (animations, immovables, critters)
 */
void Tribe_Descr::parse_bobs(const char* directory) {
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/bobs", directory);

	g_fs->FindFiles(subdir, "*", &dirs);

	for (filenameset_t::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", it->c_str());

		if (!g_fs->FileExists(fname))
			continue;

		const char *dirname;
		const char *slash = strrchr(it->c_str(), '/');
		const char *backslash = strrchr(it->c_str(), '\\');

		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		if (slash)
			dirname = slash+1;
		else
			dirname = it->c_str();

		try
		{
			Profile prof(fname, "global"); // section-less file
			if
				(strcasecmp
				 	(prof.get_safe_section("global").get_safe_string("type"),
				 	 "critter"))
			{
				Immovable_Descr * const descr = new Immovable_Descr(this, dirname);
				descr->parse(it->c_str(), &prof);
				m_immovables.add(descr);
			} else
				m_bobs.add
					(Bob::Descr::create_from_dir
					 	(dirname, it->c_str(), &prof, this));
		} catch (std::exception &e) {
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		} catch (...) {
			cerr << it->c_str() << ": unknown exception (garbage directory?)" << endl;
		}
	}
}

/*
===========
This loads a warehouse with the given start wares as defined in
the conf files
===========
*/
void Tribe_Descr::load_warehouse_with_start_wares
	(Editor_Game_Base & egbase, Warehouse & wh) const
{
	typedef starting_resources_map::const_iterator smit;
	{
		const smit startwares_end = m_startwares.end();
		for (smit it = m_startwares.begin(); it != startwares_end; ++it)
			wh.insert_wares
			(safe_ware_index(it->first.c_str()), it->second);
	}
	{
		const smit startworkers_end = m_startworkers.end();
		for
			(smit it = m_startworkers.begin();
			 it != startworkers_end;
			 ++it)
			wh.insert_workers
				(safe_worker_index(it->first.c_str()), it->second);
	}
	{
		const smit startsoldiers_end = m_startsoldiers.end();
		for
			(smit it = m_startsoldiers.begin();
			 it != startsoldiers_end;
			 ++it)
		{
			const std::vector<std::string> list(split_string(it->first, "/"));

			if (list.size() != 4)
				throw wexception
					("Error in tribe (%s), startsoldier %s is not valid!",
					 name().c_str(),
					 it->first.c_str());

			char * endp;
			long int const hplvl      = strtol(list[0].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad hp level '%s'", list[0].c_str());
			long int const attacklvl  = strtol(list[1].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad attack level '%s'", list[1].c_str());
			long int const defenselvl = strtol(list[2].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad defense level '%s'", list[2].c_str());
			long int const evadelvl   = strtol(list[3].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad evade level '%s'", list[3].c_str());

			if (upcast(Game, game, &egbase))
				for (int32_t i = 0; i < it->second; ++i) {
					Soldier & soldier = static_cast<Soldier &>
						(dynamic_cast<Soldier_Descr const *>
						 	(get_worker_descr(worker_index("soldier")))
						 ->create(*game, wh.owner(), wh, wh.get_position()));
					soldier.set_level(hplvl, attacklvl, defenselvl, evadelvl);
					wh.incorporate_worker(game, &soldier);
				}
			//  TODO what to do in editor
		}
	}
}


/*
 * does this tribe exist?
 */
bool Tribe_Descr::exists_tribe(const std::string & name, TribeBasicInfo* info) {
	std::string buf = "tribes/";
	buf            += name;
	buf            += "/conf";

	FileRead f;
	if (f.TryOpen(*g_fs, buf.c_str())) {
		if (info) {
			Profile prof(buf.c_str());
			info->name = name;
			info->uiposition =
				prof.get_safe_section("tribe").get_int("uiposition", 0);
		}

		return true;
	}

	return false;
}

struct TribeBasicComparator {
	bool operator()(const TribeBasicInfo& t1, const TribeBasicInfo& t2) {
		return t1.uiposition < t2.uiposition;
	}
};

/**
 * Fills the given string vector with the names of all tribes that exist.
 */
void Tribe_Descr::get_all_tribenames(std::vector<std::string> & target) {
	assert(target.empty());

	//  get all tribes
	std::vector<TribeBasicInfo> tribes;
	filenameset_t m_tribes;
	g_fs->FindFiles("tribes", "*", &m_tribes);
	for
		(filenameset_t::iterator pname = m_tribes.begin();
		 pname != m_tribes.end();
		 ++pname)
	{
		const std::string name = pname->substr(7);
		TribeBasicInfo info;
		if (Tribe_Descr::exists_tribe(name, &info))
			tribes.push_back(info);
	}

	std::sort(tribes.begin(), tribes.end(), TribeBasicComparator());
	for
		(std::vector<TribeBasicInfo>::const_iterator it = tribes.begin();
		 it != tribes.end();
		 ++it)
	{
		target.push_back(it->name);
	}
}

/*
==============
Resource_Descr::get_indicator

Find the best matching indicator for the given amount.
==============
*/
uint32_t Tribe_Descr::get_resource_indicator
	(Resource_Descr const * const res, uint32_t const amount) const
{
	if (not res or not amount) {
		int32_t idx = get_immovable_index("resi_none");
		if (idx == -1)
			throw wexception
				("Tribe %s doesn't declare a resource indicator resi_none!",
				 name().c_str());
		return idx;
	}

	char buffer[256];

	int32_t i = 1;
	int32_t num_indicators = 0;
	for (;;) {
		snprintf(buffer, sizeof(buffer), "resi_%s%i", res->name().c_str(), i);
		if (get_immovable_index(buffer) == -1)
			break;
		++i;
		++num_indicators;
	}

	if (not num_indicators)
		throw wexception
			("Tribe %s doesn't declar a resource indicator for resource %s!",
			 name().c_str(),
			 res->name().c_str());

	uint32_t bestmatch =
		static_cast<uint32_t>
		((static_cast<float>(amount)/res->get_max_amount()) * num_indicators);
	if (static_cast<int32_t>(amount) < res->get_max_amount())
		bestmatch += 1; // Resi start with 1, not 0

	snprintf
		(buffer, sizeof(buffer), "resi_%s%i", res->name().c_str(), bestmatch);

	// NoLog("Resource(%s): Indicator '%s' for amount = %u\n",
	//res->get_name(), buffer, amount);



	return get_immovable_index(buffer);
}

/*
 * Return the given ware or die trying
 */
Ware_Index Tribe_Descr::safe_ware_index(const char * const warename) const {
	Ware_Index const result = ware_index(warename);

	if (not result)
		throw wexception
			("tribe %s does not define ware type \"%s\"",
			 name().c_str(), warename);
	return result;
}

/*
 * Return the given worker or die trying
 */
Ware_Index Tribe_Descr::safe_worker_index(const char * const workername) const {
	Ware_Index const result = worker_index(workername);

	if (not result)
		throw wexception
			("tribe %s does not define worker type \"%s\"",
			 name().c_str(), workername);
	return result;
}

/*
 * Return the given building or die trying
 */
Building_Index Tribe_Descr::safe_building_index(const char *buildingname) const {
	Building_Index const result = building_index(buildingname);

	if (not result)
		throw wexception
			("tribe %s does not define building type \"%s\"",
			 name().c_str(), buildingname);
	return result;
}

};
