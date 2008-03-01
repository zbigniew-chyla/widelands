/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef FINDNODE_H
#define FINDNODE_H

#include <stdint.h>
#include <vector>

namespace Widelands {

struct FCoords;
class Map;

struct FindNode {
private:
	struct BaseCapsule {
		BaseCapsule() : refcount(1) {}
		virtual ~BaseCapsule() {}

		void addref() {refcount++;}
		void deref() {
			if (--refcount == 0)
				delete this;
		}
		virtual bool accept(const Map &, const FCoords& coord) const = 0;

		int refcount;
	};
	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(const T& _op) : op(_op) {}
		bool accept(const Map & map, const FCoords& coord) const {return op.accept(map, coord);}

		const T op;
	};

	BaseCapsule* capsule;

public:
	FindNode(const FindNode& o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindNode() {
		capsule->deref();
		capsule = 0;
	}
	FindNode& operator=(const FindNode& o) {
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template<typename T>
	FindNode(const T& op) {
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	bool accept(const Map & map, const FCoords& coord) const {
		return capsule->accept(map, coord);
	}
};

struct FindNodeCaps {
	FindNodeCaps(uint8_t mincaps) : m_mincaps(mincaps) {}

	bool accept(Map const &, const FCoords&) const;

private:
	uint8_t m_mincaps;
};

// Accepts fields if they are accepted by all subfunctors.
struct FindNodeAnd {
	FindNodeAnd() {}

	void add(const FindNode& findfield, bool negate = false);

	bool accept(Map const &, const FCoords&) const;

private:
	struct Subfunctor {
		bool negate;
		FindNode findfield;

		Subfunctor(const FindNode& _ff, bool _negate);
	};

	std::vector<Subfunctor> m_subfunctors;
};

// Accepts fields based on what can be built there
struct FindNodeSize {
	enum Size {
		sizeAny    = 0,   //  any field not occupied by a robust immovable
		sizeBuild,        //  any field we can build on (flag or building)
		sizeSmall,        //  at least small size
		sizeMedium,
		sizeBig,
		sizeMine,         //  can build a mine on this field
		sizePort,         //  can build a port on this field
	};

	FindNodeSize(Size size) : m_size(size) {}

	bool accept(Map const &, const FCoords&) const;

private:
	Size m_size;
};

// Accepts fields based on the size of immovables on the field
struct FindNodeImmovableSize {
	enum {
		sizeNone   = 1 << 0,
		sizeSmall  = 1 << 1,
		sizeMedium = 1 << 2,
		sizeBig    = 1 << 3
	};

	FindNodeImmovableSize(uint32_t sizes) : m_sizes(sizes) {}

	bool accept(Map const &, const FCoords&) const;

private:
	uint32_t m_sizes;
};

// Accepts a field if it has an immovable with a given attribute
struct FindNodeImmovableAttribute {
	FindNodeImmovableAttribute(uint32_t attrib) : m_attribute(attrib) {}

	bool accept(Map const &, const FCoords&) const;

private:
	uint32_t m_attribute;
};


// Accepts a field if it has the given resource
struct FindNodeResource {
	FindNodeResource(uint8_t res) : m_resource(res) {}

	bool accept(Map const &, const FCoords&) const;

private:
	uint8_t m_resource;
};

}

#endif // FINDNODE_H
