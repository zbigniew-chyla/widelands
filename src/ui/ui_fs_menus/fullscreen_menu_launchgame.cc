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

#include "fullscreen_menu_launchgame.h"

//#include "editor_game_base.h"
#include "fullscreen_menu_loadgame.h"
#include "fullscreen_menu_mapselect.h"
#include "game.h"
#include "gamechatpanel.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "i18n.h"
#include "instances.h"
#include "layered_filesystem.h"
#include "map.h"
#include "map_loader.h"
#include "player.h"
#include "playerdescrgroup.h"
#include "profile.h"

Fullscreen_Menu_LaunchGame::Fullscreen_Menu_LaunchGame
	(GameSettingsProvider * const settings, GameController * const ctrl)
:
Fullscreen_Menu_Base("launchgamemenu.jpg"),
m_select_map
	(this,
	 550, 210, 200, 26,
	 1,
	 &Fullscreen_Menu_LaunchGame::select_map, this,
	 _("Select map"),
	 std::string(),
	 false),
m_select_save
	(this,
	 550, 250, 200, 26,
	 1,
	 &Fullscreen_Menu_LaunchGame::select_savegame, this,
	 _("Select Savegame"),
	 std::string(),
	 false),
m_back
	(this,
	 550, 450, 200, 26,
	 0,
	 &Fullscreen_Menu_LaunchGame::back_clicked, this,
	 _("Back")),
m_ok
	(this,
	 550, 480, 200, 26,
	 2,
	 &Fullscreen_Menu_LaunchGame::start_clicked, this,
	 _("Start game"),
	 std::string(),
	 false),
m_title        (this, MENU_XRES / 2,  70, _("Launch Game"),      Align_HCenter),
m_mapname      (this, 650,           180, std::string(),         Align_HCenter),
m_notes        (this, 50, 110, 700, 60, std::string()),
m_settings     (settings),
m_ctrl         (ctrl),
m_chat         (0),
m_is_scenario  (false),
m_is_savegame  (false)
{

	m_title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	uint32_t y = 150;
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		m_players[i] =
			new PlayerDescriptionGroup(this, 50, y += 30, settings, i);

}


/**
 * In singleplayer:
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void Fullscreen_Menu_LaunchGame::start()
{
	if
		((m_settings->settings().mapname.size() == 0)
		 &&
		 m_settings->canChangeMap()
		 &&
		 not m_settings->settings().multiplayer)
	{
		select_map();

		if (m_settings->settings().mapname.size() == 0)
			end_modal(0); // back was pressed
	}

	refresh();
}


void Fullscreen_Menu_LaunchGame::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * Set a new chat provider.
 *
 * This automatically creates and display a chat panel when appropriate.
 */
void Fullscreen_Menu_LaunchGame::setChatProvider(ChatProvider * const chat)
{
	delete m_chat;
	m_chat = chat ? new GameChatPanel(this, 50, 420, 480, 160, *chat) : 0;
}


/**
 * back-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::back_clicked()
{
	if (! m_settings->settings().multiplayer) {
		//  The following behaviour might look strange at first view, but for the
		//  user it seems as if the launchgame-menu is a child of mapselect and
		//  not the other way around - just end_modal(0); will be seen as bug
		//  from user point of view, so we reopen the mapselect-menu.
		m_settings->setMap("", "", 0);
		select_map();
		if (m_settings->settings().mapname.size() == 0)
			end_modal(0);
		refresh();
	} else
		end_modal(0);
}


/**
 * start-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::start_clicked()
{
	if (!g_fs->FileExists(m_filename))
		throw wexception("Tried to start a game with a file you don't have.");
	if (m_settings->canLaunch()) {
		if (!m_is_savegame)
			end_modal(1 + m_is_scenario);
		else
			end_modal(3 + m_is_scenario);
	}
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void Fullscreen_Menu_LaunchGame::refresh()
{
	GameSettings const & settings = m_settings->settings();

	m_mapname.set_text
		(settings.mapname.size() != 0 ? settings.mapname : _("(no map)"));
	m_is_savegame = settings.savegame;
	m_filename = settings.mapfilename;
	m_nr_players = settings.players.size();

	m_ok.set_enabled(m_settings->canLaunch());
	m_select_map.set_visible(m_settings->canChangeMap());
	m_select_map.set_enabled(m_settings->canChangeMap());
	m_select_save.set_visible
			(m_settings->settings().multiplayer & m_settings->canChangeMap());
	m_select_save.set_enabled
			(m_settings->settings().multiplayer & m_settings->canChangeMap());

	if (m_settings->settings().scenario)
		set_scenario_values();

	// Print warnings and information between title and player desc. group
	if (!g_fs->FileExists(m_filename)) {
		m_notes.set_text
				(_("WARNING!!! Host selected file \"")
				 + m_filename
				 + _("\" for this game, but you don't have it.")
				 + _(" Please add it manually."));
		for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
			m_players[i]->refresh();
		m_notes.set_color(UI_FONT_CLR_WARNING);
	} else {
		if (!m_is_savegame) {
			m_notes.set_text("");

			// update the player description groups
			for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
				m_players[i]->refresh();

		} else { // Multi player savegame information starts here.

			if (m_filename != m_filename_proof)
				// load all playerdata from savegame
				load_previous_playerdata();
			std::string notetext = _("Original:");

			char buf[32];
			int8_t i = 1;

			// Print information about last players
			for (; i <= m_nr_players; ++i) {
				snprintf(buf, sizeof(buf), "  [%i] ", i);
				notetext += buf;

				if (m_player_save_name[i-1].empty()) {
					notetext += "--";
					//  set player description group disabled so that no-one can
					//  take this place
					m_players[i-1]->enable_pdg(false);
					continue;
				}

				// Refresh player description group of this player
				m_players[i-1]->refresh();
				m_players[i-1]->show_tribe_button(false);
				notetext += m_player_save_name[i-1] + " (";
				if (m_player_save_name[i-1].empty())
					throw wexception("Player has a name but no tribe");
				notetext += m_player_save_tribe[i-1] + ")";
			}

			// update remaining player description groups
			for (; i <= MAX_PLAYERS; ++i)
				m_players[i-1]->refresh();

			// Finally set the notes
			m_notes.set_text(notetext);
			m_notes.set_color(UI_FONT_CLR_FG);
		}
	}
}


/**
 * Select a map and send all information to the user interface.
 */
