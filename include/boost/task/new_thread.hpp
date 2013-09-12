
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_NEW_THREAD_H
#define BOOST_TASKS_NEW_THREAD_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/result_of.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <boost/task/callable.hpp>
#include <boost/task/context.hpp>
#include <boost/task/task.hpp>
#include <boost/task/utility.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

struct joiner
{
	void operator()( thread * thrd)
	{
		try
		{
			BOOST_ASSERT( thrd);
			BOOST_ASSERT( thrd->joinable() );
			thrd->join();
		}
		catch (...)
		{}
		delete thrd;
	}
};

}

struct new_thread
{
	template< typename Fn >
	task< typename result_of< Fn() >::result_type >
    operator()( Fn fn)
	{
        typedef typename result_of< Fn() >::result_type  R;

		BOOST_ASSERT( ! this_task::runs_in_pool() );

        promise< R > prom;
        unique_future< R > f( prom.get_future() );
        context ctx1, ctx2;
        task< R > t( f, ctx1);
        callable ca( fn, boost::move( prom), ctx2);
        shared_ptr< thread > thrd( new thread( ca), detail::joiner() );
        ctx1.reset( thrd);
        return t;
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type >
    operator()( BOOST_RV_REF( Fn) fn)
	{
        typedef typename result_of< Fn() >::result_type  R;

		BOOST_ASSERT( ! this_task::runs_in_pool() );

        promise< R > prom;
        unique_future< R > f( prom.get_future() );
        context ctx1, ctx2;
        task< R > t( f, ctx1);
        callable ca( boost::move( fn), boost::move( prom), ctx2);
        shared_ptr< thread > thrd( new thread( ca), detail::joiner() );
        ctx1.reset( thrd);
        return t;
	}
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_NEW_THREAD_H
