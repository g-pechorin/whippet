
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
	while (true)
	{
		whippet::_component* attached = nullptr;

		forany<whippet::_component*>(attached, [](whippet::_component*& attached, whippet::_component& component)
		{
			assert(nullptr == attached);
			attached = &component;
			return false;
		});

		// did we find something?
		if (!attached)
			break;

		attached->detach();
	}
}

whippet::universe& whippet::entity::world(void) const
{
	assert(nullptr != _world);
	return *_world;
}
