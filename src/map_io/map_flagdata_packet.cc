/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "map_io/map_flagdata_packet.h"

#include <map>

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/building.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "logic/worker.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 4

void MapFlagdataPacket::Read
	(FileSystem            &       fs,
	 EditorGameBase      &       egbase,
	 bool                    const skip,
	 MapObjectLoader &       mol)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/flag_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version && packet_version <= CURRENT_PACKET_VERSION) {
			const Map  & map    = egbase.map();
			Extent const extent = map.extent();
			for (;;) {
				if (2 <= packet_version && fr.EndOfFile())
					break;
				Serial const serial = fr.Unsigned32();
				if (packet_version < 2 && serial == 0xffffffff) {
					if (!fr.EndOfFile())
						throw GameDataError
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Flag & flag = mol.get<Flag>(serial);

					//  Owner is already set, nothing to do from PlayerImmovable.

					if (packet_version < 3) {
						if
							(upcast
							 	(Flag const,
							 	 mf,
							 	 map[flag.m_position = ReadCoords32(&fr, extent)]
							 	 .get_immovable()))
						{
							if (mf != &flag)
								throw GameDataError
									("wrong flag (%u) at given position (%i, %i)",
									 mf->serial(),
									 flag.m_position.x, flag.m_position.y);
						} else
							throw GameDataError
								("no flag at given position (%i, %i)",
								 flag.m_position.x, flag.m_position.y);
					}
					flag.m_animstart = fr.Unsigned16();

					{
						FCoords building_position = map.get_fcoords(flag.m_position);
						map.get_tln(building_position, &building_position);
						flag.m_building =
							dynamic_cast<Building *>
								(building_position.field->get_immovable());
					}
					if (packet_version < 3) {
						if (uint32_t const building_serial = fr.Unsigned32())
							try {
								const Building & building =
									mol.get<Building>(building_serial);
								if (flag.m_building != &building)
									throw GameDataError
										(
									 	 "has building %u at (%i, %i), which is not "
									 	 "at the top left node",
										 building_serial,
										 building.get_position().x,
										 building.get_position().y);
							} catch (const WException & e) {
								throw GameDataError
									("building (%u): %s", building_serial, e.what());
							}
						else
							flag.m_building = nullptr;
					}

					//  Roads are set somewhere else.

					// Compatibility stuff: Read 6 bytes of attic stuff
					// that is no longer used (was m_wares_pending)
					for (uint32_t i = 0; i < 6; ++i)
						fr.Unsigned32();

					flag.m_ware_capacity = fr.Unsigned32();

					{
						uint32_t const wares_filled = fr.Unsigned32();
						flag.m_ware_filled = wares_filled;
						for (uint32_t i = 0; i < wares_filled; ++i) {
							flag.m_wares[i].pending = fr.Unsigned8();
							if (packet_version < 4)
								flag.m_wares[i].priority = 0;
							else
								flag.m_wares[i].priority = fr.Signed32();
							uint32_t const ware_serial = fr.Unsigned32();
							try {
								flag.m_wares[i].ware =
									&mol.get<WareInstance>(ware_serial);

								if (uint32_t const nextstep_serial = fr.Unsigned32()) {
									try {
										flag.m_wares[i].nextstep =
											&mol.get<PlayerImmovable>(nextstep_serial);
									} catch (const WException & e) {
										throw GameDataError
											("next step (%u): %s",
											 nextstep_serial, e.what());
									}
								} else
									flag.m_wares[i].nextstep = nullptr;
							} catch (const WException & e) {
								throw GameDataError
									("ware #%u (%u): %s", i, ware_serial, e.what());
							}
						}

						if (uint32_t const always_call_serial = fr.Unsigned32())
							try {
								flag.m_always_call_for_flag =
									&mol.get<Flag>(always_call_serial);
							} catch (const WException & e) {
								throw GameDataError
									("always_call (%u): %s",
									 always_call_serial, e.what());
							}
						else
							flag.m_always_call_for_flag = nullptr;

						//  workers waiting
						uint16_t const nr_workers = fr.Unsigned16();
						for (uint32_t i = 0; i < nr_workers; ++i) {
							uint32_t const worker_serial = fr.Unsigned32();
							try {
								//  The check that this worker actually has a
								//  waitforcapacity task for this flag is in
								//  Flag::load_finish, which is called after the worker
								//  (with his stack of tasks) has been fully loaded.
								flag.m_capacity_wait.push_back
									(&mol.get<Worker>(worker_serial));
							} catch (const WException & e) {
								throw GameDataError
									("worker #%u (%u): %s", i, worker_serial, e.what());
							}
						}

						//  flag jobs
						uint16_t const nr_jobs = fr.Unsigned16();
						assert(flag.m_flag_jobs.empty());
						for (uint16_t i = 0; i < nr_jobs; ++i) {
							Flag::FlagJob f;
							if (fr.Unsigned8()) {
								f.request =
									new Request
										(flag,
										 0,
										 Flag::flag_job_request_callback,
										 wwWORKER);
								f.request->Read
									(fr, ref_cast<Game, EditorGameBase>(egbase), mol);
							} else {
								f.request = nullptr;
							}
							f.program = fr.CString();
							flag.m_flag_jobs.push_back(f);
						}

						mol.mark_object_as_loaded(flag);
					}
				} catch (const WException & e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			}
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("flagdata: %s", e.what());
	}
}


