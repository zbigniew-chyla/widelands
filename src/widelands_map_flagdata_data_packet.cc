/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "widelands_map_flagdata_data_packet.h"

#include "building.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "error.h"

#include <map>


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Flagdata_Data_Packet::~Widelands_Map_Flagdata_Data_Packet() {}

/*
 * Read Function
 */
void Widelands_Map_Flagdata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
   if( skip )
      return;

   FileRead fr;
   try {
      fr.Open( fs, "binary/flag_data" );
   } catch ( ... ) {
      // not there, so skip
      return ;
   }

   // First packet version
   int packet_version=fr.Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      while(1) {
         uint ser=fr.Unsigned32();

         if(ser==0xffffffff) break; // end of flags
         assert(ol->is_object_known(ser));
         assert(ol->get_object_by_file_index(ser)->get_type()==Map_Object::FLAG);

         Flag* flag=static_cast<Flag*>(ol->get_object_by_file_index(ser));

         // The owner is already set, nothing to do from
         // PlayerImmovable

         flag->m_position.x=fr.Unsigned16();
         flag->m_position.y=fr.Unsigned16();
         flag->m_animstart=fr.Unsigned16();
         int building=fr.Unsigned32();
         if(building) {
            assert(ol->is_object_known(building));
            flag->m_building=static_cast<Building*>(ol->get_object_by_file_index(building));
         } else
            flag->m_building=0;


         // Roads are set somewhere else

         for(uint i=0; i<6; i++)
            flag->m_items_pending[i]=fr.Unsigned32();
         flag->m_item_capacity=fr.Unsigned32();
         flag->m_item_filled=fr.Unsigned32();

         // items
         for(int i=0; i<flag->m_item_filled; i++) {
            flag->m_items[i].pending=fr.Unsigned8();
            uint item=fr.Unsigned32();
            assert(ol->is_object_known(item));
            flag->m_items[i].item=static_cast<WareInstance*>(ol->get_object_by_file_index(item));

            uint nextstep=fr.Unsigned32();
            if(nextstep) {
               assert(ol->is_object_known(nextstep));
               flag->m_items[i].nextstep=static_cast<PlayerImmovable*>(ol->get_object_by_file_index(nextstep));
            } else {
               flag->m_items[i].nextstep=0;
            }
         }

         // always call
         uint always_call=fr.Unsigned32();
         if(always_call) {
            assert(ol->is_object_known(always_call));
            flag->m_always_call_for_flag=static_cast<Flag*>(ol->get_object_by_file_index(always_call));
         } else
            flag->m_always_call_for_flag=0;

         // Workers waiting
         uint nr_workers=fr.Unsigned16();
         flag->m_capacity_wait.resize(nr_workers);
         for(uint i=0; i<nr_workers; i++) {
            uint id=fr.Unsigned32();
            assert(ol->is_object_known(id));
            flag->m_capacity_wait[i]=ol->get_object_by_file_index(id);
         }

         // Flag jobs
         uint nr_jobs=fr.Unsigned16();
         assert(!flag->m_flag_jobs.size());
         for(uint i=0; i<nr_jobs; i++) {
            Flag::FlagJob f;
            bool request=fr.Unsigned8();
            if(!request)
               f.request=0;
            else {
               f.request = new Request(flag, 1,
	                        &Flag::flag_job_request_callback, flag, Request::WORKER);
               f.request->Read(&fr, egbase, ol);
            }
            f.program=fr.CString();
            flag->m_flag_jobs.push_back(f);
         }

         // Path finding variables are not saved
//
//         flag->mpf_cycle=fr.Unsigned32();
//         flag->mpf_heapindex=fr.Signed32();
//         flag->mpf_realcost=fr.Signed32();
//         uint backlink=fr.Unsigned32();
//         if(backlink) {
//            assert(ol->is_object_known(backlink));
//            flag->mpf_backlink=static_cast<Flag*>(ol->get_object_by_file_index(backlink));
//         } else
//             flag->mpf_backlink=0;
//         flag->mpf_estimate=fr.Signed32();

         ol->mark_object_as_loaded(flag);
      }


      // DONE
      return;
   }
   throw wexception("Unknown version %i in Widelands_Map_Flagdata_Data_Packet!\n", packet_version);

   assert( 0 );
}


