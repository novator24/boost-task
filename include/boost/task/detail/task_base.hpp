
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TASKS_DETAIL_TASK_BASE_H
#define BOOST_TASKS_DETAIL_TASK_BASE_H

#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/utility.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename R >
struct task_base : private noncopyable
{
    typedef intrusive_ptr< task_base >  ptr_t;

    atomic< unsigned int >    use_count;

    task_base() :
        use_count( 0)
    {}

	virtual bool interruption_requested() const = 0;

	virtual void interrupt() = 0;

    virtual void wait() const = 0;

    virtual bool wait_until( system_time const&) const = 0;

    virtual R get() const = 0;

    virtual bool is_ready() const = 0;

    virtual bool has_value() const = 0;

    virtual bool has_exception() const = 0;

	inline friend void intrusive_ptr_add_ref( task_base * p)
	{ p->use_count.fetch_add( 1, memory_order_relaxed); }
	
	inline friend void intrusive_ptr_release( task_base * p)
	{
		if ( 1 == p->use_count.fetch_sub( 1, memory_order_release) )
		{
			atomic_thread_fence( memory_order_acquire);
			delete p;
		}
	}
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_TASK_BASE_H
