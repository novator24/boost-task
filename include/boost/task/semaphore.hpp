
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASK_SEMAPHORE_H
#define BOOST_TASK_SEMAPHORE_H

#include <boost/task/detail/config.hpp>

extern "C"
{
# if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#include <Windows.h>
# else
#include <sys/sem.h>
# endif
}

#include <boost/utility.hpp>

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

class BOOST_TASK_DECL semaphore : private boost::noncopyable
{
private:
# if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
	HANDLE	handle_;
# else
	int		handle_;
# endif
public:
	semaphore( int);

	~semaphore();

	void post( int = 1);

	void wait();

	bool try_wait();
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#endif // BOOST_TASK_SEMAPHORE_H
