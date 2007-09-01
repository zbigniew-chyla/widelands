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

#ifndef __S__GAME_PRELOAD_DATA_PACKET_H
#define __S__GAME_PRELOAD_DATA_PACKET_H

#include "game_data_packet.h"

#include <string>

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */
struct Game_Preload_Data_Packet : public Game_Data_Packet {
      virtual ~Game_Preload_Data_Packet();

	virtual void Read
		(FileSystem &, Game*, Widelands_Map_Map_Object_Loader * const = 0)
		throw (_wexception);
	virtual void Write
		(FileSystem &, Game*, Widelands_Map_Map_Object_Saver * const = 0)
		throw (_wexception);

      const char* get_mapname() {return m_mapname.c_str();}
      uint get_gametime() {return m_gametime;}

private:
      std::string m_mapname;
      uint m_gametime;
};

#endif
