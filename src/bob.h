/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#ifndef __BOB_H
#define __BOB_H

// #include "worldfiletypes.h"
#include "graphic.h"
#include "pic.h"
#include "myfile.h"
#include "instances.h"

// class World;
//class Pic;

class Animation;

struct Animation_Pic {
   ushort *data;
   Animation* parent;
};

class Animation {
   public:
      enum {
         HAS_TRANSPARENCY = 1,
         HAS_SHADOW = 2,
         HAS_PL_CLR = 3
      };

      Animation(void) { npics=0; pics=0;}
      ~Animation(void) { 
         if(npics) {
            uint i; 
            for(i=0; i<npics; i++) {
               free(pics[i].data);
            }
            free(pics);
         }
      }
   
      inline ushort get_w(void) { return w; }
      inline ushort get_h(void) { return h; }
      inline ushort get_hsx(void) { return hsx; }
      inline ushort get_hsy(void) { return hsy; }
      void add_pic(ushort size, ushort* data) {
         if(!npics) {
            npics=1;
            pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
         } else {
            npics++;
            pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic*)*npics);
         }
         pics[npics-1].data=(ushort*)malloc(size);
         pics[npics-1].parent=this;
         memcpy(pics[npics-1].data, data, size);
      }

      void set_flags(uint mflags) { flags=mflags; }
      void set_dimensions(ushort mw, ushort mh) { w=mw; h=mh; }
      void set_hotspot(ushort x, ushort y) { hsx=x; hsy=y; }

      int read(Binary_file*);

      // TEMP 
      inline Animation_Pic* get_pic(ushort) { return &pics[0]; }

      // TEMP ENDS
   private:
      uint flags;
      ushort w, h;
      ushort hsx, hsy;
      ushort npics;
      Animation_Pic *pics;
};

//
// This class describes a in-game Boring bob
//
class Boring_Bob : public Map_Object {
   public:
      Boring_Bob(ushort n) : Map_Object(n) { } 
      virtual ~Boring_Bob(void) { }

      int act(Game* g);

   private:
};

//
// This class describes a in-game Diminishing bob
//
class Diminishing_Bob : public Map_Object {
   public:
      Diminishing_Bob(ushort n) : Map_Object(n) { } 
      virtual ~Diminishing_Bob(void) { }

      int act(Game* g);

   private:
};

class Logic_Bob_Descr {
   public:
      enum {
         BOB_GROWING=0,
         BOB_DIMINISHING,
         BOB_BORING,
         BOB_CRITTER
      };

      Logic_Bob_Descr(void) { }
      virtual ~Logic_Bob_Descr(void) { }

      virtual int read(Binary_file*);
      virtual int create_instance(Instance*)=0;
      const char* get_name(void) { return name; }

      inline Animation* get_anim(void) { return &anim; }

   protected:
      char name[30];
      Animation anim;
};

//
// Growings
// 
class Growing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Growing_Bob_Descr(void) { ends_in=0; growing_speed=0; }
      virtual ~Growing_Bob_Descr(void) {  }

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      Logic_Bob_Descr* ends_in;
      ushort growing_speed;
};

// 
// Diminishing
// 
class Diminishing_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Diminishing_Bob_Descr(void) { ends_in=0; stock=0; }
      virtual ~Diminishing_Bob_Descr(void) { }

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      Logic_Bob_Descr* ends_in;
      ushort stock;
};

// 
// Borings
// 
class Boring_Bob_Descr : virtual public Logic_Bob_Descr {
   public:
      Boring_Bob_Descr(void) { ttl=0; }
      virtual ~Boring_Bob_Descr(void) { } 

      virtual int read(Binary_file* f);
      int create_instance(Instance*);

   private:
      ushort ttl; // time to life
};

#endif

