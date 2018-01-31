
#pragma once

#include "whippet.hpp"

#include <array>

template<typename C>
inline
C* whippet::_component::as(void)
{
	return reinterpret_cast<C*>(
		_manager->as(
			std::type_index(typeid(C)),
			this));
}

template<typename C, typename ... ARGS>
inline
C& whippet::entity::attach(ARGS&&... args)
{
	const auto key = std::type_index(typeid(C));
	auto& val = _world->_providers[key];
	auto ram = val->alloc(*this);
	auto guy = new (ram) C(args...);
	assert(guy->owner()._guid == _guid);
	return *guy;
}

template<typename T, typename C>
void whippet::entity::visit(T& userdata, bool(*callback)(T&, C&))
{
	_world->visit_(
		_guid, std::type_index(typeid(C)),
		reinterpret_cast<void*>(&userdata),
		reinterpret_cast<bool(*)(void*, void*)>(callback)
	);
}

template<typename T>
void whippet::entity::visit(T& userdata, bool(*callback)(T&, whippet::_component&))
{
	_world->visit_(
		_guid, std::type_index(typeid(whippet::_component)),
		reinterpret_cast<void*>(&userdata),
		reinterpret_cast<bool(*)(void*, void*)>(callback)
	);
}

template<typename C>
inline
void whippet::universe::install(void)
{
	const auto kind = std::type_index(typeid(C));

	assume(!installed_(kind), "Duplicate invocations of install could bloat the binary");
	if (installed_(kind))
		return;

	struct provider : whippet::_provider
	{
		bool is(const std::type_index id) override
		{
			if (std::type_index(typeid(int)) != std::type_index(typeid(const int)))
				assume(
					std::type_index(typeid(const C)) != id,
					"Oppsie"
				);

			return std::type_index(typeid(C)) == id;
		}

		void* as(const std::type_index id, _component* me) override
		{
			if (id != std::type_index(typeid(C)))
				return nullptr;

			return reinterpret_cast<void*>(static_cast<C*>(me));
		}

		// need this to handler pre-init
		struct record
		{
			record(void) = delete;
			record(const record&) = delete;

			record& operator=(const record&) = delete;

			uint8_t _data[sizeof(C)];

			C* get_T(void) { return reinterpret_cast<C*>(_data); }

			whippet::_component* get_c(void) { return static_cast<whippet::_component*>(get_T()); }

			const whippet::_component* see_c(void)const { return static_cast<const whippet::_component*>(reinterpret_cast<const C*>(_data)); }

			const C* get_T(void) const { return reinterpret_cast<const C*>(_data); }

			const whippet::_component* get_c(void) const { return static_cast<const whippet::_component*>(get_T()); }

			bool inuse(void) const { return get_c()->inuse(); }

			record(const whippet::entity& e, whippet::guid_t g)
			{
				auto comp = get_c();

				assert(0 != g._weak);

				// pre-new the base component
				comp->_owner = e;
				comp->_guid = g;
				comp->_manager = e.world()._providers[std::type_index(typeid(C))].get();

				// hackery; please excuse
				assert(e._guid == comp->_owner._guid);
				assert(e._world == comp->_owner._world);
				assert(g == comp->_guid);
			}

			~record(void)
			{
				get_T()->~C();
				assert((!inuse()) && "Needs to be fresh before we can clean");
			}

			static bool inuse(const record* r)
			{
				return (r->see_c()->inuse());
			}

			static void clean(record* r)
			{
				r->get_c()->_guid = 0;
			}
		};

		hanoi<record> _storage;

		provider(void) { }

		/// returns a pointer to a new instance of the derived-class for in-place allocation
		void* alloc(const whippet::entity& owner) override
		{
			auto& emplaced = _storage.emplace_unspecified(owner, owner.world().guid_activate());
			auto pointer = emplaced.get_T();
			return reinterpret_cast<void*>(pointer);
		}

		/// destroys an instance
		void detach(whippet::_component* self) override
		{
			for (auto it = _storage.begin(); it != _storage.end(); ++it)
				if (it->get_c() == self)
				{
					_storage.erase(it);
					return;
				}

			assert(false && "Coudn't find component - was it already detached?");
		}

		void purge(void) override
		{
			while (!_storage.empty())
			{
				auto& head = *(_storage.begin());
				auto comp = head.get_c();

				assert(this == comp->_manager);
				comp->detach();
			}
		}

		bool visit(const whippet::guid_t entity_guid, const bool cast_to_kind, void* userdata, bool(*callback)(void*, void*)) override
		{
			for (auto& storage : _storage)
				if ((entity_guid == 0) || ((entity_guid != 0) && (storage.get_c()->_owner._guid == entity_guid)))
					if (!callback(userdata, cast_to_kind ? storage.get_T() : storage.get_c()))
						return false;

			return true;
		}

		void weed(void) override { _storage.weed(); }

#if _DEBUG
		virtual ~provider(void) override
		{
			// needs to be empty due to ... reasons ...
			assert(_storage.empty());
		}
#endif
	};

	_providers[kind] = std::make_unique<provider>();
}

template<typename T>
inline
bool whippet::universe::installed(void) const
{
	return installed_(std::type_index(typeid(T)));
}

template<typename S>
inline
S& whippet::universe::system(void)
{
	const auto name = std::type_index(typeid(S));
	for (auto next = _systems; nullptr != next; next = next->_next)
		if (name == next->_name)
			return *static_cast<S*>(next);

	//
	auto object = reinterpret_cast<S*>(malloc(sizeof(S)));

	object->_next = _systems;
	object->_world = this;
	object->_name = name;

	// the s-static cast means that I/we need this sorf of funkiness
	// ... could use a virtual destructor and retain the pointer ... might be smaller actually
	object->_cleanup = [](whippet::_system* data)
	{
		auto next = data->_next;

		if (nullptr != next)
			next->_cleanup(next);

		auto object = static_cast<S*>(data);
		object->~S();
		free(object);
	};

	_systems = static_cast<whippet::_system*>(object);

	return *(new (object) S());
}

template<typename T, typename C>
void whippet::universe::visit(T& userdata, bool(*callback)(T&, C&))
{
	visit_(
		0, std::type_index(typeid(C)),
		reinterpret_cast<void*>(&userdata),
		reinterpret_cast<bool(*)(void*, void*)>(callback)
	);
}

#ifdef whippet__porcelain

template<typename C>
size_t whippet::porcelain::component_count(whippet::entity entity, bool(*filter)(C&))
{
	struct counter
	{
		size_t _count;
		bool(*_filter)(C&);
	};

	counter self = {
		(size_t)0,
		filter,
	};

	entity.visit<counter, C>(self, [](counter& self, C& next)
	{
		if (self._filter(next))
			++(self._count);

		return true;
	});

	return self._count;
}

template<typename C>
C& whippet::porcelain::component(whippet::entity entity, const uint32_t index)
{
	struct output_t
	{
		C* _data;
		uint32_t _togo;
	};

	output_t output = {
		nullptr,
		index,
	};

	assume(index < whippet::porcelain::component_count<C>(entity));

	// iterate through stuff on us until we find one that's relevant
	entity.visit<output_t, C>(output, [](output_t& output, C& found)
	{
		if (output._togo)
			--(output._togo);
		else
			output._data = &found;

		return nullptr == output._data;
	});

	assume(nullptr != output._data);

	return *(output._data);
}


#endif
