
// Peter LaValle
// 2018-01-15
//
// this provides a contiguous container for objects
// - proper object lifecycle
// - includes iteration
// - emplace to an unspecified location
// - needs inuse() and clean() methods on data object
//
#pragma once

#include <assert.h>
#include <stdint.h>

#include <vector>
#include <array>
#include <algorithm>
#include <memory>

#include "generator.hpp"

template <typename E>
class hanoi final
{
	static const uint8_t LAYER_SIZE_INITIAL = 14;
	static const uint8_t LAYER_SIZE_EXPAND = 3;
	static const uint8_t LAYER_SIZE_LIMIT = 143;

	static_assert(0 <= LAYER_SIZE_INITIAL, "the initial size of the layers must be > 0");

	friend struct iterator_forward;

	// TODO; perform byte alignment

	class entry final
	{
		uint8_t _data[sizeof(E)];
	public:
		E* get(void) { return reinterpret_cast<E*>(_data); }

		const E* see(void) const { return reinterpret_cast<const E*>(_data); }

		static bool inuse(const entry* e)
		{
			return E::inuse(e->see());
		}

		static void clean(entry* e)
		{
			E::clean(e->get());
			assert(!inuse(e));
		}

		entry(void)
		{
			clean(this);
		}

		~entry(void)
		{
			assert(!inuse(this));
		}
	};

	/// layers contain (some number of) entries
	struct layer final
	{
		std::vector<entry> _data;
		std::unique_ptr<layer> _next;

		layer(const size_t size) :
			_data(size),
			_next(nullptr)
		{
		}

		/// "weed" out empty layers
		void weed(std::unique_ptr<layer>& self)
		{
			// first; do the next one
			if (_next)
				_next->weed(_next);

			// scan for an in-use item and return IFF one exists
			for (auto & e : _data)
				if (entry::inuse(&e))
					return;

			// cool; nothing is being used in *this* layer - wipe it out
			self = std::move(_next);
		}
	};

	std::unique_ptr<layer> _data = nullptr;

	template<const bool LIVE>
	$generator(layer_iterator)
	{
		uint32_t _entry;
		typename layer* _layer;

		layer_iterator(layer* _) :
			_layer(_)
		{
		}

		$emit(typename entry*)
		{
			for (; nullptr != _layer; _layer = _layer->_next.get())
				for (_entry = 0; _entry < _layer->_data.size(); ++_entry)
					if (LIVE == entry::inuse(_layer->_data.data() + _entry))
						$yield(&(_layer->_data[_entry]));
		}
		$stop;
	};
public:

	/// allows "weeding" unused data
	void weed(void) { if (_data) _data->weed(_data); }

	struct iterator_forward final
	{
		iterator_forward(void) = delete;
		~iterator_forward(void);

		E& operator*(void);
		E* operator->(void);

		iterator_forward& operator++(void);

		bool operator!=(const iterator_forward&) const;
		bool operator==(const iterator_forward&) const;

	private:
		friend class hanoi<E>;
		typename layer_iterator<true> _inner;
		typename entry* _last;

		iterator_forward(typename layer*);
	};

	hanoi(void) : _data(nullptr) { }
	~hanoi(void) { while (!empty()) { erase(begin()); } }

	hanoi(const hanoi&) = delete;
	hanoi& operator=(const hanoi&) = delete;

	template <typename ...ARGS>
	E& emplace_unspecified(ARGS&& ...);

	void erase(const iterator_forward&);

	iterator_forward begin(void) { return hanoi<E>::iterator_forward::iterator_forward(_data.get()); }
	iterator_forward end(void) { return hanoi<E>::iterator_forward::iterator_forward(nullptr); }

	bool empty(void) { return begin() == end(); }

	/// erase the referenced element
	void erase(E& element)
	{
		assert(nullptr != _data);

		for (auto it = begin(); it != end(); ++it)
		{
			if ((&element) != &(*it))
				continue;

			erase(it);
			return;
		}

		warn("tried to erase non-existant element");
	}
};

//
// ==== ==== ==== ====
// Implementation
//

template <typename E>
template <typename ...ARGS>
inline
E& hanoi<E>::emplace_unspecified(ARGS&& ... args)
{
	hanoi<E>::entry* place;
	hanoi<E>::layer_iterator<false> layers(_data.get());

	// see if there's a place in an old layer
	if (layers(place))
	{
		// this spot is free!
		assert(!hanoi<E>::entry::inuse(place));

		// create a component (remeber that you should mark it as used ASAP)
		auto emplaced = new (place->get()) E(args...);

		// check to be sure that worked
		assert(hanoi<E>::entry::inuse(place));

		// return the result
		return *emplaced;
	}

	// add a new layer
	{
		// determine grown size
		size_t size = nullptr != _data
			? _data->_data.size() + hanoi::LAYER_SIZE_EXPAND
			: hanoi::LAYER_SIZE_INITIAL;

		// limit if applicable
		if (hanoi::LAYER_SIZE_LIMIT)
			size = std::min<size_t>(hanoi::LAYER_SIZE_LIMIT, size);

		// create the layer
		std::unique_ptr<layer> next = std::make_unique<layer>(size);

		// put the layer into place
		next->_next = std::move(_data);
		_data = std::move(next);
	}

	// recur
	return emplace_unspecified(args...);
}

template <typename E>
inline
void hanoi<E>::erase(typename const hanoi<E>::iterator_forward& position)
{
	assert(nullptr != position._last);

	assume(hanoi<E>::entry::inuse(position._last));
	if (hanoi<E>::entry::inuse(position._last))
	{
		position._last->get()->~E();
	}
	assume(!(hanoi<E>::entry::inuse(position._last)));
}

template <typename E>
inline
hanoi<E>::iterator_forward::iterator_forward(typename hanoi<E>::layer* layer) :
	_inner(layer)
{
	if (!(_inner(_last)))
		_last = nullptr;
}

template <typename E>
inline
E& hanoi<E>::iterator_forward::operator*(void)
{
	return *(_last->get());
}

template <typename E>
inline
E* hanoi<E>::iterator_forward::operator->(void)
{
	return _last->get();
}

template <typename E>
inline
typename hanoi<E>::iterator_forward& hanoi<E>::iterator_forward::operator++(void)
{
	if (!(_inner(_last)))
		_last = nullptr;
	return *this;
}

template <typename E>
inline
bool hanoi<E>::iterator_forward::operator!=(typename const hanoi<E>::iterator_forward& other)const
{
	return !((*this) == other);
}

template <typename E>
inline
bool hanoi<E>::iterator_forward::operator==(typename const hanoi<E>::iterator_forward& other)const
{
	return _last == other._last;
}

template <typename E>
inline
hanoi<E>::iterator_forward::~iterator_forward(void)
{
}
