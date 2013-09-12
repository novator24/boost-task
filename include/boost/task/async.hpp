
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_ASYNC_H
#define BOOST_TASKS_ASYNC_H

#include <cstddef>

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/result_of.hpp>

#include <boost/task/new_thread.hpp>
#include <boost/task/own_thread.hpp>
#include <boost/task/static_pool.hpp>
#include <boost/task/task.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( Fn fn)
{ return new_thread()( fn); }

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( BOOST_RV_REF( Fn) fn)
{ return new_thread()( boost::move( fn) ); }

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( Fn fn, own_thread ot)
{ return ot( fn); }

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( BOOST_RV_REF( Fn) fn, own_thread ot)
{ return ot( boost::move( fn) ); }

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( Fn fn, new_thread nt)
{ return nt( fn); }

template< typename Fn >
task< typename result_of< Fn() >::result_type >
async( BOOST_RV_REF( Fn) fn, new_thread nt)
{ return nt( boost::move( fn) ); }

template< typename Fn, typename Queue >
task< typename result_of< Fn() >::result_type >
async( Fn fn, static_pool< Queue > & pool)
{ return pool.submit( fn); }

template< typename Fn, typename Queue >
task< typename result_of< Fn() >::result_type >
async( BOOST_RV_REF( Fn) fn, static_pool< Queue > & pool)
{ return pool.submit( boost::move( fn) ); }

template< typename Fn, typename Attr, typename Queue >
task< typename result_of< Fn() >::result_type >
async( Fn fn, Attr attr, static_pool< Queue > & pool)
{ return pool.submit( fn, attr); }

template< typename Fn, typename Attr, typename Queue >
task< typename result_of< Fn() >::result_type >
async( BOOST_RV_REF( Fn) fn, Attr attr, static_pool< Queue > & pool)
{ return pool.submit( boost::move( fn), attr); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_ASYNC_H
