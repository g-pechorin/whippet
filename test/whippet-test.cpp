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


#include <whippet.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

/// test to see if testing works
TEST(whippet, nothing)
{
	;
}

/// test to see if the "universe" can be created
TEST(whippet, turn_on)
{
	whippet::universe universe;
}

/// create an entity
TEST(whippet, create)
{
	whippet::universe()
		.create();
}

/// remove an entity
TEST(whippet, remove)
{
	whippet::universe()
		.create()
		.remove();
}

/// tries to add/remove entities to test that names don't collide
TEST(whippet, create2r1c2)
{
	whippet::universe universe;

	auto e0 = universe.create();
	auto e1 = universe.create();

	ASSERT_NE(e0.guid(), e1.guid());
	e0.remove();

	auto e2 = universe.create();
	ASSERT_NE(e1.guid(), e2.guid());
	auto e3 = universe.create();
	ASSERT_NE(e1.guid(), e3.guid());
	ASSERT_NE(e2.guid(), e3.guid());
}

/// test to install a component
TEST(whippet, install)
{
	/// this is the bare minimum that a component needs to implement
	struct foonk : whippet::_component
	{
		/// to hack around some subtle quirks of my approach - need to pass something to component constructors
		foonk(const char*)
		{
			// at this point; the baseclass has already been init so you're good to start doing stuff
		}
	};

	{
		whippet::universe world;
		world.install<foonk>();
	}
}

/// attach component
TEST(whippet, attach)
{
	static bool was_eq = false;
	struct foonk : whippet::_component
	{
		foonk(const char* name)
		{
			assert(!was_eq);
			was_eq = std::string("foonk") == name;
			assert(was_eq);
		}
	};

	{
		whippet::universe universe;

		universe.install<foonk>();

		auto e0 = universe.create();
		auto e1 = universe.create();
		auto e2 = universe.create();

		auto& c0 = e1.attach<foonk>("foonk");
		ASSERT_NE(nullptr, &c0);
	}
	ASSERT_TRUE(was_eq);
}

#ifdef whippet__porcelain
/// attach component and count it
TEST(whippet, counting)
{
	static int total;
	total = 0;
	struct foonk : whippet::_component
	{
		std::string _name;
		foonk(const char* name) :
			_name(name)
		{
			++total;
		}

		~foonk()
		{
			--total;
		}
	};
	struct boop : whippet::_component
	{
		boop(int)
		{
			++total;
		}

		~boop()
		{
			--total;
		}
	};

	{
		whippet::universe universe;

		universe.install<foonk>();
		universe.install<boop>();

		auto e0 = universe.create();
		auto e1 = universe.create();
		auto e2 = universe.create();

		ASSERT_EQ(0, total);

		e0.attach<boop>(3);
		e1.attach<foonk>("foonk");
		e2.attach<boop>(2);

		ASSERT_EQ(3, total);

		ASSERT_EQ(1, whippet::porcelain::component_count(e0));
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e0, [](boop& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return them._name == "foonk"; }));

		ASSERT_EQ(1, whippet::porcelain::component_count(e1, [](whippet::_component&) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<boop>(e1, [](boop& them) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return them._name == "foonk"; }));

		ASSERT_EQ(1, whippet::porcelain::component_count(e2, [](whippet::_component&) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e2, [](boop& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return them._name == "foonk"; }));

		e2.attach<foonk>("foonk");
		e2.attach<foonk>("foonk");
		e2.attach<foonk>("baur");
		e2.attach<foonk>("grop");

		ASSERT_EQ(1, whippet::porcelain::component_count(e0, [](whippet::_component&) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e0, [](boop& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0, [](foonk& them) { return them._name == "foonk"; }));

		ASSERT_EQ(1, whippet::porcelain::component_count(e1, [](whippet::_component&) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<boop>(e1, [](boop& them) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return true; }));
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e1, [](foonk& them) { return them._name == "foonk"; }));

		ASSERT_EQ(5, whippet::porcelain::component_count(e2, [](whippet::_component&) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e2, [](boop& them) { return true; }));
		ASSERT_EQ(4, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return true; }));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return them._name == "baur"; }));
		ASSERT_EQ(2, whippet::porcelain::component_count<foonk>(e2, [](foonk& them) { return them._name == "foonk"; }));
	}
	ASSERT_EQ(0, total);
}
#endif