void MapFlagdataPacket::Write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)

{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map & map = egbase.map();
	const Field & fields_end = map[map.max_index()];
	for (Field * field = &map[0]; field < &fields_end; ++field)
		if (upcast(Flag const, flag, field->get_immovable())) {
			assert(mos.is_object_known(*flag));
			assert(!mos.is_object_saved(*flag));

			fw.Unsigned32(mos.get_object_file_index(*flag));

			//  Owner is already written in the existanz packet.

			//  Animation is set by creator.
			fw.Unsigned16(flag->m_animstart);

			//  Roads are not saved, they are set on load.

			// Compatibility stuff: Write 6 bytes of attic stuff that is
			// no longer used (was m_wares_pending)
			for (uint32_t i = 0; i < 6; ++i)
				fw.Unsigned32(0);

			fw.Unsigned32(flag->m_ware_capacity);

			fw.Unsigned32(flag->m_ware_filled);

			for (int32_t i = 0; i < flag->m_ware_filled; ++i) {
				fw.Unsigned8(flag->m_wares[i].pending);
				fw.Signed32(flag->m_wares[i].priority);
				assert(mos.is_object_known(*flag->m_wares[i].ware));
				fw.Unsigned32(mos.get_object_file_index(*flag->m_wares[i].ware));
				if
					(PlayerImmovable const * const nextstep =
					 	flag->m_wares[i].nextstep.get(egbase))
					fw.Unsigned32(mos.get_object_file_index(*nextstep));
				else
					fw.Unsigned32(0);
			}

			if (Flag const * const always_call_for = flag->m_always_call_for_flag)
			{
				assert(mos.is_object_known(*always_call_for));
				fw.Unsigned32(mos.get_object_file_index(*always_call_for));
			} else
				fw.Unsigned32(0);

			//  worker waiting for capacity
			const Flag::CapacityWaitQueue & capacity_wait =
				flag->m_capacity_wait;
			fw.Unsigned16(capacity_wait.size());
			for (const OPtr<Worker >&  temp_worker : capacity_wait) {
				Worker const * const obj = temp_worker.get(egbase);
				assert
					(obj);
				assert
					(obj->get_state(Worker::taskWaitforcapacity));
				assert
					(obj->get_state(Worker::taskWaitforcapacity)->objvar1.serial()
					 ==
					 flag                                               ->serial());
				assert(mos.is_object_known(*obj));
				fw.Unsigned32(mos.get_object_file_index(*obj));
			}
			const Flag::FlagJobs & flag_jobs = flag->m_flag_jobs;
			fw.Unsigned16(flag_jobs.size());

			for (const Flag::FlagJob& temp_job : flag_jobs) {
				if (temp_job.request) {
					fw.Unsigned8(1);
					temp_job.request->Write
						(fw, ref_cast<Game, EditorGameBase>(egbase), mos);
				} else
					fw.Unsigned8(0);

				fw.String(temp_job.program);
			}

			mos.mark_object_as_saved(*flag);

		}

	fw.Write(fs, "binary/flag_data");
}

}