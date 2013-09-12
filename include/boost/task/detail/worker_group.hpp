
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_WORKER_GROUP_H
#define BOOST_TASKS_DETAIL_WORKER_GROUP_H

#include <cstddef>
#include <vector>

#include <boost/config.hpp>
#include <boost/thread.hpp>

#include <boost/task/detail/config.hpp>
#include <boost/task/detail/worker.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

class BOOST_TASK_DECL worker_group
{
private:
	typedef std::vector< worker::ptr_t >	container_t;

	container_t	worker_;

public:
	typedef container_t::iterator		iterator;
	typedef container_t::const_iterator	const_iterator;

	template< typename Pool >
	worker_group( Pool const& pool, std::size_t size) :
		worker_()
	{
		for ( int i = 0; i < size; ++i)
			worker_.push_back( worker::create( pool) );
	}

	~worker_group();

	std::size_t size() const;

	bool empty() const;

	const worker::ptr_t operator[]( std::size_t) const;

	const iterator begin();
	const const_iterator begin() const;

	const iterator end();
	const const_iterator end() const;

	void start_all();

	void join_all();

	void interrupt_all();
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_WORKER_GROUP_H

