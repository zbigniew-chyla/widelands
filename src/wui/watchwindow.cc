/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include "wui/watchwindow.h"

#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/rect.h"
#include "graphic/graphic.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/map_objects/bob.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"
#include "wui/mapview.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

#define NUM_VIEWS 5
#define REFRESH_TIME 5000

// Holds information for a view
struct WatchWindowView {
	MapView::View view;
	Widelands::ObjectPointer tracking;  //  if non-null, we're tracking a Bob
};

struct WatchWindow : public UI::Window {
	WatchWindow(InteractiveGameBase& parent,
	            int32_t x,
	            int32_t y,
	            uint32_t w,
	            uint32_t h,
	            bool single_window_ = false);
	~WatchWindow() override;

	Widelands::Game& game() const {
		return parent_.game();
	}

	boost::signals2::signal<void(Vector2f)> warp_mainview;

	void add_view(Widelands::Coords);
	void next_view();
	void save_coords();
	void close_cur_view();
	void toggle_buttons();

protected:
	void think() override;
	void stop_tracking_by_drag();
	void draw(RenderTarget&) override;

private:
	void do_follow();
	void do_goto();
	void view_button_clicked(uint8_t index);
	void set_current_view(uint8_t idx, bool save_previous = true);

	InteractiveGameBase& parent_;
	MapView map_view_;
	uint32_t last_visit_;
	bool single_window_;
	uint8_t cur_index_;
	UI::Button* view_btns_[NUM_VIEWS];
	std::vector<WatchWindowView> views_;
};

static WatchWindow* g_watch_window = nullptr;

WatchWindow::WatchWindow(InteractiveGameBase& parent,
                         int32_t const x,
                         int32_t const y,
                         uint32_t const w,
                         uint32_t const h,
                         bool const init_single_window)
   : UI::Window(&parent, "watch", x, y, w, h, _("Watch")),
     parent_(parent),
     map_view_(this, game().map(), 0, 0, 200, 166),
     last_visit_(game().get_gametime()),
     single_window_(init_single_window),
     cur_index_(0) {
	UI::Button* followbtn =
	   new UI::Button(this, "follow", 0, h - 34, 34, 34, UI::ButtonStyle::kWuiSecondary,
	                  g_gr->images().get("images/wui/menus/menu_watch_follow.png"), _("Follow"));
	followbtn->sigclicked.connect(boost::bind(&WatchWindow::do_follow, this));

	UI::Button* gotobtn = new UI::Button(
	   this, "center_mainview_here", 34, h - 34, 34, 34, UI::ButtonStyle::kWuiSecondary,
	   g_gr->images().get("images/wui/menus/menu_goto.png"), _("Center the main view on this"));
	gotobtn->sigclicked.connect(boost::bind(&WatchWindow::do_goto, this));

	if (init_single_window) {
		for (uint8_t i = 0; i < NUM_VIEWS; ++i) {
			view_btns_[i] = new UI::Button(
			   this, "view", 74 + (17 * i), 200 - 34, 17, 34, UI::ButtonStyle::kWuiSecondary, "-");
			view_btns_[i]->sigclicked.connect(boost::bind(&WatchWindow::view_button_clicked, this, i));
		}

		UI::Button* closebtn =
		   new UI::Button(this, "close", w - 34, h - 34, 34, 34, UI::ButtonStyle::kWuiSecondary,
		                  g_gr->images().get("images/wui/menu_abort.png"), _("Close"));
		closebtn->sigclicked.connect(boost::bind(&WatchWindow::close_cur_view, this));
	}

	map_view_.field_clicked.connect(
	   [&parent](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		   parent.map_view()->field_clicked(node_and_triangle);
		});
	map_view_.track_selection.connect(
	   [&parent](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		   parent.map_view()->track_selection(node_and_triangle);
		});
	map_view_.changeview.connect([this] { stop_tracking_by_drag(); });
	warp_mainview.connect([&parent](const Vector2f& map_pixel) {
		parent.map_view()->scroll_to_map_pixel(map_pixel, MapView::Transition::Smooth);
	});
}

void WatchWindow::draw(RenderTarget& dst) {
	UI::Window::draw(dst);
	if (!is_minimal()) {
		parent_.draw_map_view(&map_view_, &dst);
	}
}

/**
 * Add a view to a watchwindow, if there is space left.
 *
 * This also resets the view cycling timer.
 */
void WatchWindow::add_view(Widelands::Coords const coords) {
	if (views_.size() >= NUM_VIEWS)
		return;
	WatchWindowView view;

	map_view_.scroll_to_field(coords, MapView::Transition::Jump);

	view.tracking = nullptr;
	view.view = map_view_.view();
	last_visit_ = game().get_gametime();

	views_.push_back(view);
	set_current_view(views_.size() - 1, views_.size() > 1);
	if (single_window_)
		toggle_buttons();
}

// Switch to next view
void WatchWindow::next_view() {
	set_current_view((cur_index_ + 1) % views_.size());
}

// Saves the coordinates of a view if it was already shown (and possibly moved)
void WatchWindow::save_coords() {
	auto& view = views_[cur_index_];
	view.view = map_view_.view();
}

// Enables/Disables buttons for views_
void WatchWindow::toggle_buttons() {
	for (uint32_t i = 0; i < NUM_VIEWS; ++i) {
		if (i < views_.size()) {
			view_btns_[i]->set_title(std::to_string(i + 1));
			view_btns_[i]->set_enabled(true);
		} else {
			view_btns_[i]->set_title("-");
			view_btns_[i]->set_enabled(false);
		}
	}
}

