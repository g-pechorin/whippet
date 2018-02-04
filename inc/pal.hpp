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


/// Peter LaValle
/// 2017-12-26
///
/// The classes are organised in two phases, the first is made of "base" classes and the second layer includes classes made from this base layer
///
///
#pragma once


#include <assert.h>
#include <stdint.h>

#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

// #define real16_t ???
#define real32_t float
#define real64_t double

#ifdef _DEBUG
#	include <stdlib.h>
#	include <iostream>


#	define _fail__line__(LINE) #LINE
#	define _fail_line__(LINE) _fail__line__(LINE)
#	define _fail_line_ "\n\t>" __FUNCTION__ "(...)\n\t@" __FILE__ "\n\t:"  _fail_line__(__LINE__)

// if _DEBUG then fail spits out a big nice message/trace
#define fael(MESSAGE, ...) do { fflush(stdout); fprintf(stderr, "FAIL: "); fprintf(stderr, MESSAGE, __VA_ARGS__); fprintf(stderr,_fail_line_ "\n"); fflush(stderr); exit(EXIT_FAILURE); } while (false)

// like fail but, no exit
#define warn(MESSAGE, ...) do { fflush(stdout); fprintf(stderr, "WARN: "); fprintf(stderr, MESSAGE, __VA_ARGS__); fprintf(stderr,_fail_line_ "\n"); fflush(stderr); } while (false)

// debug-only!
#	define STUB(MESSAGE, ...) fael(MESSAGE, __VA_ARGS__)
#	define TODO(MESSAGE, ...) do { static bool TODO = true; if (TODO) { TODO = false; fflush(stdout); fprintf(stderr, "TODO: "); fprintf(stderr, MESSAGE, __VA_ARGS__); fprintf(stderr, _fail_line_ "\n"); fflush(stderr); } } while(false)

// like assert, but, doesn't exit
#	define assume(CONDITION, ...) do { if (!(CONDITION)) { fflush(stdout); fprintf(stderr, "ASSUMPTION; "  #CONDITION "\n"); fprintf(stderr, ""  __VA_ARGS__); fprintf(stderr, _fail_line_ "\n"); fflush(stderr); } } while (false)

// like assert, but, it's always used
#	define require(CONDITION, ...) do { if (!(CONDITION)) { fflush(stdout); fprintf(stderr, "REQUIREMENT: " #CONDITION "\n"); fprintf(stderr, "" __VA_ARGS__); fprintf(stderr,_fail_line_ "\n"); fflush(stderr); exit(EXIT_FAILURE); } } while (false)

// an assert that takes a long message
#	define passert(CONDITION, ...) do { if (!(CONDITION)) { fflush(stdout); fprintf(stderr, "ASSERT: " #CONDITION "\n"); fprintf(stderr, "" __VA_ARGS__); fprintf(stderr,_fail_line_ "\n"); fflush(stderr); exit(EXIT_FAILURE); } } while (false)


#define debug_var(NAME,VALUE) auto NAME = (VALUE)
#else

#	define fael(MESSAGE, ...) do { exit(EXIT_FAILURE); } while(false)
#	define warn(MESSAGE, ...) do { ; } while(false) 
#	define assume(CONDITION, ...) do { ; } while(false) 
#	define require(CONDITION, ...) do { if (!(CONDITION)) { exit(EXIT_FAILURE); } } while (false)
#	define passert(CONDITION, ...) do { } while (false)

#define debug_var(NAME,VALUE) 
#endif

#define WARN(MESSAGE) warn(MESSAGE _fail_line_)


#ifdef _MSC_VER
#	define NOINLINE __declspec(noinline)
#else
#	define NOINLINE __attribute__((noinline))
#endif

#define pal__operator_inlaid(T, A, B) \
		bool operator==(T them)const { return (A) == (B); }; \
		bool operator!=(T them)const { return (A) != (B); }; \
		bool operator<=(T them)const { return (A) <= (B); }; \
		bool operator>=(T them)const { return (A) >= (B); }; \
		bool operator<(T them)const { return (A) < (B); }; \
		bool operator>(T them)const { return (A) > (B); };

#define pal__operator_prototypes(T) \
		bool operator==(T)const; \
		bool operator!=(T)const; \
		bool operator<=(T)const; \
		bool operator>=(T)const; \
		bool operator<(T)const; \
		bool operator>(T)const;


