/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

// NOCOM(#sirver): check for ME also in conf files and therelike.

#include "animation.h"
#include "surface.h"
#include "graphic.h"
#include <cassert>

#include <SDL.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "wexception.h"

#include "animation.h"
#include "image.h"
#include "image_cache.h"
#include "image_transformations.h"

#include "diranimations.h"
#include "logic/bob.h"
#include "constants.h"
#include "i18n.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "wexception.h"

#include "helper.h"
// NOCOM(#sirver): check and order includes

#include <cstdio>

using namespace std;


namespace  {

// NOCOM(#sirver): this should not be here
// // NOCOM(#sirver): documentation
class NumberGlob : boost::noncopyable {
public:
   typedef uint32_t type;
	NumberGlob(const string& pictmp) : templ_(pictmp), cur_(0) {
		int nchars = std::count(pictmp.begin(), pictmp.end(), '?');
		fmtstr_ = "%0" + boost::lexical_cast<string>(nchars) + "i";

		max_ = 1;
		for (int i = 0; i < nchars; ++i) {
			max_ *= 10;
			replstr_ += "?";
		}
		max_ -= 1;
	}

	bool next(string* s) {
		if (cur_ > max_)
			return false;

		if (max_) {
			*s = boost::replace_last_copy(templ_, replstr_, (boost::format(fmtstr_) % cur_).str());
		} else {
			*s = templ_;
		}
		++cur_;
		return true;
	}

private:
	string templ_;
	string fmtstr_;
	string replstr_;
	uint32_t cur_;
	uint32_t max_;
};

class AnimationImpl : public Animation {
public:
	virtual ~AnimationImpl() {}
	AnimationImpl(const string& directory, Section & s);

	// Implements Animation.
	virtual uint16_t width() const {return frames_[0]->width();}
	virtual uint16_t height() const {return frames_[0]->height();}
	virtual uint16_t nr_frames() const {return frames_.size();}
	virtual uint32_t frametime() const {return frametime_;}
	virtual const Point& hotspot() const {return hotspot_;};
	virtual const Image& representative_image(const RGBColor& clr) const {return get_frame(0, clr);}
	void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const;
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const;

private:
	const Image& get_frame(uint32_t time, const RGBColor& playercolor) const;
	const Image& get_frame(uint32_t time) const;

	uint32_t frametime_;
	Point hotspot_;
	bool hasplrclrs_;

	vector<const Image*> frames_;
	vector<const Image*> pcmasks_;

