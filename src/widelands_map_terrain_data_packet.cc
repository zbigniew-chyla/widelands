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

#include <map>
#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_terrain_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Terrain_Data_Packet::~Widelands_Map_Terrain_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Terrain_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base * egbase,
 const bool,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   FileRead fr;
   fr.Open( fs, "binary/terrain");

	Map & map = egbase->map();
	World & world = map.world();

   // first packet version
   int packet_version=fr.Unsigned16();

	if(packet_version==CURRENT_PACKET_VERSION) {
		const Uint16 nr_terrains = fr.Unsigned16();

      // construct ids and map
		std::map<const Uint16, const Terrain_Descr *> smap;
		for (Uint16 i = 0; i < nr_terrains; ++i) {
			const Uint16       id   = fr.Unsigned16();
         const char * const name = fr.CString   ();
			const std::map<const Uint16, const Terrain_Descr *>::const_iterator
				it = smap.find(id);
			if (it != smap.end()) log
				("Widelands_Map_Terrain_Data_Packet::Read: WARNING: Found "
				 "duplicate terrain id %i: Previously defined as \"%s\", now as "
				 "\"%s\".\n",
				 id,
				 it->second->get_name(),
				 name);
			if (not world.get_terrain(name)) throw wexception
				("Terrain '%s' exists in map, not in world!", name);
			smap[id] = world.get_terrain(name);
      }

      // Now get all the terrains
		const Map::Index max_index = map.max_index();
		for (Map::Index i = 0; i < max_index; ++i) {
			Field & f = map[i];
			f.set_terrainr(*smap[fr.Unsigned8()]);
			f.set_terraind(*smap[fr.Unsigned8()]);
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Terrain_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{

   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the terrains at run time doens't matter.
   // This is slow like hell.
   // Write the number of terrains
	const Map & map = egbase->map();
	const World & world = map.world();
	const Terrain_Descr::Index nr_terrains = world.get_nr_terrains();
   fw.Unsigned16(nr_terrains);

   // Write all terrain names and their id's
	std::map<const char * const, Terrain_Descr::Index> smap;
	for (Terrain_Descr::Index i = 0; i < nr_terrains; ++i) {
		const char * const name = world.get_terrain(i)->get_name();
		smap[name] = i;
      fw.Unsigned16(i);
		fw.CString(name);
   }

   // Now, all terrains as unsigned chars in order
	const Map::Index max_index = map.max_index();
	for (Map::Index i = 0; i < max_index; ++i) {
		Field & f = map[i];
		fw.Unsigned8(smap[f.get_terr().get_name()]);
		fw.Unsigned8(smap[f.get_terd().get_name()]);
   }

   fw.Write( fs, "binary/terrain");
   // DONE
}
