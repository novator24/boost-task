
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_CONFIG_H
#define BOOST_TASKS_DETAIL_CONFIG_H

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#ifdef BOOST_TASK_DECL
# undef BOOST_TASK_DECL
#endif

#if defined(BOOST_HAS_DECLSPEC)
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_TASK_DYN_LINK)
#  if ! defined(BOOST_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  if defined(BOOST_TASK_SOURCE)
#   define BOOST_TASK_DECL BOOST_SYMBOL_EXPORT
#  else 
#   define BOOST_TASK_DECL BOOST_SYMBOL_IMPORT
#  endif
# endif
#endif

#if ! defined(BOOST_TASK_DECL)
# define BOOST_TASK_DECL
#endif

#if ! defined(BOOST_TASK_SOURCE) && ! defined(BOOST_ALL_NO_LIB) && ! defined(BOOST_TASK_NO_LIB)
# define BOOST_LIB_NAME boost_task
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_TASK_DYN_LINK)
#  define BOOST_DYN_LINK
# endif
# include <boost/config/auto_link.hpp>
#endif

#endif // BOOST_TASKS_DETAIL_CONFIG_H

