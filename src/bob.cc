/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#include "widelands.h"
#include "world.h"
#include "game.h"
#include "cmd_queue.h"
#include "player.h"
#include "bob.h"
#include "map.h"
#include "profile.h"


/*
==============================================================================

Bob IMPLEMENTATION		

==============================================================================
*/

/*
===============
Bob_Descr::Bob_Descr
Bob_Descr::~Bob_Descr
===============
*/
Bob_Descr::Bob_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
}

Bob_Descr::~Bob_Descr(void)
{
}

/*
===============
Bob_Descr::parse

Parse additional information from the config file
===============
*/
void Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
	m_idle_anim = g_anim.get(directory, prof->get_safe_section("idle"), picname, encdata);
}


/*
===============
Bob_Descr::create

Create a bob of this type
===============
*/
Bob *Bob_Descr::create(Editor_Game_Base *gg, Player *owner, Coords coords, bool logic)
{
   Game* g=static_cast<Game*>(gg);
   Bob *bob = create_object(logic);
   bob->set_owner(owner);
   bob->set_position(g, coords);
   bob->init(g);

   return bob;
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Bob::Bob

Zero-initialize a map object
===============
*/
Bob::Bob(Bob_Descr* descr, bool wl)
	: Map_Object(descr, wl)
{
	m_owner = 0;
	m_position.x = m_position.y = 0; // not linked anywhere
	m_position.field = 0;
	m_linknext = 0;
	m_linkpprev = 0;

	m_actid = 0; // this isn't strictly necessary, but it shuts up valgrind and "feels" cleaner

	m_anim = 0;
	m_animstart = 0;

	m_walking = IDLE;
	m_walkstart = m_walkend = 0;

	m_task = 0;
	m_task_acting = false;
	m_task_switching = false;
}

/*
===============
Bob::~Bob()

Cleanup an object. Removes map links
===============
*/
Bob::~Bob()
{
	if (m_position.field) {
		molog("Map_Object::~Map_Object: m_pos.field != 0, cleanup() not called!\n");
		*(int *)0 = 0;
	}
}

/*
===============
Bob::get_type
===============
*/
int Bob::get_type()
{
	return BOB;
}


/*
Objects and tasks
-----------------

Every object _always_ has a current task which it is doing.
For a boring object, this task is always an IDLE task, which will be
reduced to effectively 0 CPU overhead.

For another simple example, look at animals. They have got two states:
moving or not moving. This is actually represented as two tasks,
IDLE and MOVE_PATH, which are both part of the default package that comes
with Map_Object.

Now there are some important considerations:
- every object must always have a task, even if it's IDLE
- be careful as to when you call task handling functions; the comments
  above each function tell you when you can call them, and which functions
  you can call from them
- a task can only end itself; it cannot be ended by someone else
- there are default, predefined tasks (TASK_IDLE, TASK_MOVEPATH); use them
- you must call start_task_*() for the default tasks. Do not start them
  directly!

To implement a new task, you need to create a new task_begin(), task_act()
and perhaps task_end(). Create a switch()-statement for the new task(s) and
call the base class' task_*() functions in the default branch.
Most likely, you'll also want a start_task_*()-type function.
*/

/*
===============
Bob::init

Make sure you call this from derived classes!

Initialize the object
===============
*/
void Bob::init(Editor_Game_Base* gg)
{
   Map_Object::init(gg);

   if(get_logic()) {
      Game* g=static_cast<Game*>(gg);
      // Initialize task system
      m_lasttask = 0;
      m_lasttask_success = true;
      m_nexttask = 0;

      do_next_task(g);
   }
}

/*
===============
Bob::cleanup

Perform independant cleanup as necessary.
===============
*/
void Bob::cleanup(Editor_Game_Base *gg)
{
   if(get_logic()) {
      Game* g=static_cast<Game*>(gg);
      if (get_current_task())
         task_end(g); // subtle...
   }

   if (m_position.field) {
      m_position.field = 0;
      *m_linkpprev = m_linknext;
      if (m_linknext)
         m_linknext->m_linkpprev = m_linkpprev;

   }
   Map_Object::cleanup(gg);
}

/*
===============
Bob::act

Called by Cmd_Queue when a CMD_ACT event is triggered.
Hand the acting over to the task.

Change to the next task if necessary.
===============
*/
void Bob::act(Game* g, uint data)
{
	// Eliminate spurious calls of act().
	// These calls are to be expected and perfectly normal, e.g. when a carrier's
	// idle task is interrupted by the request to pick up a ware from a flag.
	if (data != m_actid)
		return;

	// Call the appropriate act function.
	m_task_acting = true;
	int tdelta = task_act(g, m_task_interrupt);
	// a tdelta == 0 is probably NOT what you want if the task continues - make your intentions clear
	assert(!m_task || tdelta < 0 || tdelta > 0);
	m_task_acting = false;

	if (!m_task) {
		do_next_task(g);
		return;
	}

	if (tdelta > 0)
		g->get_cmdqueue()->queue(g->get_gametime()+tdelta, SENDER_MAPOBJECT, CMD_ACT, m_serial,
		                         ++m_actid, 0);
}

/*
===============
Bob::do_next_task [private]

Try to get the next task running.
===============
*/
void Bob::do_next_task(Game* g)
{
	int task_retries = 0;

	assert(!m_task);

	while(!m_task) {
		assert(task_retries < 5); // detect infinite loops early

		m_task_switching = true;
		task_start_best(g, m_lasttask, m_lasttask_success, m_nexttask);
		m_task_switching = false;

		do_start_task(g);

		task_retries++;
	}
}

/*
===============
Bob::start_task

Start the given task.

Only allowed when m_task_switching, i.e. from init() and act().
Consequently, derived classes can only call this from task_start_best().
===============
*/
void Bob::start_task(Game* g, uint task)
{
	assert(m_task_switching);
	assert(!m_task);

	m_task = task;
}

/*
===============
Bob::do_start_task [private]

Actually start the task (m_task is set already)
===============
*/
void Bob::do_start_task(Game* g)
{
	assert(m_task);

	m_task_interrupt = false;

	m_task_acting = true;
	int tdelta = task_begin(g);
	// a tdelta == 0 is probably NOT what you want - make your intentions clear
	assert(!m_task || tdelta < 0 || tdelta > 0);
	m_task_acting = false;

	if (m_task && tdelta > 0)
		g->get_cmdqueue()->queue(g->get_gametime()+tdelta, SENDER_MAPOBJECT, CMD_ACT, m_serial,
		                         ++m_actid, 0);
}

/*
===============
Bob::end_task

Let the task end itself, indicating success or failure.
nexttask will be passed to task_start_best() to help the decision.

Only allowed when m_task_acting, i.e. from act() or start_task()
and thus only from task_begin() and task_act()

Be aware that end_task() calls task_end() which may cleanup some
structures belonging to your task.
===============
*/
void Bob::end_task(Game* g, bool success, uint nexttask)
{
	assert(m_task_acting);
	assert(m_task);

	task_end(g);

	m_lasttask = m_task;
	m_lasttask_success = success;
	m_nexttask = nexttask;

	m_task = 0;
}

/*
===============
Bob::interrupt_task

Ask the current task to end (with failure) as soon as possible, i.e. within
the next second or so.
If hard is true, the task is stopped immediately.

If hard is false, task_interrupt() is called. This function returns true if it
is okay to interrupt the task immediately. Otherwise, the task will be
interrupted on the next task_act().
===============
*/
void Bob::interrupt_task(Game *g, bool hard, uint nexthint)
{
	assert(!m_task_acting);
	assert(!m_task_switching);
	assert(m_task);

	if (!hard) {
		bool stop;

		m_task_acting = true;
		stop = task_interrupt(g);
		m_task_acting = false;

		if (!stop) {
			// cause an interrupt on the next CMD_ACT
			m_task_interrupt = true;
			return;
		}
	}


	// Terminate the current task now.
	task_end(g);

	m_lasttask = m_task;
	m_lasttask_success = false;
	m_nexttask = nexthint;

	m_task = 0;

	++m_actid; // be very sure to eliminate spurious CMD_ACTs

	// Start the next task
	do_next_task(g);
}


/*
===============
Bob::start_task_idle

Start an idle phase, using the given animation
If the timeout is a positive value, the idle phase stops after the given
time.

This task always succeeds unless interrupted.
===============
*/
void Bob::start_task_idle(Game* g, uint anim, int timeout)
{
	// timeout == 0 will wait indefinitely - probably NOT what you want (use -1 for infinite)
	assert(timeout < 0 || timeout > 0);

	set_animation(g, anim);
	task.idle.timeout = timeout;
	start_task(g, TASK_IDLE);
}

/*
===============
Bob::start_task_movepath

Start moving to the given destination. persist is the same parameter as
for Map::findpath().

Returns false if no path could be found.

The task finishes once the goal has been reached. It may fail.
===============
*/
bool Bob::start_task_movepath(Game* g, Coords dest, int persist, DirAnimations *anims)
{
	task.movepath.path = new Path;

	if (g->get_map()->findpath(m_position, dest, get_movecaps(), persist, task.movepath.path) < 0) {
		delete task.movepath.path;
		return false;
	}

	task.movepath.step = 0;
	task.movepath.anims = anims;

	start_task(g, TASK_MOVEPATH);
	return true;
}

/*
===============
Bob::start_task_movepath

Start moving along the given, precalculated path.
===============
*/
void Bob::start_task_movepath(Game* g, const Path &path, DirAnimations *anims)
{
	assert(path.get_start() == get_position());

	task.movepath.path = new Path(path);
	task.movepath.step = 0;
	task.movepath.anims = anims;

	start_task(g, TASK_MOVEPATH);
}

/*
===============
Bob::start_task_forcemove

Move into the given direction, without passability checks.
===============
*/
void Bob::start_task_forcemove(Game *g, int dir, DirAnimations *anims)
{
	task.forcemove.dir = dir;
	task.forcemove.anims = anims;

	start_task(g, TASK_FORCEMOVE);
}

/*
===============
Bob::task_begin [virtual]

This function is called to start a task.

In this function, you may:
 - call end_task()
 - call set_animation(), start_walk(), set_position() and similar functions
 - call task_act() for "array-based" tasks

You can schedule a call to task_act() by returning the time, in milliseconds,
until task_act() should be could. NOTE: this time is relative to the current
time!
If you return a value <= 0, task_act() will _never_ be called. This means that
the task won't end itself.
===============
*/
int Bob::task_begin(Game* g)
{
	switch(get_current_task()) {
	case TASK_IDLE:
		return task.idle.timeout;

	case TASK_MOVEPATH:
		return task_act(g, false);

	case TASK_FORCEMOVE:
	{
		int dir = task.forcemove.dir;
		int tdelta = start_walk(g, (WalkingDir)dir,
		                        task.forcemove.anims->get_animation(dir), true);
		assert(tdelta > 0);
		return tdelta;
	}
	}

	throw wexception("Bob::task_begin: unhandled task");
}

/*
===============
Bob::task_act [virtual]

Calls to this function are scheduled by this function and task_begin().

In this function you may call all the functions available in task_begin().
If interrupt is true, you should call end_task() indicating failure.

As with task_begin(), you can also schedule another call to task_act() by
returning a value > 0
===============
*/
int Bob::task_act(Game* g, bool interrupt)
{
	switch(get_current_task()) {
	case TASK_IDLE:
		end_task(g, true, 0); // success, no next task
		return 0; /* will be ignored */

	case TASK_MOVEPATH:
	{
		if (task.movepath.step)
			end_walk(g);

		if (interrupt) {
			molog("TASK_MOVEPATH: interrupted\n");
			end_task(g, false, 0); // failure
			return 0; // will be ignored
		}

		if (task.movepath.step >= task.movepath.path->get_nsteps()) {
			assert(m_position == task.movepath.path->get_end());
			end_task(g, true, 0); // success
			return 0;
		}

		char dir = task.movepath.path->get_step(task.movepath.step);

		int tdelta = start_walk(g, (WalkingDir)dir, task.movepath.anims->get_animation(dir));
		if (tdelta < 0) {
			molog("TASK_MOVEPATH: can't walk\n");
			end_task(g, false, 0); // failure to reach goal
			return 0;
		}

		task.movepath.step++;
		return tdelta;
	}

	case TASK_FORCEMOVE:
		end_walk(g);
		end_task(g, true, 0);
		return 0;
	}

	throw wexception("task_act: Unhandled task %i", m_task);
}


/*
===============
Bob::task_interrupt [virtual]

This is called when someone wants to interrupt a task nicely.
Return true if you want to end the task right now.
Return false if you want to continue until the next task_act().
===============
*/
bool Bob::task_interrupt(Game*)
{
	switch(get_current_task()) {
	case TASK_MOVEPATH:
	case TASK_FORCEMOVE:
		return false;

	default:
		return true;
	}
}


/*
===============
Bob::task_end [virtual]

Called by end_task(). Use it to clean up any structures allocated in
task_begin() or a start_task_*() type function.
===============
*/
void Bob::task_end(Game*)
{
	switch(get_current_task()) {
	case TASK_MOVEPATH:
		if (task.movepath.path)
			delete task.movepath.path;
		break;
	}
}


/*
===============
Bob::calc_drawpos

Calculates the actual position to draw on from the base field position.
This function takes walking etc. into account.
===============
*/
void Bob::calc_drawpos(Editor_Game_Base* game, Point pos, Point* drawpos)
{
	Map *map = game->get_map();
	FCoords end;
	FCoords start;
	Point spos;
	Point epos;

	end = m_position;
	epos = pos;
	spos = epos;

	switch(m_walking) {
	case WALK_NW: map->get_brn(end, &start); spos.x += FIELD_WIDTH/2; spos.y += FIELD_HEIGHT/2; break;
	case WALK_NE: map->get_bln(end, &start); spos.x -= FIELD_WIDTH/2; spos.y += FIELD_HEIGHT/2; break;
	case WALK_W: map->get_rn(end, &start); spos.x += FIELD_WIDTH; break;
	case WALK_E: map->get_ln(end, &start); spos.x -= FIELD_WIDTH; break;
	case WALK_SW: map->get_trn(end, &start); spos.x += FIELD_WIDTH/2; spos.y -= FIELD_HEIGHT/2; break;
	case WALK_SE: map->get_tln(end, &start); spos.x -= FIELD_WIDTH/2; spos.y -= FIELD_HEIGHT/2; break;

	case IDLE: start.field = 0; break;
	}

	if (start.field) {
		spos.y += MULTIPLY_WITH_HEIGHT_FACTOR(end.field->get_height());
		spos.y -= MULTIPLY_WITH_HEIGHT_FACTOR(start.field->get_height());

		float f = (float)(game->get_gametime() - m_walkstart) / (m_walkend - m_walkstart);
		if (f < 0) f = 0;
		else if (f > 1) f = 1;

		epos.x = (int)(f*epos.x + (1-f)*spos.x);
		epos.y = (int)(f*epos.y + (1-f)*spos.y);
	}

	*drawpos = epos;
}


/*
===============
Bob::draw

Draw the map object.
posx/posy is the on-bitmap position of the field we're currently on.

It LERPs between start and end position when we're walking.
Note that the current field is actually the field we're walking to, not
the one we start from.
===============
*/
void Bob::draw(Editor_Game_Base *game, RenderTarget* dst, Point pos)
{
	if (!m_anim)
		return;

	const RGBColor* playercolors = 0;
	Point drawpos;

	calc_drawpos(game, pos, &drawpos);

	if (get_owner())
		playercolors = get_owner()->get_playercolor();

	dst->drawanim(drawpos.x, drawpos.y, m_anim, game->get_gametime() - m_animstart, playercolors);
}


/*
===============
Bob::set_animation

Set a looping animation, starting now.
===============
*/
void Bob::set_animation(Game* g, uint anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/*
===============
Bob::is_walking

Return true if we're currently walking
===============
*/
bool Bob::is_walking()
{
	return m_walking != IDLE;
}

/*
===============
Bob::end_walk

Call this from your task_act() function that was scheduled after start_walk().
===============
*/
void Bob::end_walk(Game* g)
{
	m_walking = IDLE;
}


/*
===============
Bob::start_walk

Cause the object to walk, honoring passable/impassable parts of the map using movecaps.
If force is true, the passability check is skipped.

Returns the number of milliseconds after which the walk has ended. You must
call end_walk() after this time, so schedule a task_act().

Returns a negative value when we can't walk into the requested direction.
===============
*/
int Bob::start_walk(Game *g, WalkingDir dir, uint a, bool force)
{
	FCoords newf;

	g->get_map()->get_neighbour(m_position, dir, &newf);

	// Move capability check by ANDing with the field caps
	//
	// The somewhat crazy check involving MOVECAPS_SWIM should allow swimming objects to
	// temporarily land.
	uint movecaps = get_movecaps();

	if (!force) {
		if (!(m_position.field->get_caps() & movecaps & MOVECAPS_SWIM && newf.field->get_caps() & MOVECAPS_WALK) &&
		    !(newf.field->get_caps() & movecaps))
			return -1;
	}

	// Move is go
	int tdelta = 2000; // :TODO: height-based speed changes

	m_walking = dir;
	m_walkstart = g->get_gametime();
	m_walkend = m_walkstart + tdelta;

	set_position(g, newf);
	set_animation(g, a);

	return tdelta; // yep, we were successful
}

/*
===============
Bob::set_position

Moves the Map_Object to the given position.
===============
*/
void Bob::set_position(Game* g, Coords coords)
{
	if (m_position.field) {
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	m_position = FCoords(coords, g->get_map()->get_field(coords));

	m_linknext = m_position.field->bobs;
	m_linkpprev = &m_position.field->bobs;
	if (m_linknext)
		m_linknext->m_linkpprev = &m_linknext;
	m_position.field->bobs = this;
}


/*
==============================================================================

class Critter_Bob

==============================================================================
*/

//
// Description
//
class Critter_Bob_Descr : public Bob_Descr {
   public:
      Critter_Bob_Descr(const char *name);
      virtual ~Critter_Bob_Descr(void) { }

      virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
      Bob *create_object(bool logic);

      inline bool is_swimming(void) { return m_swimming; }
      inline DirAnimations* get_walk_anims(void) { return &m_walk_anims; }

   private:
		DirAnimations	m_walk_anims;
      bool				m_swimming;
};

Critter_Bob_Descr::Critter_Bob_Descr(const char *name)
	: Bob_Descr(name)
{
	m_swimming = 0;
}

void Critter_Bob_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Bob_Descr::parse(directory, prof, encdata);

	Section *s = prof->get_safe_section("global");
	
	s->get_int("stock", 0);
	m_swimming = s->get_bool("swimming", false);
	
   // Read all walking animations.
	// Default settings are in [walk]
	char sectname[256];
	
	snprintf(sectname, sizeof(sectname), "%s_walk_??", m_name);
	m_walk_anims.parse(directory, prof, sectname, prof->get_section("walk"), encdata);
}


//
// Implementation
//
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

class Critter_Bob : public Bob {
	MO_DESCR(Critter_Bob_Descr);

public:
	Critter_Bob(Critter_Bob_Descr *d, bool);
	virtual ~Critter_Bob(void);

	uint get_movecaps();

	virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d, bool logic)
	: Bob(d, logic)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint Critter_Bob::get_movecaps() { return get_descr()->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }

void Critter_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	if (prev == TASK_IDLE)
	{
		// Pick a target at random
		Coords dst;
		
		dst.x = m_position.x + (g->logic_rand()%5) - 2;
		dst.y = m_position.y + (g->logic_rand()%5) - 2;
		
		if (start_task_movepath(g, dst, 3, get_descr()->get_walk_anims()))
			return;
	
		start_task_idle(g, get_descr()->get_idle_anim(), 1 + g->logic_rand()%1000);
		return;
	}
	
	// idle for a longer period
	start_task_idle(g, get_descr()->get_idle_anim(), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
}

Bob *Critter_Bob_Descr::create_object(bool logic)
{
	return new Critter_Bob(this, logic);
}


/*
==============================================================================

Bob_Descr factory

==============================================================================
*/

/*
===============
Bob_Descr::create_from_dir(const char *directory) [static]
 
Master factory to read a bob from the given directory and create the
appropriate description class.
===============
*/
Bob_Descr *Bob_Descr::create_from_dir(const char *name, const char *directory, Profile *prof)
{
	Bob_Descr *bob = 0;

	try
	{
		Section *s = prof->get_safe_section("global");
		const char *type = s->get_safe_string("type");

		if (!strcasecmp(type, "critter")) {
			bob = new Critter_Bob_Descr(name);
		} else
			throw wexception("Unsupported bob type '%s'", type);

		bob->parse(directory, prof, 0);
	}
	catch(std::exception &e) {
		if (bob)
			delete bob;
		throw wexception("Error reading bob %s: %s", directory, e.what());
	}
	catch(...) {
		if (bob)
			delete bob;
		throw;
	}
	
	return bob;
}