/// tests the pre-init things from inside of a component
TEST(whippet, check_pre_new)
{
	static bool name_match = false;
	static whippet::universe* pointer = nullptr;
	static bool is_null = false;
	struct foonk : whippet::_component
	{
		foonk(const char* name)
		{
			is_null = nullptr == pointer;
			assert(nullptr == pointer);
			name_match = std::string("baur") == name;
			pointer = &(owner().world());
		}
	};

	whippet::universe universe;


	universe.install<foonk>();

	ASSERT_TRUE(universe.installed<foonk>());

	ASSERT_EQ(nullptr, pointer) << "the pointer isn't null when it should be";

	auto& c0 = universe.create().attach<foonk>("baur");

	ASSERT_EQ(&universe, pointer) << "the pointers don't patch";
	ASSERT_EQ(true, name_match) << "the names don't match";
}


/// tests the pre-init things from inside of a component
TEST(whippet, check_system_lifecycle)
{
	static whippet::universe* pointer = nullptr;
	static bool cleaned = false;
	struct foonk : whippet::_system
	{
		foonk(void)
		{
			pointer = &(world());
		}

		~foonk(void)
		{
			cleaned = true;
		}
	};

	ASSERT_EQ(nullptr, pointer) << "the pointer isn't null when it should be";
	{
		whippet::universe universe;

		ASSERT_EQ(nullptr, pointer) << "the pointer isn't null when it should be";

		universe.system<foonk>();

		ASSERT_EQ(&universe, pointer) << "the pointers don't patch";
	}

	ASSERT_EQ(true, cleaned) << "system wasn't cleaned up";
}

#ifdef whippet__porcelain
TEST(whippet, updoot_components)
{
	struct foonk : whippet::_component
	{
		std::string _name;
		int _count = 1;
		foonk(const char* name) :
			_name(name)
		{
		}
	};

	struct boop : whippet::_component
	{
		int _count;
		boop(int i) :
			_count(i)
		{
		}
	};

	whippet::universe universe;

	universe.install<foonk>();
	universe.install<boop>();

	auto e0 = universe.create();
	auto e1 = universe.create();
	auto e2 = universe.create();

	e0.attach<boop>(3);
	e1.attach<foonk>("foonk");
	e2.attach<boop>(2);

	// these need to be references
	auto& f0 = e2.attach<foonk>("foonk");
	auto& f1 = e2.attach<foonk>("baur");
	auto& f2 = e2.attach<foonk>("baur"); f2._count = -23;
	auto& f3 = e2.attach<foonk>("grop"); f3._count = -9;

	int i;
	universe.visit<int, foonk>(i, [](int&, foonk& component)
	{
		if (component._name == "baur")
			component._count += 3;
		return true;
	});

	universe.visit<int, boop>(i, [](int&, boop& component)
	{
		component._count += 1;
		return true;
	});

	// check boop
	{
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e0));
		ASSERT_EQ(0, whippet::porcelain::component_count<boop>(e1));
		ASSERT_EQ(1, whippet::porcelain::component_count<boop>(e2));

		ASSERT_EQ(4, whippet::porcelain::component<boop>(e0)._count);

		ASSERT_EQ(3, whippet::porcelain::component<boop>(e2)._count);
	}

	// check foonk
	{
		ASSERT_EQ(0, whippet::porcelain::component_count<foonk>(e0));
		ASSERT_EQ(1, whippet::porcelain::component_count<foonk>(e1));
		ASSERT_EQ(4, whippet::porcelain::component_count<foonk>(e2));

		ASSERT_EQ(1, f0._count);
		ASSERT_EQ(4, f1._count);
		ASSERT_EQ(-20, f2._count);
		ASSERT_EQ(-9, f3._count);
	}
}
#endif

/// tests the `is<?>()` function
TEST(whippet, is_as)
{
	struct foo : whippet::_component
	{
		foo(int) {}
	};
	struct bar : whippet::_component
	{
		bar(double) {}
	};

	whippet::universe universe;

	universe.install<foo>();
	universe.install<bar>();

	auto e0 = universe.create();

	// attached components are `whippet::_component&` so some method is needed to check before casting can be performed
	auto& component = e0.attach<foo>(19);
	ASSERT_FALSE(component.is<bar>());
	ASSERT_FALSE(e0.attach<bar>(.9).is<foo>());
	ASSERT_TRUE(e0.attach<foo>(18).is<foo>());
	ASSERT_TRUE(e0.attach<bar>(.8).is<bar>());
}
