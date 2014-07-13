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

#ifndef WL_NOTIFICATIONS_NOTIFICATIONS_H
#define WL_NOTIFICATIONS_NOTIFICATIONS_H

#include <functional>
#include <memory>

#include "notifications/notifications_impl.h"

namespace Notifications {

// The Notification framework is build around a singleton that dispatches
// 'Note'. A Note is any class that has a static uint32_t kUniqueNoteId member
// that must be unique throughout the whole system.
//
// The only public interface for the framework are the two functions below.

// Subscribes to a Note of type 'T' with the given callback function. The
// returned object is opaque, but will unsubscribe on destruction.
template <typename T>
std::unique_ptr<Subscriber<T>> subscribe(std::function<void(const T&)> callback) {
	return NotificationsManager::get()->subscribe<T>(callback);
}

// Publishes 'message' to all existing subscribers.
template <typename T> void publish(const T& message) {
	return NotificationsManager::get()->publish<T>(message);
}

}  // namespace Notifications

#endif  // end of include guard: WL_NOTIFICATIONS_NOTIFICATIONS_H
