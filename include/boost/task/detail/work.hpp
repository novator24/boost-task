
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_WORK_H
#define BOOST_TASKS_DETAIL_WORK_H

#include <algortihm>

#include <boost/config.hpp>
#include <boost/context/all.hpp>
#include <boost/move/move.hpp>

#include <boost/task/callable.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

class work
{
private:
	ctx::fcontext_t	caller_;
	ctx::fcontext_t	callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( work);

public:
    typedef void ( * unspecified_bool_type)( work ***);

    static void unspecified_bool( work ***) {}

	work() :
		caller_(), callee_()
	{}

	//FIXME: make stacksize and -unwinding customizable
	work( callable const& ca) :
		ctx_( ca,
			  contexts::default_stacksize(),
			  contexts::stack_unwind,
			  contexts::return_to_caller)
	{}

    work( BOOST_RV_REF( work) other) :
        ctx_()
    { swap( other); }

    work & operator=( BOOST_RV_REF( work) other)
    {
        if ( this == & other) return * this;
        work tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return ctx_ ? unspecified_bool : 0; }

    bool operator!() const
    { return ! ctx_; }

    void swap( work & other)
    {
		std::swap( started_, other.started_);
		ctx_.swap( other.ctx_);
	}

	void run()
	{
		if ( ! ctx_.is_started() )
			ctx_.start();
		else
			ctx_.resume();	
	}

	void yield()
	{ ctx_.suspend(); }

	bool is_started() const
	{ return ctx_.is_started(); }

	bool is_complete() const
	{ return ctx_.is_complete(); }
};

inline
void swap( work & l, work & r)
{ l.swap( r); }

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_WOR_H
