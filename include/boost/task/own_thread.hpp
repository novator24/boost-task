
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_OWN_THREAD_H
#define BOOST_TASKS_OWN_THREAD_H

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/result_of.hpp>
#include <boost/thread/future.hpp>

#include <boost/task/callable.hpp>
#include <boost/task/context.hpp>
#include <boost/task/task.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

struct own_thread
{
	template< typename Fn >
	task< typename result_of< Fn() >::result_type >
    operator()( Fn fn)
	{
        typedef typename result_of< Fn() >::result_type R;
        promise< R > prom;
        unique_future< R > f( prom.get_future() );
        context ctx;
        task< R > t( f, ctx);
        callable ca( fn, boost::move( prom), ctx);
        ca();
        return t;
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type >
    operator()( BOOST_RV_REF( Fn) fn)
	{
        typedef typename result_of< Fn() >::result_type R;
        promise< R > prom;
        unique_future< R > f( prom.get_future() );
        context ctx;
        task< R > t( f, ctx);
        callable ca( boost::move( fn), boost::move( prom), ctx);
        ca();
        return t;
	}
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_OWN_THREAD_H
