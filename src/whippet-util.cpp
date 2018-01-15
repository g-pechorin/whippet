
#include "whippet.hpp"

#ifdef whippet__util

size_t whippet::util::components_count(whippet::entity& entity, bool(*filter)(whippet::_component&))
{
	struct counter
	{
		size_t _count;
		bool(*_filter)(whippet::_component&);
	};

	counter self{
		(size_t)0,
		filter
	};

	entity.forany<counter>(self, [](counter& self, whippet::_component& next)
	{
		if (self._filter(next))
			++(self._count);

		return true;
	});

	return self._count;
}

#endif
