/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include <lua.hpp>

#include "c_utils.h"
#include "log.h"
#include "logic/game.h"
#include "logic/player.h"
#include "wui/interactive_player.h"

#include "lua_editor.h"


/* RST
:mod:`wl.editor`
==================

.. module:: wl.editor
   :synopsis: Editor scripting functions

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.editor

The :mod:`wl.editor` module contains functionality that integrates Lua into the
editor. This module is not loaded inside the game, that is if wl.editor is
:const:`nil` the script is run inside a game, else in the editor.
*/



/* RST
Player
------

.. class:: EPlayer(n)

	Child of: :class:`wl.bases.PlayerBase`

	This class represents one of the players in the editor.

	:arg n: player number, range is 1 - :const:`number of players`
	:type n: Integer
*/
const char L_EPlayer::className[] = "EPlayer";
const MethodType<L_EPlayer> L_EPlayer::Methods[] = {
	{0, 0},
};
const PropertyType<L_EPlayer> L_EPlayer::Properties[] = {
	{0, 0, 0},
};


/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

const static struct luaL_reg wleditor [] = {
	{0, 0}
};

void luaopen_wleditor(lua_State * const L) {
	luaL_register(L, "wl.editor", wleditor);
	lua_pop(L, 1); // pop the table from the stack again
	
	register_class<L_EPlayer>(L, "editor", true);
	add_parent<L_EPlayer, L_PlayerBase>(L);
	lua_pop(L, 1); // Pop the meta table
}

