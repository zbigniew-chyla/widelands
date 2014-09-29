/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_io/game_map_packet.h"

#include <memory>

#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "map_io/map_saver.h"
#include "map_io/widelands_map_loader.h"

namespace Widelands {

GameMapPacket::~GameMapPacket() {
	delete m_wms;
	delete m_wml;
}

void GameMapPacket::Read
	(FileSystem & fs, Game & game, MapObjectLoader * const)
{
	if (!fs.FileExists("map") || !fs.IsDirectory("map"))
		throw GameDataError("no map");

	//  Now Load the map as it would be a normal map saving.
	delete m_wml;

	m_wml = new WidelandsMapLoader(fs.MakeSubFileSystem("map"), &game.map());

	m_wml->preload_map(true);

	//  DONE, mapfs gets deleted by WidelandsMapLoader.

	return;
}


void GameMapPacket::Read_Complete(Game & game) {
	m_wml->load_map_complete(game, true);
	m_mol = m_wml->get_map_object_loader();
}


void GameMapPacket::Write
	(FileSystem & fs, Game & game, MapObjectSaver * const)
{

	std::unique_ptr<FileSystem> mapfs
		(fs.CreateSubFileSystem("map", FileSystem::DIR));

	//  Now Write the map as it would be a normal map saving.
	delete m_wms;
	m_wms = new MapSaver(*mapfs, game);
	m_wms->save();
	m_mos = m_wms->get_map_object_saver();
}

}