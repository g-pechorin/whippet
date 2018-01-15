
#include "whippet.hpp"

#include <array>

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
void whippet::entity::forall(T& userdata, bool(*callback)(T&, C&))
{
	_world->forall_(
		_guid,
		std::type_index(typeid(C)),
		reinterpret_cast<void*>(&userdata),
		reinterpret_cast<bool(*)(void*, void*)>(callback)
	);
}

template<typename T>
void whippet::entity::forany(T& userdata, bool(*callback)(T&, whippet::_component&))
{
	_world->forany_(
		_guid,
		reinterpret_cast<void*>(&userdata),
		reinterpret_cast<bool(*)(void*, whippet::_component*)>(callback)
	);
}

template<typename C>
inline
void whippet::universe::install(void)
{
	const auto kind = std::type_index(typeid(C));
	if (installed_(kind))
		return;

	struct provider : whippet::_provider
	{
		// need this to handler pre-init
		struct record
		{
			record(void) = delete;
			record(const record&) = delete;

			record& operator=(const record&) = delete;

			uint8_t _data[sizeof(C)];

			C* get_T(void)
			{
				return reinterpret_cast<C*>(_data);
			}

			whippet::_component* get_c(void)
			{
				return static_cast<whippet::_component*>(get_T());
			}

			const C* get_T(void) const { return reinterpret_cast<const C*>(_data); }

			const whippet::_component* get_c(void) const { return static_cast<const whippet::_component*>(get_T()); }

			bool is_fresh(void) const
			{
				return get_c()->is_fresh();
			}

			record(const whippet::entity& e, whippet::guid_t g)
			{
				auto comp = get_c();

				// create the base component
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
				assert(is_fresh() && "Needs to be fresh before we can clean");
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
			{
				auto& storage = *it;
				if (storage.get_c() == self)
				{
					storage.get_T()->~C();

					storage.get_c()->_guid = 0;

					storage.get_c()->_owner = whippet::entity();
					storage.get_c()->_manager = nullptr;

					_storage.erase(it);
					return;
				}
			}
			assert(false && "Coudn't find component - was it already detached?");
		}

		void forall(const whippet::guid_t entity_guid, void* userdata, bool(*callback)(void*, void*)) override
		{
			for (auto& next : _storage)
				if (entity_guid == next.get_c()->_owner._guid)
					if (!callback(userdata, next.get_T()))
						return;
		}

		void forany(const whippet::guid_t entity_guid, void* userdata, bool(*callback)(void*, _component*))
		{
			struct original
			{
				void* _userdata;
				bool(*_callback)(void*, _component*);
			};

			original self{
				userdata,
				callback
			};

			forall(entity_guid, &self, [](void* u, void* c)
			{
				return reinterpret_cast<original*>(u)->_callback(
					reinterpret_cast<original*>(u)->_userdata,
					static_cast<whippet::_component*>(reinterpret_cast<C*>(c))
				);
			});
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

		virtual ~provider(void)
		{
			// needs to be empty due to ... reasons ...
			assert(_storage.empty());
		}
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

#ifdef whippet__util

template<typename C>
size_t whippet::util::component_count(whippet::entity& entity, bool(*filter)(C&))
{
	struct counter
	{
		size_t _count;
		bool(*_filter)(C&);
	};

	counter self{
		(size_t)0,
		filter
	};

	entity.forall<counter, C>(self, [](counter& self, C& next)
	{
		if (self._filter(next))
			++(self._count);

		return true;
	});

	return self._count;
}

#endif