	/// mapping of soundeffect name to frame number, indexed by frame number .
	map<uint32_t, string> sfx_cues;
};

AnimationImpl::AnimationImpl
	(const string& directory,
	 Section& s)
	: frametime_(FRAME_LENGTH) {

	// Read mapping from frame numbers to sound effect names and load effects
	while (Section::Value * const v = s.get_next_val("sfx")) {
		char * parameters = v->get_string(), * endp;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		uint32_t const frame_number = value;
		try {
			if (endp == parameters or frame_number != value)
				throw wexception
					(_("expected %s but found \"%s\""),
					 _("frame number"), parameters);
			parameters = endp;
			force_skip(parameters);
			g_sound_handler.load_fx(directory, parameters);
			map<uint32_t, string>::const_iterator const it =
				sfx_cues.find(frame_number);
			if (it != sfx_cues.end())
				throw wexception
					("redefinition for frame %u to \"%s\" (previously defined to "
					 "\"%s\")",
					 frame_number, parameters, it->second.c_str());
		} catch (const _wexception & e) {
			throw wexception("sfx: %s", e.what());
		}
		sfx_cues[frame_number] = parameters;
	}

	hasplrclrs_ = s.get_bool("playercolor", false);

	int32_t const fps = s.get_int("fps");
	if (fps < 0)
		throw wexception("fps is %i, must be non-negative", fps);
	if (fps > 0)
		frametime_ = 1000 / fps;

	hotspot_ = s.get_Point("hotspot");

	// NOCOM(#sirver): check text
	//  In the filename template, the last sequence of '?' characters (if any)
	//  is replaced with a number, for example the template "idle_??" is
	//  replaced with "idle_00". Then the code looks if there is a file with
	//  that name + any extension in the list above. Assuming that it finds a
	//  file, it increments the number so that the filename is "idle_01" and
	//  looks for a file with that name + extension, and so on until it can not
	//  find any file. Then it is assumed that there are no more frames in the
	//  animation.

	//  Allocate a buffer for the filename. It must be large enough to hold the
	//  image name template, the "_pc" (3 characters) part for playercolor
	//  masks, the extension (4 characters including the dot) and the null
	//  terminator. Copy the image name template into the buffer.
	// NOCOM(#sirver): this should go easier.
	string picnametempl;
	if (char const * const pics = s.get_string("pics")) {
		picnametempl = directory + pics;
	} else {
		picnametempl = directory + s.get_name();
	}
	// Strip the .png extension if it has one.
	boost::replace_all(picnametempl, ".png", "");

	NumberGlob glob(picnametempl);
	string filename_wo_ext;
	while(glob.next(&filename_wo_ext)) {
		string filename = filename_wo_ext + ".png";
		log("#sirver filename: %s\n", filename.c_str());
		// Load the base image
		// strcpy(after_basename, extensions[extnr]);
		if (!g_fs->FileExists(filename))
			break;

		try {
			const Image* image = g_gr->images().get(filename);
			if (frames_.size() && (frames_[0]->width() != image->width() or frames_[0]->height() != image->height()))
				throw wexception
					("wrong size: (%u, %u), should be (%u, %u) like the "
					 "first frame",
					 image->width(), image->height(), frames_[0]->width(), frames_[0]->height());
			//  Get a new AnimFrame.
			frames_.push_back(image);
		} catch (const exception & e) {
			throw wexception
				("could not load animation frame %s: %s\n",
				 filename.c_str(), e.what());
		}

		if (hasplrclrs_) {
			//TODO Do not load playercolor mask as opengl texture or use it as
			//     opengl texture.
			string pc_filename = filename_wo_ext + "_pc.png";

			if (g_fs->FileExists(pc_filename)) {
				try {
					const Image* image = g_gr->images().get(pc_filename);
					if (frames_[0]->width() != image->width() or frames_[0]->height() != image->height())
						throw wexception
							("playercolor mask has wrong size: (%u, %u), should "
							 "be (%u, %u) like the animation frame",
							 image->width(), image->height(), frames_[0]->width(), frames_[0]->height());
					pcmasks_.push_back(image);
				} catch (const exception & e) {
					throw wexception
						("error while reading \"%s\": %s", pc_filename.c_str(), e.what());
				}
			}
		}
	}

	if (frames_.empty())
		throw wexception
			("animation %s has no frames", picnametempl.c_str());

	if (pcmasks_.size() and pcmasks_.size() < frames_.size())
		throw wexception
			("animation has %"PRIuS" frames but playercolor mask has only %"PRIuS" frames",
			 frames_.size(), pcmasks_.size());
}

/// Find out if there is a sound effect registered for the animation's frame
/// and try to play it. This is used to have sound effects that are tightly
/// synchronized to an animation, for example when a geologist is shown
/// hammering on rocks.
///
/// \par time  The time currently on display.
void AnimationImpl::trigger_soundfx
	(uint32_t time, uint32_t stereo_position) const {
	uint32_t const framenumber = time / frametime_ % nr_frames();
	const map<uint32_t, string>::const_iterator sfx_cue = sfx_cues.find(framenumber);
	if (sfx_cue != sfx_cues.end())
		g_sound_handler.play_fx(sfx_cue->second, stereo_position, 1);
}

void AnimationImpl::blit(uint32_t time, const Point& dst, const Rect& srcrc, const RGBColor* clr, Surface* target) const {
	assert(target);

	const Image& frame = clr ? get_frame(time, *clr) : get_frame(time);
	target->blit(dst, frame.surface(), srcrc);
}

const Image& AnimationImpl::get_frame(uint32_t time, const RGBColor& playercolor) const {
	const Image& original = get_frame(time);
	if (!hasplrclrs_)
		return original;

	assert(frames_.size() == pcmasks_.size());
	const uint32_t framenumber = time / frametime_ % nr_frames();
	return *ImageTransformations::player_colored(playercolor, &original, pcmasks_[framenumber]);
}

const Image& AnimationImpl::get_frame(uint32_t time) const {
	const uint32_t framenumber = time / frametime_ % nr_frames();
	assert(framenumber < nr_frames());
	return *frames_[framenumber];
}


}  // namespace