void WatchWindow::set_current_view(uint8_t idx, bool save_previous) {
	assert(idx < views_.size());

	if (save_previous)
		save_coords();

	if (single_window_) {
		view_btns_[cur_index_]->set_perm_pressed(false);
		view_btns_[idx]->set_perm_pressed(true);
	}
	cur_index_ = idx;
	map_view_.set_view(views_[cur_index_].view, MapView::Transition::Jump);
}

WatchWindow::~WatchWindow() {
	g_watch_window = nullptr;
}

/*
===============
Update the map_view_ if we're tracking something.
===============
*/
void WatchWindow::think() {
	UI::Window::think();

	if ((game().get_gametime() - last_visit_) > REFRESH_TIME) {
		last_visit_ = game().get_gametime();
		next_view();
		return;
	}

	if (upcast(Widelands::Bob, bob, views_[cur_index_].tracking.get(game()))) {
		const Widelands::Map& map = game().map();
		const Vector2f field_position = MapviewPixelFunctions::to_map_pixel(map, bob->get_position());
		const Vector2f pos = bob->calc_drawpos(game(), field_position, 1.f);

		// Drop the tracking if it leaves our vision range
		InteractivePlayer* ipl = game().get_ipl();
		if (ipl && 1 >= ipl->player().vision(map.get_index(bob->get_position(), map.get_width()))) {
			// Not in sight
			views_[cur_index_].tracking = nullptr;
		} else {
			map_view_.scroll_to_map_pixel(pos, MapView::Transition::Jump);
		}
	}
}

/*
===============
When the user drags the map_view_, we stop tracking.
===============
*/
void WatchWindow::stop_tracking_by_drag() {
	// Disable switching while dragging
	if (map_view_.is_dragging()) {
		last_visit_ = game().get_gametime();
		views_[cur_index_].tracking = nullptr;
	}
}

/**
 * Called when the user clicks the "follow" button.
 *
 * If we are currently tracking a bob, stop tracking.
 * Otherwise, start tracking the nearest bob from our current position.
 */
void WatchWindow::do_follow() {
	Widelands::Game& g = game();
	if (views_[cur_index_].tracking.get(g)) {
		views_[cur_index_].tracking = nullptr;
	} else {
		//  Find the nearest bob. Other object types can not move and are
		//  therefore not of interest.
		Vector2f center_map_pixel = map_view_.view_area().rect().center();
		const Widelands::Map& map = g.map();
		MapviewPixelFunctions::normalize_pix(map, &center_map_pixel);
		std::vector<Widelands::Bob*> bobs;
		//  Scan progressively larger circles around the given position for
		//  suitable bobs.
		for (Widelands::Area<Widelands::FCoords> area(
		        map.get_fcoords(MapviewPixelFunctions::calc_node_and_triangle(
		                           map, center_map_pixel.x, center_map_pixel.y)
		                           .node),
		        2);
		     area.radius <= 32; area.radius *= 2)
			if (map.find_bobs(area, &bobs))
				break;
		//  Find the bob closest to us
		float closest_dist = 0;
		Widelands::Bob* closest = nullptr;
		for (uint32_t i = 0; i < bobs.size(); ++i) {
			Widelands::Bob* const bob = bobs[i];
			const Vector2f field_position =
			   MapviewPixelFunctions::to_map_pixel(map, bob->get_position());
			const Vector2f p = bob->calc_drawpos(g, field_position, 1.f);
			const float dist = MapviewPixelFunctions::calc_pix_distance(map, p, center_map_pixel);
			InteractivePlayer* ipl = game().get_ipl();
			if ((!closest || closest_dist > dist) &&
			    (!ipl ||
			     1 < ipl->player().vision(map.get_index(bob->get_position(), map.get_width())))) {
				closest = bob;
				closest_dist = dist;
			}
		}
		views_[cur_index_].tracking = closest;
	}
}

/**
 * Called when the "go to" button is clicked.
 *
 * Cause the main map_view_ to jump to our current position.
 */
void WatchWindow::do_goto() {
	warp_mainview(map_view_.view_area().rect().center());
}

/**
 * Sets the current view to @p index and resets timeout.
 */
void WatchWindow::view_button_clicked(uint8_t index) {
	set_current_view(index);
	last_visit_ = game().get_gametime();
}

/**
 * Closes the current view.
 *
 * This is called when the "close" button is clicked (only in single watchwindow mode).
 * If there is only one view remaining, the window itself is closed.
 */
void WatchWindow::close_cur_view() {
	if (views_.size() == 1) {
		die();
		return;
	}

	views_.erase(views_.begin() + cur_index_);
	set_current_view(cur_index_ % views_.size(), false);
	toggle_buttons();
}

/*
===============
show_watch_window

Open a watch window.
===============
*/
void show_watch_window(InteractiveGameBase& parent, const Widelands::Coords& coords) {
	if (g_options.pull_section("global").get_bool("single_watchwin", false)) {
		if (!g_watch_window) {
			g_watch_window = new WatchWindow(parent, 250, 150, 200, 200, true);
		}
		g_watch_window->add_view(coords);
	} else {
		auto* window = new WatchWindow(parent, 250, 150, 200, 200, false);
		window->add_view(coords);
	}
}
