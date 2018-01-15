
///
/// my minimal-footprint remake of https://google.github.io/corgi/
///
/// this rewrite is marked with >#< and uses std::type_index to key components and systems as well as allowing vardric attachment
///

#pragma once

// ifdef; some "utility" junk is included that purists may not tolerate
// ... the utility routines are done entirely in user-land so there's no reason that *you* couldn't do them by hand, but, they're used by the unit tests to achieve full coverage
#define whippet__util

// this provides a pretty "assume()" macro that you may not care about
#include <pal.hpp>

// this provides a contiguous container
#include "hanoi.hpp"

#include <assert.h>
#include <stdint.h>

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeindex>

namespace whippet
{
	typedef pal::strong<uint32_t> guid_t;

	struct entity;
	struct _component;
	struct _provider;
	struct universe;
	struct _system;

	/// entities are really just a GUID which take a pointer along for the ride
	struct entity
	{
		guid_t guid(void) const;

		entity(void);

		entity& operator=(const entity&);

		template<typename C, typename ...ARGS>
		C& attach(ARGS&&...);

		/// iterate through all components of any type
		template<typename T>
		void forany(T& userdata, bool(*callback)(T&, _component&));

		/// iterate through components with the named type
		template<typename T, typename C>
		void forall(T& userdata, bool(*callback)(T&, C&));

		/// destroy this entity (and any attached components)
		void remove(void);

		universe& world(void) const;
	private:
		friend struct universe;
		universe* _world;
		guid_t _guid;
		entity(universe*, const guid_t);
	};

	/// a base class for components
	struct _component
	{
		_component(const _component&) = delete;
		_component& operator=(const _component&) = delete;

		/// does this need to be virtual?
		virtual ~_component(void);

		universe& world(void) const;
		entity owner(void) const;
		guid_t guid(void) const;

		/// remove this component from the entity
		/// this is (by necesity) sort of a front-end for the actual removal logic
		void detach(void);

		/// casts or crashes
		template<typename C>
		C* as(void);

	protected:
		_component(void);
	private:
		friend struct universe;
		entity _owner;
		guid_t _guid;
		_provider* _manager;
		bool is_fresh(void) const;
	};


	struct _provider
	{
		typedef std::unique_ptr<_provider> ptr;

		virtual void* alloc(const entity&) = 0;
		virtual void detach(_component*) = 0;
		virtual void forall(const whippet::guid_t entity_guid, void* userdata, bool(*callback)(void*, void*)) = 0;
		virtual void forany(const whippet::guid_t entity_guid, void* userdata, bool(*callback)(void*, _component*)) = 0;

		/// a nesescary evil to remove components before the provider is destroyed
		virtual void purge(void) = 0;
		virtual ~_provider(void);
	};

	struct _system
	{
		_system(const _system&) = delete;
		_system& operator=(const _system&) = delete;

		/// casts or crashes
		template<typename S>
		S* as(void);

		universe& world(void);
	protected:
		_system(void);
	private:
		friend struct universe;
		void(*_cleanup)(struct _system*);
		struct _system* _next;
		struct universe* _world;
		std::type_index _name;
	};

	/// a manager holds EVERYTHING
	struct universe
	{
		universe(const universe&) = delete;
		universe& operator=(const universe&) = delete;

		universe(void);
		~universe(void);

		entity create(void);

		template<typename T>
		void install(void);

		template<typename T>
		bool installed(void) const;

		template<typename S>
		S& system(void);

	private:
		friend struct _component;
		friend struct entity;

		friend struct provider;

		std::set<guid_t> _guid_active;
		pal::map<std::type_index, _provider::ptr> _providers;
		struct _system* _systems;

		/// activate the next guid and return it
		guid_t guid_activate(void);

		/// release a guid that's no longer in use
		void guid_release(guid_t);

		// privates
		void forall_(const guid_t, const std::type_index, void*, bool(*)(void*, void*));
		void forany_(const guid_t, void*, bool(*)(void*, _component*));
		bool installed_(const std::type_index)const;
	};

#ifdef whippet__util
	/// utility methods to do stuff
	/// ... all done in user-land
	struct util
	{
		util(void) = delete;
		util(const util&) = delete;
		util& operator=(const util&) = delete;

		template<typename C>
		static size_t component_count(entity&, bool(*)(C&));

		static size_t components_count(entity&, bool(*)(_component&));
	};
#endif
}

#include "whippet.inline.hpp"
