/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "base/rect.h"

Rect::Rect() : x(0), y(0), w(0), h(0) {
}

Rect::Rect(int32_t gx, int32_t gy, uint32_t W, uint32_t H) : x(gx), y(gy), w(W), h(H) {
}

Rect::Rect(const Point& p, uint32_t W, uint32_t H) : Rect(p.x, p.y, W, H) {
}

Point Rect::top_left() const {
	return Point(x, y);
}

Point Rect::bottom_right() const {
	return top_left() + Point(w, h);
}

bool Rect::contains(const Point& pt) const {
	return pt.x >= x && pt.x < x + static_cast<int32_t>(w) && pt.y >= y &&
	       pt.y < y + static_cast<int32_t>(h);
}