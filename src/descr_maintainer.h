/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__DESCR_MAINTAINER_H
#define __S__DESCR_MAINTAINER_H

/**
 * This template is used to have a typesafe maintaining class for Bob_Descr,
 * Worker_Descr and so on.
 */
template <class T> class Descr_Maintainer {
   public:
      Descr_Maintainer(void) { nitems=0; items=0; place_for=0; }
      ~Descr_Maintainer(void) ;

	static typename T::Index invalid_index()
	{return std::numeric_limits<typename T::Index>::max();}

      T* exists(const char* name);
      int add(T* item);
      ushort get_nitems(void) const { return nitems; }
      int get_index(const char * const name) const; // can return -1
      void reserve(uint n) {
		items = static_cast<T * * const>(realloc(items, sizeof(T *) * n));
         place_for=n;
      }

      T * get(const int idx) const {
         if (idx >= 0 and idx < static_cast<int>(nitems)) return items[idx];
         else return 0;
      }

   private:
      uint place_for;
      uint nitems;
      T** items;
};

template <class T>
int Descr_Maintainer<T>::get_index(const char * const name) const {

   ushort i;
   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return i;
   }

   return -1;
}

template <class T>
int Descr_Maintainer<T>::add(T* item) {
   nitems++;
   if(nitems>=place_for) {
      reserve(nitems);
   }
   items[nitems-1]=item;
	return nitems-1;
}

//
//returns elemt if it exists, 0 if it doesnt
//
template <class T>
T* Descr_Maintainer<T>::exists(const char* name) {
   uint i;

   for(i=0; i<nitems; i++) {
      if(!strcasecmp(name, items[i]->get_name())) return items[i];
   }
	return 0;
}

template <class T>
Descr_Maintainer<T>::~Descr_Maintainer(void) {
   uint i;
   for(i=0; i<nitems; i++) {
      delete items[i];
   }
   free(items);
}

#endif
