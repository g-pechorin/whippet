
#include <whippet.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(whippet, nothing)
{
	;
}

TEST(whippet, turn_on)
{
	whippet::universe universe;
}

TEST(whippet, install)
{
	struct foonk : whippet::_component
	{
		foonk(const char*)
		{
		}
	};

	{
		whippet::universe world;
		world.install<foonk>();
	}

	if (false)
		FAIL() << "SAN";
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
		universe.install<foonk>();

		auto e0 = universe.create();
		auto e1 = universe.create();
		auto e2 = universe.create();

		auto& c0 = e1.attach<foonk>("foonk");
		ASSERT_NE(nullptr, &c0);
	}
	ASSERT_TRUE(was_eq);
}

#ifdef whippet__util
/// attach component and count it
TEST(whippet, counting)
{
	struct foonk : whippet::_component
	{
		std::string _name;
		foonk(const char* name) :
			_name(name)
		{
		}
	};
	struct boop : whippet::_component
	{
		boop(int)
		{}
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

	ASSERT_EQ(1, whippet::util::components_count(e0, [](whippet::_component&) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<boop>(e0, [](boop& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return them._name == "foonk"; }));

	ASSERT_EQ(1, whippet::util::components_count(e1, [](whippet::_component&) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<boop>(e1, [](boop& them) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<foonk>(e1, [](foonk& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e1, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(1, whippet::util::component_count<foonk>(e1, [](foonk& them) { return them._name == "foonk"; }));

	ASSERT_EQ(1, whippet::util::components_count(e2, [](whippet::_component&) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<boop>(e2, [](boop& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e2, [](foonk& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e2, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e2, [](foonk& them) { return them._name == "foonk"; }));

	e2.attach<foonk>("foonk");
	e2.attach<foonk>("foonk");
	e2.attach<foonk>("baur");
	e2.attach<foonk>("grop");

	ASSERT_EQ(1, whippet::util::components_count(e0, [](whippet::_component&) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<boop>(e0, [](boop& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e0, [](foonk& them) { return them._name == "foonk"; }));

	ASSERT_EQ(1, whippet::util::components_count(e1, [](whippet::_component&) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<boop>(e1, [](boop& them) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<foonk>(e1, [](foonk& them) { return true; }));
	ASSERT_EQ(0, whippet::util::component_count<foonk>(e1, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(1, whippet::util::component_count<foonk>(e1, [](foonk& them) { return them._name == "foonk"; }));

	ASSERT_EQ(5, whippet::util::components_count(e2, [](whippet::_component&) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<boop>(e2, [](boop& them) { return true; }));
	ASSERT_EQ(4, whippet::util::component_count<foonk>(e2, [](foonk& them) { return true; }));
	ASSERT_EQ(1, whippet::util::component_count<foonk>(e2, [](foonk& them) { return them._name == "baur"; }));
	ASSERT_EQ(2, whippet::util::component_count<foonk>(e2, [](foonk& them) { return them._name == "foonk"; }));
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
