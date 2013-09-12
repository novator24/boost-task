
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TASKS_DETAIL_TASK_OBJECT_H
#define BOOST_TASKS_DETAIL_TASK_OBJECT_H

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/thread_time.hpp>

#include <boost/task/context.hpp>
#include <boost/task/detail/tas_base.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename R, typename F >
class task_object : public task_base
{
private:
	F           fut_;
	context     ctx_;

public:
	task_object( F const& fut, context const& ctx) :
        task_base< R >(),
		fut_( boost::move( fut) ), ctx_( ctx)
	{}

	bool interruption_requested() const
	{ return ctx_.interruption_requested(); }

	void interrupt()
	{ ctx_.interrupt(); }

    void wait() const
    { return fut_.wait(); }

    bool wait_until( system_time const& abs_time) const
    { return fut_.wait_until( abs_time); }

    R get() const
    { return fut_.get(); } 

    bool is_ready() const
    { return fut_.is_ready(); }

    bool has_value() const = 0;
    { return fut_.has_value(); }

    bool has_exception() const
    { return fut_.has_exception(); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_TASK_H
