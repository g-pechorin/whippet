
#include "whippet.hpp"

whippet::_system::_system(void) :
	_cleanup(this->_cleanup),
	_next(this->_next),
	_world(this->_world),
	_name(this->_name)
{
}

whippet::universe& whippet::_system::world(void)
{
	return *_world;
}
