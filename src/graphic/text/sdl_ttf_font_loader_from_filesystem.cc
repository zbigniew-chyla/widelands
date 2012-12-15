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

#include <string>

#include <boost/format.hpp>

#include "rt_errors.h"
#include "sdl_ttf_font_impl.h"

#include "io/fileread.h"
#include "io/filesystem/filesystem.h"

using namespace std;
using namespace boost;

namespace RT {

class SDLTTF_FontLoaderFromFilesystem : public IFontLoader {
public:
	SDLTTF_FontLoaderFromFilesystem(FileSystem* fs);
	virtual ~SDLTTF_FontLoaderFromFilesystem();
	virtual IFont * load(const string& name, int ptsize);

private:
	FileSystem* fs_;
};

SDLTTF_FontLoaderFromFilesystem::SDLTTF_FontLoaderFromFilesystem(FileSystem* fs)
	: fs_(fs) {
}
SDLTTF_FontLoaderFromFilesystem::~SDLTTF_FontLoaderFromFilesystem() {
}

IFont* SDLTTF_FontLoaderFromFilesystem::load(const string& face, int ptsize) {
	std::string filename = "fonts/";
	filename += face;

	FileRead fr;
	fr.Open(*fs_, filename.c_str());

	SDL_RWops* ops = SDL_RWFromMem(fr.Data(0), fr.GetSize());
	if (!ops)
		throw BadFont("could not load font!: RWops Pointer invalid");

	TTF_Font* font = TTF_OpenFontIndexRW(ops, true, ptsize, 0);
	if (!font)
		throw BadFont((format("could not load font!: %s") % TTF_GetError()).str());

	if(!font)
		throw BadFont((format("Font loading error for %s, %i pts: %s") % face % ptsize % TTF_GetError()).str());

	return new SDLTTF_Font(font, face, ptsize);
}

IFontLoader * ttf_fontloader_from_filesystem(FileSystem* fs) {
	return new SDLTTF_FontLoaderFromFilesystem(fs);
}

}

