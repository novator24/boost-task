
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/task/detail/worker_group.hpp"

#include <boost/foreach.hpp>
#include <boost/utility.hpp>

namespace boost {
namespace tasks {
namespace detail {

worker_group::~worker_group()
{ if ( ! empty() ) join_all(); }

const worker
worker_group::operator[]( std::size_t pos) const
{ return worker_[pos]; }

std::size_t
worker_group::size() const
{ return worker_.size(); }

bool
worker_group::empty() const
{ return worker_.empty(); }

const worker_group::iterator
worker_group::begin()
{ return worker_.begin(); }

const worker_group::const_iterator
worker_group::begin() const
{ return worker_.begin(); }

const worker_group::iterator
worker_group::end()
{ return worker_.end(); }

const worker_group::const_iterator
worker_group::end() const
{ return worker_.end(); }

void
worker_group::start_all()
{
	BOOST_FOREACH( worker & w, worker_)
	{ w.start(); }
}

void
worker_group::join_all()
{
	BOOST_FOREACH( worker & w, worker_)
	{
		if ( w.joinable() )
		{ w.join(); }
	}
	cont_.clear();
}

void
worker_group::interrupt_all()
{
	BOOST_FOREACH( worker & w, worker_)
	{ w.interrupt(); }
}

}}}