/*
 * Write Function
 */
void Widelands_Map_Flagdata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         Field* f=map->get_field(Coords(x,y));
         BaseImmovable* imm=f->get_immovable();
         if(!imm) continue;

         if(imm->get_type()==Map_Object::FLAG) {
            Flag* flag=static_cast<Flag*>(imm);

            assert(os->is_object_known(flag));
            assert(!os->is_object_saved(flag));

            // Write serial
            fw.Unsigned32(os->get_object_file_index(flag));

            // Owner is already written in the existanz packet

            // Write position
            fw.Unsigned16(flag->m_position.x);
            fw.Unsigned16(flag->m_position.y);

            // Animation is set by creator
            fw.Unsigned16(flag->m_animstart);

            // Building is not used, it is set by Building_Data packet through
            // attach building.
            if(flag->m_building) {
               assert(os->is_object_known(flag->m_building));
               fw.Unsigned32(os->get_object_file_index(flag->m_building));
            } else {
               fw.Unsigned32(0);
            }

            // Roads are not saved, they are set on load

            // Pending items
            for(uint i=0; i<6; i++)
                  fw.Unsigned32(flag->m_items_pending[i]);

            // Capacity
            fw.Unsigned32(flag->m_item_capacity);

            // Items filled
            fw.Unsigned32(flag->m_item_filled);

            // items
            for(int i=0; i<flag->m_item_filled; i++) {
               fw.Unsigned8(flag->m_items[i].pending);
               assert(os->is_object_known(flag->m_items[i].item));
               fw.Unsigned32(os->get_object_file_index(flag->m_items[i].item));
               if(os->is_object_known(flag->m_items[i].nextstep))
                  fw.Unsigned32(os->get_object_file_index(flag->m_items[i].nextstep));
               else
                  fw.Unsigned32(0);
            }

            // always call
            if(flag->m_always_call_for_flag) {
               assert(os->is_object_known(flag->m_always_call_for_flag));
               fw.Unsigned32(os->get_object_file_index(flag->m_always_call_for_flag));
            } else {
               fw.Unsigned32(0);
            }

            // Worker waiting for capacity
            fw.Unsigned16(flag->m_capacity_wait.size());
            for(uint i=0; i<flag->m_capacity_wait.size(); i++) {
               Map_Object* obj=flag->m_capacity_wait[i].get(egbase);
               assert(os->is_object_known(obj));
               fw.Unsigned32(os->get_object_file_index(obj));
            }

            // Flag jobs
            fw.Unsigned16(flag->m_flag_jobs.size());
            for(std::list<Flag::FlagJob>::iterator i=flag->m_flag_jobs.begin();
                  i!=flag->m_flag_jobs.end(); i++) {
               if(i->request) {
                  fw.Unsigned8(1);
                  i->request->Write(&fw,egbase,os);
               } else fw.Unsigned8(0);


               fw.CString(i->program.c_str());
            }

            os->mark_object_as_saved(flag);

            // Path finding variables are not saved.
            //fw.Unsigned32(flag->mpf_cycle);
            //fw.Signed32(flag->mpf_heapindex);
            //fw.Signed32(flag->mpf_realcost);
            //if(flag->mpf_backlink) {
            //   assert(os->is_object_known(flag->mpf_backlink));
            //   fw.Unsigned32(os->get_object_file_index(flag->mpf_backlink));
            //} else
            //   fw.Unsigned32(0);
            //fw.Signed32(flag->mpf_estimate);
         }
      }
   }

   fw.Unsigned32(0xffffffff); // End of flags

   fw.Write( fs, "binary/flag_data" );
   // DONE
}
