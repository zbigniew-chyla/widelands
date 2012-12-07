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
#include <map>

#include <SDL.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include "graphic/text/rt_errors.h"
#include "graphic/text/sdl_helper.h"
#include "upcast.h"

#include "lodepng.h"

#include "thin_graphic.h"

using namespace std;
using namespace boost;


#include <iostream> // TODO(sirver): Remove me

class ThinSDLSurfacePA : public virtual IBlitableSurface {
public:
	ThinSDLSurfacePA(SDL_Surface* surf) : surf_(surf) {}
	virtual ~ThinSDLSurfacePA() {}

	 // TODO(sirver): Should only be w, should be const
	virtual uint32_t get_w() const {return surf_->w;}
	virtual uint32_t get_h() const {return surf_->h;}
	virtual SDL_PixelFormat const & format() const {return *surf_->format;}
	virtual uint16_t get_pitch() const {return surf_->pitch;}
	virtual uint8_t * get_pixels() const {
		return static_cast<uint8_t*>(surf_->pixels);
	}

	virtual void lock(LockMode) {SDL_LockSurface(surf_);}
	virtual void unlock(UnlockMode) {SDL_UnlockSurface(surf_);}

	// TODO(sirver): These functions are wrong
	virtual uint32_t get_pixel(uint32_t x, uint32_t y) {
		return static_cast<uint32_t*>(surf_->pixels)[y*surf_->pitch + x];
	}
	virtual void set_pixel(uint32_t x, uint32_t y, uint32_t clr) {
		static_cast<uint32_t*>(surf_->pixels)[y*surf_->pitch + x] = clr;
	}


private:
	SDL_Surface* surf_;
};

class ThinSDLSurface : public virtual IBlitableSurface {
public:
	ThinSDLSurface(SDL_Surface * surf, bool free_pixels) :
		surf_(surf), pa_(surf), free_pixels_(free_pixels)
	{}
	virtual ~ThinSDLSurface() {
		if (surf_) {
			// TODO(sirver): Leaking left and right
			// SDL_FreeSurface(surf_);
			if (free_pixels_) {
				free(surf_->pixels);
			}
		}
		surf_ = 0;
	}

	virtual bool valid() const {return true;}
	virtual uint32_t get_w() const {return surf_->w;}
	virtual uint32_t get_h() const {return surf_->h;}

	virtual IPixelAccess & pixelaccess() {return pa_;}

	void blit(const Point& dst, const IPicture* src, const Rect& srcrc, Composite cm)
	{
		upcast(const ThinSDLSurface, sdlsurf, src);
		assert(sdlsurf);
		assert(this);
		SDL_Rect srcrect = {srcrc.x, srcrc.y, srcrc.w, srcrc.h};
		SDL_Rect dstrect = {dst.x, dst.y, 0, 0};

		bool alpha;
		uint8_t alphaval;
		if (cm == CM_Solid || cm == CM_Copy) {
			alpha = sdlsurf->surf_->flags & SDL_SRCALPHA;
			alphaval = sdlsurf->surf_->format->alpha;
			SDL_SetAlpha(sdlsurf->surf_, 0, 0);
		}

		SDL_BlitSurface(sdlsurf->surf_, &srcrect, surf_, &dstrect);

		if (cm == CM_Solid || cm == CM_Copy) {
			SDL_SetAlpha(sdlsurf->surf_, alpha?SDL_SRCALPHA:0, alphaval);
		}
	}

	void fill_rect(const Rect& rc, RGBAColor clr) {
		const uint32_t color = clr.map(*surf_->format);
		SDL_Rect r = {rc.x, rc.y, rc.w, rc.h};
		SDL_FillRect(surf_, &r, color);
	}

private:
	SDL_Surface * surf_;
	ThinSDLSurfacePA pa_;
	bool free_pixels_;
};


// TODO(sirver): Rename to TinyGraphics
class ThinGraphic : boost::noncopyable, virtual public IGraphic {
public:
	virtual ~ThinGraphic();
	virtual IPicture* convert_sdl_surface_to_picture(SDL_Surface *, bool alpha = false);
	virtual IPicture* load_image(const std::string &, bool alpha = false);
	virtual const IPicture* get_picture(PicMod, std::string const &, bool alpha = true);
	virtual void add_picture_to_cache(PicMod, const std::string &, IPicture*);
	IBlitableSurface * create_surface(int32_t w, int32_t h, bool alpha = false);

private:
	typedef std::pair<const string, IPicture*> MapEntry;
	typedef map<string, IPicture*> GraphicMap;
	GraphicMap m_imgcache;
};


ThinGraphic::~ThinGraphic() {
	// TODO(sirver): delete the images we own
}

IPicture* ThinGraphic::convert_sdl_surface_to_picture(SDL_Surface * surf, bool alpha)
{
	IPicture* rv(new ThinSDLSurface(surf, false));
	// TODO(sirver): We are leaking left and right here
	// SDL_FreeSurface(surf);
	return rv;
}

// TODO(sirver): Pass a PicMod and add to cache immediately
// TODO: understand the semantic of these functions. Is load_image
// caching already. Is get_picture calling load image?
IPicture* ThinGraphic::load_image(const std::string & s, bool alpha) {
	unsigned w, h;
	unsigned char * image;

	unsigned error = lodepng_decode32_file(&image, &w, &h,
			("imgs/" + s).c_str());
	if (error)
		throw RT::BadImage
			((format("Problem loading image %s: %s\n") % s % lodepng_error_text(error)).str());

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

	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w*4, rmask, gmask, bmask, amask);
	if (!surf)
		throw RT::BadImage
			((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

	ThinSDLSurface* rv = new ThinSDLSurface(surf, true);
	return m_imgcache[s] = rv;
}

// TODO(sirver): Why is this exposed?
void ThinGraphic::add_picture_to_cache(PicMod /* module */, const std::string & name, IPicture* pic) {
	// We ignore module completely here.
	m_imgcache[name] = pic;
}

const IPicture* ThinGraphic::get_picture
	(PicMod const module, const std::string & fname, bool alpha)
{
	GraphicMap::iterator i = m_imgcache.find(fname);
	if (i != m_imgcache.end())
		return i->second;

	load_image(fname, alpha);
	return m_imgcache[fname];
}

IBlitableSurface * ThinGraphic::create_surface(int32_t w, int32_t h, bool alpha) {
	return new ThinSDLSurface(RT::empty_sdl_surface(w,h,alpha), false);
}


IGraphic * create_thin_graphic() {
	return new ThinGraphic();
}