#define pal__operator_implement(M, C, A, T, B) \
	bool C :: operator==(T them)const { return (A) == (B); }; \
	bool C :: operator!=(T them)const { return (A) != (B); }; \
	bool C :: operator<=(T them)const { return (A) <= (B); }; \
	bool C :: operator>=(T them)const { return (A) >= (B); }; \
	bool C :: operator<(T them)const { return (A) < (B); }; \
	bool C :: operator>(T them)const { return (A) > (B); };

namespace pal
{
	//
	// Phase 1
	//

	template<typename K, typename V>
	struct map : std::map<K, V>
	{
		bool contains(const K& value) const { return end() != find(value); }
	};

	/// hack to whack stuff into places where it doesn't want to be
	template<typename E>
	struct qpack
	{
		const E* _data;
		const uint32_t _size;
		const bool _free;

		size_t size(void) const { return _size; }

		qpack(const qpack&) = delete;
		qpack(const qpack&&) = delete;
		qpack& operator=(const qpack&) = delete;
		qpack& operator=(const qpack&&) = delete;

		qpack(const std::initializer_list<E> list) :
			_size((uint32_t)list.size()),
			_data((E*)malloc(sizeof(E) * list.size())),
			_free(true)
		{
			std::copy(list.begin(), list.end(), const_cast<E*>(_data));
		}

		qpack(const uint32_t len, E* ptr, bool cpy = true) :
			_size(len),
			_data(cpy ? (E*)memcpy(malloc(sizeof(E) * len), ptr, sizeof(E) * len) : ptr),
			_free(cpy)
		{
		}

		~qpack(void)
		{
			if (_free)
				free(const_cast<E*>(_data));
		}

		typedef const E* iterator;

		iterator begin(void) const
		{
			return _data;
		}
		iterator end(void) const
		{
			return _data + _size;
		}
	};

	template<typename E>
	struct queue : public std::queue<E>
	{
		E pull(void)
		{
			auto data = front();
			pop();
			return data;
		}
	};

	template<typename T>
	struct set : std::set<T>
	{
		bool contains(const T& value) const { return end() != find(value); }
	};

	/// twitchy semblance of strong(er) types
	template<typename T>
	struct strong
	{
		T _weak;

		strong(const T& weak) : _weak(weak) {}
		strong<T>& operator=(const T& weak) { _weak = weak; return *this; }

		T& operator*(void) { return _weak; }

		pal__operator_inlaid(const T&, _weak, them);
		pal__operator_inlaid(const strong<T>&, _weak, them._weak);
	};

	//
	// Phase 2
	//

	struct adler final
	{
		const static uint32_t MOD_ADLER = 65521;

		const uint32_t _a;
		const uint32_t _b;

		adler(void);

		adler(const adler&, const char);

		adler operator << (const char) const;

		adler operator << (const char*) const;

		adler operator()(const size_t, const char*) const;

		uint32_t operator()(void) const;

		operator uint32_t(void) const;

		struct sum : strong<uint32_t>
		{
			// TODO; save std::string here durring debug?

			sum(const char*);
			sum(const adler&);

			pal__operator_prototypes(const char*);
			pal__operator_prototypes(const sum);
		};
	};

	template<typename E>
	class event_manager final
	{
		void attach_(void*, void(*)(void*, const E&));
		void detach_(void*, void(*)(void*, const E&));

		std::thread _thread;
		bool _terminate;

		struct handler_s
		{
			void* _data;
			void(*_code)(void*, const E&);

			handler_s(void*, void(*)(void*, const E&));

			bool operator <(const handler_s&) const;
		};

		struct {
			std::mutex _lock;
			std::set<handler_s> _active;
		} _handler;

		struct {
			std::mutex _lock;
			pal::queue<E> _messages;
			std::condition_variable _condition;
		} _queue;

		void thread_main(void);

	public:

		/*
			#define pal_event_manager_E ??? to set type
			#define pal_event_manager_cpp ??? to set build symbol linkie thingie
			#include <pal.event_manager.inc.hpp>
		*/
		template<typename H>
		void attach(H* data, void(*code)(H*, const E&))
		{
			attach_(
				reinterpret_cast<void*>(data),
				reinterpret_cast<void(*)(void*, const E&)>(code)
			);
		}

		template<typename H>
		void detach(H* data, void(*code)(H*, const E&))
		{
			detach_(
				reinterpret_cast<void*>(data),
				reinterpret_cast<void(*)(void*, const E&)>(code)
			);
		}

		void broadcast(const E&);

		event_manager(void);
		~event_manager(void);
	};
}

#ifdef pal_cpp
#	include "pal.cpp" // can be a single-header
#endif
