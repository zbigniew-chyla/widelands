/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "campvis.h"
#include "filesystem.h"
#include "profile.h"
#include "wexception.h"

#include <sys/stat.h>
#include <stdlib.h>

/**
 * Get the path of campaign visiblity save-file
 */
std::string Campaign_visiblity_save::get_path()
{
	std::string savepath;
	if (char const * const buf = getenv("HOME")) {
		savepath  = std::string(buf);
		savepath += "/.widelands";
		g_fs->EnsureDirectoryExists(savepath);
	}
	savepath += "/ssave";
	// Make sure ssave directory exists in any case.
	g_fs->EnsureDirectoryExists(savepath);

	savepath += "/campvis"; //  add the name of save-file

	// check if campaigns visiblity-save is available
	if (!(g_fs->FileExists(savepath))) {
		make_campvis(savepath);
	}

	return savepath;
}


/**
 * Create the campaign visiblity save-file of the user
 */
void Campaign_visiblity_save::make_campvis(std::string savepath)
{
	int32_t i = 0;
	int32_t imap = 0;
	char csection[12];
	char cvisible[12];
	char number[4];
	std::string mapsection;
	std::string cms;

	// read in the campaign config
	Profile prof("campaigns/cconfig");
	Section & cconf_s = prof.get_safe_section("global");

	// Write down visiblity of campaigns
	Profile campvis(savepath.c_str());
	Section *vis;
	vis=campvis.pull_section("campaigns");

	sprintf(cvisible, "campvisi%i", i);
	sprintf(csection, "campsect%i", i);
	while (cconf_s.get_string(csection)) {
		vis->set_bool(csection, cconf_s.get_bool(cvisible), "0");

		++i;
		sprintf(cvisible, "campvisi%i", i);
		sprintf(csection, "campsect%i", i);
	}

	// Write down visiblity of campaign maps
	vis = campvis.pull_section("campmaps");
	i = 0;

	sprintf(csection, "campsect%i", i);
	while (cconf_s.get_string(csection)) {
		mapsection = cconf_s.get_string(csection);

		cms = mapsection;
		sprintf(number, "%02i", imap);
		cms += number;

		while (Section * const s = prof.get_section(cms.c_str())) {
			vis->set_bool(cms.c_str(), s->get_bool("visible"), "0");

			++imap;
			cms = mapsection;
			sprintf(number, "%02i", imap);
			cms += number;
		}

		++i;
		sprintf(csection, "campsect%i", i);
		imap = 0;
	}

	campvis.write(savepath.c_str(), false);
}


/**
 * Set an campaign entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param visible should the map be visible?
 */
void Campaign_visiblity_save::set_campaign_visiblity(std::string entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	Section *vis;

	vis=campvis.pull_section("campaigns");
	vis->set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}


/**
 * Set an campaignmap entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param visible should the map be visible?
 */
void Campaign_visiblity_save::set_map_visiblity(std::string entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	Section *vis;

	vis=campvis.pull_section("campmaps");
	vis->set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}