/*
==============================================================================

AnimationManager IMPLEMENTATION

==============================================================================
*/

/**
 * Loads an animation, graphics sound and everything.
 *
 * The animation resides in the given directory and is described by the given
 * section.
 *
 * This function looks for image files in this order:
 *    key 'pics', if present
 *    picnametempl, if not null
 *    \<sectionname\>_??.png
 *
 * \param directory     which directory to look in for image and sound files
 * \param s             conffile section to search for data on this animation
 * \param picnametempl  a template for the image names
*/
uint32_t AnimationManager::load(const string& directory, Section & s)
	 // char       const *       picnametempl) // NOCOM(#sirver): what
{
	m_animations.push_back(new AnimationImpl(directory, s));
	uint32_t const id = m_animations.size();

	return id;
}


/*
===============
Return AnimationData for this animation or throws if this id is unknown.
===============
*/
const Animation& AnimationManager::get_animation(uint32_t id) const
{
	if (!id || id > m_animations.size())
		throw wexception("Requested unknown animation with id: %i", id);

	return *m_animations[id - 1];
}


/*
==============================================================================

DirAnimations IMPLEMENTAION

==============================================================================
*/

DirAnimations::DirAnimations
	(uint32_t const dir1,
	 uint32_t const dir2,
	 uint32_t const dir3,
	 uint32_t const dir4,
	 uint32_t const dir5,
	 uint32_t const dir6)
{
	m_animations[0] = dir1;
	m_animations[1] = dir2;
	m_animations[2] = dir3;
	m_animations[3] = dir4;
	m_animations[4] = dir5;
	m_animations[5] = dir6;
}


/*
===============
Parse an animation from the given directory and config.
sectnametempl is of the form "foowalk_??", where ?? will be replaced with
nw, ne, e, se, sw and w to get the section names for the animations.

If defaults is not zero, the additional sections are not actually necessary.
If they don't exist, the data is taken from defaults and the bitmaps
foowalk_??_nn.png are used.
===============
*/
void DirAnimations::parse
	(Widelands::Map_Object_Descr & b,
	 const string & directory,
	 Profile & prof,
	 char const * const sectnametempl,
	 Section * const defaults)
{
	char dirpictempl[256];
	char sectnamebase[256];
	char * repl;

	if (strchr(sectnametempl, '%'))
		throw wexception("sectnametempl %s contains %%", sectnametempl);

	snprintf(sectnamebase, sizeof(sectnamebase), "%s", sectnametempl);
	repl = strstr(sectnamebase, "??");
	if (!repl)
		throw wexception
			("DirAnimations section name template %s does not contain %%s",
			 sectnametempl);

	strncpy(repl, "%s", 2);

	if
		(char const * const string =
		 defaults ? defaults->get_string("dirpics", 0) : 0)
	{
		snprintf(dirpictempl, sizeof(dirpictempl), "%s", string);
		repl = strstr(dirpictempl, "!!");
		if (!repl)
			throw wexception
				("DirAnimations dirpics name templates %s does not contain !!",
				 dirpictempl);

		strncpy(repl, "%s", 2);
	} else {
		snprintf(dirpictempl, sizeof(dirpictempl), "%s_??", sectnamebase);
	}

	for (int32_t dir = 0; dir < 6; ++dir) {
		static char const * const dirstrings[6] =
			{"ne", "e", "se", "sw", "w", "nw"};
		char sectname[300];

		snprintf(sectname, sizeof(sectname), sectnamebase, dirstrings[dir]);

		string const anim_name = sectname;

		Section * s = prof.get_section(sectname);
		if (!s) {
			if (!defaults)
				throw wexception
					("Section [%s] missing and no default supplied",
					 sectname);

			s = defaults;
		}

		snprintf(sectname, sizeof(sectname), dirpictempl, dirstrings[dir]);
		s->set_name(sectname); // NOCOM(#sirver): comment
		m_animations[dir] = g_gr->animations().load(directory, *s);
		b.add_animation(anim_name.c_str(), m_animations[dir]);
	}
}
