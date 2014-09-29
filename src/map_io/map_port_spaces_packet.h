/*
 * Copyright (C) 2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_MAP_IO_MAP_PORT_SPACES_PACKET_H
#define WL_MAP_IO_MAP_PORT_SPACES_PACKET_H

#include <stdint.h>

#include "map_io/map_data_packet.h"

namespace Widelands {

class Map;

/// The port data packet contains all port build spaces
struct MapPortSpacesPacket {
	void Read(FileSystem&, EditorGameBase&, bool, MapObjectLoader&);
	void Write(FileSystem&, EditorGameBase&, MapObjectSaver&);

	//  The following function prereads a given map without the need of a
	//  properly configured EditorGameBase object.
	void Pre_Read(FileSystem &, Map*);

	uint32_t get_version() {return m_version;}

private:
	uint32_t m_version;
};

}

#endif  // end of include guard: WL_MAP_IO_MAP_PORT_SPACES_PACKET_H