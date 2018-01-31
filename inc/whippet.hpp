
///
/// my minimal-footprint remake of https://google.github.io/corgi/
///
///
///

#pragma once

// ifdef; some "utility" junk is included that purists may not tolerate
// ... the utility routines are done entirely in user-land so there's no reason that *you* couldn't do them by hand, but, they're used by the unit tests to achieve full coverage
#define whippet__porcelain

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
		entity(universe*, const guid_t);

		entity& operator=(const entity&);

		template<typename C, typename ...ARGS>
		C& attach(ARGS&&...);

		/// iterate through all components of any type
		template<typename T>
		void visit(T& userdata, bool(*callback)(T&, _component&));

		/// iterate through components with the named type
		template<typename T, typename C>
		void visit(T& userdata, bool(*callback)(T&, C&));

		/// destroy this entity (and any attached components)
		void remove(void);

		universe& world(void) const;
	private:
		friend struct universe;
		universe* _world;
		guid_t _guid;
	};

	/// a base class for components
	struct _component
	{
		_component(const _component&) = delete;
		_component& operator=(const _component&) = delete;

		~_component(void);

		universe& world(void) const;
		entity owner(void) const;
		guid_t guid(void) const;

		/// remove this component from the entity
		/// this is (by necesity) sort of a front-end for the actual removal logic
		void detach(void);

		/// casts or nulls
		template<typename C>
		C* as(void);

		bool is(const std::type_index) const;

		template<typename C>
		bool is(void) const { return is(std::type_index(typeid(C))); }

	protected:
		_component(void);
	private:
		friend struct universe;
		entity _owner;
		guid_t _guid;
		_provider* _manager;
		bool inuse(void) const;
	};


	struct _provider
	{
		typedef std::unique_ptr<_provider> ptr;

		// TODO; use function pointers here instead

		virtual void* alloc(const entity&) = 0;
		virtual void* as(const std::type_index id, _component* me) = 0;
		virtual bool is(const std::type_index id) = 0;
		virtual void detach(_component*) = 0;

		/// a nesescary evil to remove components before the provider is destroyed
		virtual void purge(void) = 0;

		virtual bool visit(const whippet::guid_t entity_guid, const bool cast_to_kind, void* userdata, bool(*callback)(void*, void*)) = 0;

		virtual void weed(void) = 0;


#if _DEBUG
		// this is *just* used to do an assertion on the cleanup of derrived classes
		virtual ~_provider(void) {}
#endif
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

		template<typename T, typename C>
		void visit(T&, bool(*)(T&, C&));

		void weed(void);
	private:
		friend struct _component;
		friend struct entity;

		std::set<guid_t> _guid_active;
		pal::map<std::type_index, _provider::ptr> _providers;
		struct _system* _systems;

		/// activate the next guid and return it
		guid_t guid_activate(void);

		/// release a guid that's no longer in use
		void guid_release(guid_t);

		// privates
		void visit_(const guid_t, const std::type_index, void*, bool(*)(void*, void*));
		bool installed_(const std::type_index)const;
	};

#ifdef whippet__porcelain
	/// utility methods to do stuff
	/// ... all done in user-land
	struct porcelain
	{
		porcelain(void) = delete;
		porcelain(const porcelain&) = delete;
		porcelain& operator=(const porcelain&) = delete;
		~porcelain(void) = delete;

		template<typename C>
		static size_t component_count(entity, bool(*)(C&) = [](C&) { return true; });

		static size_t component_count(entity, bool(*)(_component&) = [](whippet::_component&) { return true; });

		template<typename C>
		static C& component(entity, const uint32_t = 0);
	};
#endif
}

#include "whippet.inline.hpp"
