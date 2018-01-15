
#include "whippet.hpp"

whippet::_component::_component(void) :
	_owner(owner()),
	_guid(guid())
{
	assert((!is_fresh()) && "You're probably attaching a component with no args - that doesn't work in V$");
}

whippet::_component::~_component(void)
{
	assert(!is_fresh());

	_owner.world().guid_release(_guid);
	_guid = 0;
	_owner = whippet::entity();
}

void whippet::_component::detach(void)
{
	assert(nullptr != _manager);
	_manager->detach(this);
}

whippet::guid_t whippet::_component::guid(void) const
{
	return _guid;
}

bool whippet::_component::is_fresh(void) const
{
	return _guid == 0;
}

whippet::entity whippet::_component::owner(void) const
{
	return _owner;
}

whippet::universe& whippet::_component::world(void) const
{
	return _owner.world();
}