void Fullscreen_Menu_LaunchGame::select_map()
{
	if (!m_settings->canChangeMap())
		return;

	GameSettings const & settings = m_settings->settings();
	Fullscreen_Menu_MapSelect msm;
	msm.setScenarioSelectionVisible(!settings.multiplayer);
	int code = msm.run();

	if (code <= 0) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->setScenario(false);
		return;  // back was pressed
	}

	m_is_scenario = code == 2;
	m_settings->setScenario(m_is_scenario);

	MapData const & mapdata = *msm.get_map();
	m_settings->setMap(mapdata.name, mapdata.filename, mapdata.nrplayers);
	m_is_savegame = false;
	enable_all_pdgs();
}


/**
 * Select a multi player savegame and send all information to the user
 * interface.
 */
void Fullscreen_Menu_LaunchGame::select_savegame()
{
	if (!m_settings->canChangeMap())
		return;

	Widelands::Game game; // The place all data is saved to.
	Fullscreen_Menu_LoadGame lsgm(game);
	int code = lsgm.run();

	if (code <= 0)
		return; // back was pressed

	m_filename = lsgm.filename();

	// Read the needed data from file "elemental" of the used map.
	FileSystem *m_fs = g_fs->MakeSubFileSystem(m_filename.c_str());
	Profile prof;
	prof.read("map/elemental", 0, *m_fs);
	Section & s = prof.get_safe_section("global");

	std::string mapname = _("(Save): ") + std::string(s.get_safe_string("name"));
	m_nr_players = s.get_safe_int("nr_players");

	m_settings->setMap(mapname, m_filename, m_nr_players, true);
	m_is_savegame = true;
	enable_all_pdgs();
}


/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void Fullscreen_Menu_LaunchGame::set_scenario_values()
{
	if (m_settings->settings().mapfilename.size() == 0)
		throw wexception
				("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  Map_Loader needs a place to put it's preload data
	Widelands::Map_Loader * const ml =
		map.get_correct_loader(m_settings->settings().mapfilename.c_str());
	map.set_filename(m_settings->settings().mapfilename.c_str());
	ml->preload_map(true);
	uint8_t nrplayers = map.get_nrplayers();
	for (uint32_t i = 0; i < nrplayers; ++i) {
		m_settings->setPlayerName (i, map.get_scenario_player_name(i+1));
		m_settings->setPlayerTribe(i, map.get_scenario_player_tribe(i+1));
	}
}

/**
 * load all playerdata from savegame
 */
void Fullscreen_Menu_LaunchGame::load_previous_playerdata()
{
	FileSystem *m_fs = g_fs->MakeSubFileSystem(m_filename.c_str());
	Profile prof;
	prof.read("map/player_names", 0, *m_fs);
	std::string strbuf;
	char buf[32];

	int8_t i = 1;
	for (; i <= m_nr_players; ++i) {
		strbuf = "";
		snprintf(buf, sizeof(buf), "player_%i", i);
		Section & s = prof.get_safe_section(buf);
		m_player_save_name [i-1] = s.get_string("name");
		m_player_save_tribe[i-1] = s.get_string("tribe");

		if (m_player_save_tribe[i-1].empty())
			continue; // if tribe is empty, the player does not exist

		// get translated tribename
		strbuf = "tribes/" + m_player_save_tribe[i-1];
		i18n::grab_textdomain(strbuf);
		strbuf += "/conf";
		Profile tribe(strbuf.c_str());
		Section & global = tribe.get_safe_section("tribe");
		m_player_save_tribe[i-1] = global.get_safe_string("name");
		i18n::release_textdomain();
	}
	m_filename_proof = m_filename;
}


/**
 * enables all player description groups.
 * This is a cleanup for the pdgs. Used f.e if user selects a map after a
 * savegame was selected, so all free player positions are reopened.
 */
void Fullscreen_Menu_LaunchGame::enable_all_pdgs()
{
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		m_players[i]->enable_pdg(true);
}
