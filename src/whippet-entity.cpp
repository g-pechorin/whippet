//Whippet; A container for entity component systems.
//Copyright (C) 2017-2018 Peter LaValle / gmail
//
//This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//See the GNU Affero General Public License for more details.
//
//You should have received a copy of the GNU Affero General Public License (agpl-3.0.txt) along with this program.
//If not, see <https://www.gnu.org/licenses/>.


#include "whippet.hpp"

whippet::entity::entity(void) :
	_world(nullptr),
	_guid(0)
{
}


whippet::entity::entity(whippet::universe* world, const whippet::guid_t guid) :
	_world(world),
	_guid(guid)
{
}

whippet::entity& whippet::entity::operator=(const whippet::entity&  other)
{
	return *new (this) whippet::entity(other._world, other._guid);
}

whippet::guid_t whippet::entity::guid(void) const
{
	return _guid;
}

void whippet::entity::remove(void)
{
	// detach all components
	// ... this loops is a bit weird; sorry
	whippet::_component* attached;
	do {
		attached = nullptr;

		visit<whippet::_component*>(attached, [](whippet::_component*& attached, whippet::_component& component)
		{
			assert(nullptr == attached);
			attached = &component;
			return false;
		});

		// did we find something?
		if (attached)
			attached->detach();
	} while (attached);
}

whippet::universe& whippet::entity::world(void) const
{
	assert(nullptr != _world);
	return *_world;
}
