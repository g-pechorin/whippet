

#include "pal.hpp"

/*
#define pal_event_manager_E ??? to set type
#define pal_event_manager_cpp ??? to set build symbol linkie thingie
#include <pal.event_manager.inc.hpp>
*/


// this is a header, yes, but it can be used to provide non-inline implementations of the C++ class for the template
// ... becasue ... demoscene! ... and curiosity


#ifndef pal_event_manager_E
// #error rtfm
#endif

#ifdef pal_event_manager_cpp

template<>
pal_event_manager_cpp pal::event_manager<pal_event_manager_E>::handler_s::handler_s(void* data, void(*code)(void*, const pal_event_manager_E&)) :
	_data(data),
	_code(code)
{
}


template<>
pal_event_manager_cpp bool pal::event_manager<pal_event_manager_E>::handler_s::operator<(const handler_s& other) const
{
	auto tc = reinterpret_cast<size_t>(_code);
	auto oc = reinterpret_cast<size_t>(other._code);

	auto td = reinterpret_cast<size_t>(_data);
	auto od = reinterpret_cast<size_t>(other._data);

	return tc == oc ? td < od : tc < oc;
}


template<>
pal_event_manager_cpp void pal::event_manager<pal_event_manager_E>::broadcast(const pal_event_manager_E& message)
{
	std::unique_lock<std::mutex> guard_queue(_queue._lock);

	_queue._messages.emplace(message);

	_queue._condition.notify_all();
}

template<>
pal_event_manager_cpp pal::event_manager<pal_event_manager_E>::event_manager(void)
{
	_terminate = false;
	_thread = std::thread([this]
	{
		this->thread_main();
	});
}

template<>
pal_event_manager_cpp void pal::event_manager<pal_event_manager_E>::thread_main(void)
{
	auto predicate = [&](void)
	{
		return !_queue._messages.empty() || _terminate;
	};

	while (true)
	{
		pal_event_manager_E message;

		{
			std::unique_lock<std::mutex> guard_queue(_queue._lock);

			_queue._condition.wait(guard_queue, predicate);

			// if there are no messages - we woke to finish
			if (_queue._messages.empty())
				break;

			message = _queue._messages.pull();
		}

		// broadcast the message
		{
			std::unique_lock<std::mutex> guard_queue(_handler._lock);

			for (auto& handler : _handler._active)
				handler._code(handler._data, message);
		}
	}
}


template<>
pal_event_manager_cpp pal::event_manager<pal_event_manager_E>::~event_manager(void)
{
	_terminate = true;
	_queue._condition.notify_all();
	_thread.join();
}

template<>
pal_event_manager_cpp void pal::event_manager<pal_event_manager_E>::attach_(void* data, void(*code)(void*, const pal_event_manager_E&))
{
	std::unique_lock<std::mutex> guard_handlers(_handler._lock);

	_handler._active.emplace(data, code);
}

#endif // pal_event_manager_cpp

#undef pal_event_manager_E
