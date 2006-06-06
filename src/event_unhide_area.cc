/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
#include "event_unhide_area.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "wexception.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Unhide_Area::Event_Unhide_Area(void) {
	set_name(_("Unhide Area").c_str());
   set_coords(Coords(0,0));
   set_player(1);
   set_area(5);
}

Event_Unhide_Area::~Event_Unhide_Area(void) {
}

/*
 * reinitialize
 */
void Event_Unhide_Area::reinitialize(Game* g) {
}

/*
 * File Read, File Write
 */
void Event_Unhide_Area::Read(Section* s, Editor_Game_Base* egbase) {
   int version = s->get_safe_int("version");

   if(version == EVENT_VERSION) {
      m_pt.x=s->get_safe_int("point_x");
      m_pt.y=s->get_safe_int("point_y");

      set_area( s->get_safe_int("area"));

      int player= s->get_safe_int("player");
      set_player(player);

      if(m_pt.x<0 || m_pt.y<0 || m_pt.x>=((int)egbase->get_map()->get_width()) || m_pt.y>=((int)egbase->get_map()->get_height()) || player<=0 || player>egbase->get_map()->get_nrplayers()) {
         // give a warning
         log("Unhide Area Event with illegal coordinates or player: (%i,%i) (Player: %i) deleted!\n", m_pt.x, m_pt.y, player);
      }
      return;
   }
   throw wexception("Unhide Area Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Unhide_Area::Write(Section* s, Editor_Game_Base *egbase) {
   // the version
   s->set_int("version", EVENT_VERSION);

   // Point
   s->set_int("point_x", m_pt.x );
   s->set_int("point_y", m_pt.y );

   // Area
   s->set_int("area", get_area());

   // Player
   s->set_int("player", get_player());
}

/*
 * run the event
 */
Event::State Event_Unhide_Area::run(Game* game) {
   assert(m_pt.x!=-1 && m_pt.y!=-1);
   assert(m_player>0 && m_player<=game->get_map()->get_nrplayers());

   Player* player=game->get_player(m_player);
   player->set_area_seen(Coords(m_pt.x,m_pt.y),get_area(), true);

   m_state = DONE;
   return m_state;
}


