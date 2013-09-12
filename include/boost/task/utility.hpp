
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)txt)

#ifndef BOOST_TASKS_UTILITY_H
#define BOOST_TASKS_UTILITY_H

#include <boost/assert.hpp>
#include <boost/thread.hpp>

#include <boost/task/detail/worker.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace this_task {

inline
bool runs_in_pool()
{ return tasks::detail::worker::instance() != 0; }

inline
worker::id worker_id()
{
	BOOST_ASSERT( runs_in_pool() );

	return tasks::detail::worker::instance()->get_id();
}

inline
void yield()
{
	BOOST_ASSERT( runs_in_pool() );

	tasks::detail::worker::instance()->yield();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_UTILITY_H
