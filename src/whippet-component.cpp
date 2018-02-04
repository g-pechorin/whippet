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

whippet::_component::_component(void) :
	_owner(owner()),
	_guid(guid())
{
	assert((inuse()) && "You're probably attaching a component with no args - that doesn't work in V$");
}

whippet::_component::~_component(void)
{
	assert(inuse());

	_owner.world().guid_release(_guid);
	_guid = 0;
	assert(!inuse());
}

void whippet::_component::detach(void)
{
	assert(inuse());
	assert(nullptr != _manager);
	_manager->detach(this);
}

whippet::guid_t whippet::_component::guid(void) const
{
	assert(inuse());
	return _guid;
}

bool whippet::_component::inuse(void) const
{
	return _guid != 0;
}

bool whippet::_component::is(const std::type_index kind) const
{
	return _manager->is(kind);
}

whippet::entity whippet::_component::owner(void) const
{
	assert(inuse());
	return _owner;
}

whippet::universe& whippet::_component::world(void) const
{
	assert(inuse());
	return _owner.world();
}
