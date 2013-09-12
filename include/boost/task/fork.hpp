
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_FORK_H
#define BOOST_TASKS_FORK_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/result_of.hpp>

#include <boost/task/callable.hpp>
#include <boost/task/context.hpp>
#include <boost/task/detail/future.hpp>
#include <boost/task/detail/worker.hpp>
#include <boost/task/task.hpp>
#include <boost/task/utility.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

template< typename Fn >
task< typename result_of< Fn >::result_of >
fork( Fn fn)
{
    typedef typename result_of< Fn() >::result_type R;

    BOOST_ASSERT( this_task::runs_in_pool() );

    detail::promise< R > prom;
    detail::unique_future< R > f( prom.get_future() );
    context ctx;
    task< R > t( f, ctx);
    detail::worker::instance()->put(
        callable( fn, boost::move( prom), ctx) );
    return t;
}

template< typename Fn >
task< typename result_of< Fn >::result_of >
fork( BOOST_RV_REF( Fn) fn)
{
    typedef typename result_of< Fn() >::result_type R;

    BOOST_ASSERT( this_task::runs_in_pool() );

    detail::promise< R > prom;
    detail::unique_future< R > f( prom.get_future() );
    context ctx;
    task< R > t( f, ctx);
    detail::worker::instance()->put(
        callable( boost::move( fn), boost::move( prom), ctx) );
    return t;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_FORK_H
