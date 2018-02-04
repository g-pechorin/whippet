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

#ifdef whippet__porcelain

size_t whippet::porcelain::component_count(whippet::entity entity, bool(*filter)(whippet::_component&))
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

	entity.visit<counter>(self, [](counter& self, whippet::_component& next)
	{
		if (self._filter(next))
			++(self._count);

		return true;
	});

	return self._count;
}

#endif
