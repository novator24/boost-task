
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_POOLSIZE_H
#define BOOST_TASKS_POOLSIZE_H

#include <cstddef>

#include <boost/task/detail/config.hpp>

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

class BOOST_TASK_DECL poolsize
{
private:
	std::size_t	value_;

public:
	explicit poolsize( std::size_t value);

	operator std::size_t () const;
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#endif // BOOST_TASKS_POOLSIZE_H
