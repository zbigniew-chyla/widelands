/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include <boost/format.hpp>
#include <SDL.h>

#include "rt_errors.h"

#include "sdl_helper.h"

using namespace boost;

namespace RT {

SDL_Surface * empty_sdl_surface(int32_t w, int32_t h, bool alpha) {
	 SDL_Surface *surface;
	 Uint32 rmask, gmask, bmask, amask;
	 /* SDL interprets each pixel as a 32-bit number, so our masks must depend
		 on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	 rmask = 0xff000000;
	 gmask = 0x00ff0000;
	 bmask = 0x0000ff00;
	 amask = alpha ? 0x000000ff : 0;
#else
	 rmask = 0x000000ff;
	 gmask = 0x0000ff00;
	 bmask = 0x00ff0000;
	 amask = alpha ? 0xff000000 : 0;
#endif

	 surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
											  rmask, gmask, bmask, amask);
	 if (!surface)
		 throw RenderError((format("Was unable to create a Surface: %s") % SDL_GetError()).str());

	 return surface;
}

}


