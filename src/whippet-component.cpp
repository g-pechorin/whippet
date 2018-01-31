
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
