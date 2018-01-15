
/// Peter LaValle
/// 2018-01-15
///
/// WIP
///
/// this provides a contiguous container for objects
/// - proper object lifecycle
/// - includes iteration
/// - emplace to an unspecified location
///
#pragma once

#include <assert.h>
#include <stdint.h>

#include <vector>
#include <array>
#include <memory>

#include "generator.hpp"

template <typename E>
class hanoi final
{
	friend struct iterator_forward;

	// TODO; offer byte alignment

	struct entry final
	{
		uint8_t _data[sizeof(E)];
		bool _live;

		entry(void) :
			_live(false)
		{
		}

		E* get(void) { return reinterpret_cast<E*>(_data); }

		~entry(void)
		{
			assert(!_live);
		}
	};

	struct layer final
	{
		std::vector<entry> _data;
		std::unique_ptr<layer> _next;

		layer(const size_t size) :
			_data(size),
			_next(nullptr)
		{
		}

		~layer(void)
		{
		}
	};

	std::unique_ptr<layer> _data = nullptr;

	$generator(layer_iterator)
	{
		const bool _live;
		uint32_t _entry;
		typename layer* _layer;

		layer_iterator(layer* _, bool live) :
			_live(live),
			_layer(_)
		{
		}

		$emit(typename entry*)
		{
			for (; nullptr != _layer; _layer = _layer->_next.get())
				for (_entry = 0; _entry < _layer->_data.size(); ++_entry)
					if (_live == _layer->_data[_entry]._live)
						$yield(&(_layer->_data[_entry]));
		}
		$stop;
	};
public:

	struct iterator_forward final
	{
		iterator_forward(void) = delete;
		~iterator_forward(void);


		E& operator*(void);
		iterator_forward& operator++(void);
		bool operator!=(const iterator_forward&) const;
		bool operator==(const iterator_forward&) const;
	private:
		friend class hanoi<E>;
		typename layer_iterator _inner;
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
	/// ... TODO; this could be done better
	void erase(E& element)
	{
		assert(nullptr != _data);

		for (auto it = begin(); it != end(); ++it)
		{
			if (reinterpret_cast<size_t>(&element) != reinterpret_cast<size_t>(&(*it)))
				continue;

			erase(it);
			return;
		}

		assert(false && "tried to erase non-existant element");
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
	hanoi<E>::layer_iterator layers(_data.get(), false);

	// see if there's a place in an old layer
	if (layers(place))
	{
		// this spot is free!
		assert(false == place->_live);

		// CLAIM IT! (the inner code might try to do the same ... don't ask)
		place->_live = true;


		// settle in
		return *(new (place->get()) E(args...));
	}

	// add a new layer
	std::unique_ptr<layer> next = std::make_unique<layer>(nullptr != _data ? _data->_data.size() + 2 : 3);
	next->_next = std::move(_data);
	_data = std::move(next);

	// recur
	return emplace_unspecified(args...);
}

template <typename E>
inline
void hanoi<E>::erase(typename const hanoi<E>::iterator_forward& position)
{
	assert(nullptr != position._last);
	assert(position._last->_live);

	if (position._last->_live)
	{
		reinterpret_cast<E*>(position._last->_data)->~E();
		position._last->_live = false;
	}

	// TODO; wipeout totally empty layers
}

template <typename E>
inline
hanoi<E>::iterator_forward::iterator_forward(typename hanoi<E>::layer* layer) :
	_inner(layer, true)
{
	if (!(_inner(_last)))
		_last = nullptr;
}

template <typename E>
inline
E& hanoi<E>::iterator_forward::operator*(void)
{
	return *reinterpret_cast<E*>(_last->_data);
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
