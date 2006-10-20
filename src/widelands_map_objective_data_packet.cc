/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "util.h"
#include "map.h"
#include "map_objective_manager.h"
#include "map_trigger_manager.h"
#include "profile.h"
#include "trigger_null.h"
#include "widelands_map_objective_data_packet.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Objective_Data_Packet::~Widelands_Map_Objective_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Objective_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if(skip)
      return;

   Profile prof;
   try {
      prof.read( "objective", 0, fs );
   } catch( ... ) {
      // might not be there
      return;
   }
	MapObjectiveManager & mom = egbase->get_map()->get_mom();
	MapTriggerManager   & mtm = egbase->get_map()->get_mtm();

   Section* s = prof.get_section( "global" );

   // read packet version
   int packet_version=s->get_int("packet_version");

   if(packet_version==CURRENT_PACKET_VERSION) {
      while(( s = prof.get_next_section(0))) {
         MapObjective* o = new MapObjective();
         o->set_name( s->get_name() );
         o->set_descr( s->get_safe_string("descr"));
         o->set_is_visible( s->get_safe_bool("visible"));
         o->set_is_optional( s->get_safe_bool("optional"));

         const char* trigname = s->get_safe_string("trigger");
         Trigger * const trig = mtm.get_trigger(trigname);
         if( !trig )
            throw wexception("Unknown trigger referenced in Objective: %s\n", trigname );
         o->set_trigger( static_cast<Trigger_Null*>(trig) ); //mmh, maybe we should check if this is really a Trigger_Null. Aaaa, screw it.

         mom.register_new_objective(o);
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Objective_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

   // Write all the objectives out
	const MapObjectiveManager & mom = egbase->get_map()->get_mom();
	const MapObjectiveManager::Index nr_objectives =
		mom.get_nr_objectives();
	for (MapObjectiveManager::Index i = 0; i < nr_objectives; ++i) {
		const MapObjective & o = mom.get_objective_by_nr(i);
		Section & s = *prof.create_section(o.get_name());
		s.set_string("descr",    o.get_descr());
		s.set_bool  ("visible",  o.get_is_visible());
		s.set_bool  ("optional", o.get_is_optional());
		s.set_string("trigger",  o.get_trigger()->get_name());
   }

   prof.write("objective", false, fs );
}
