
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_META_H
#define BOOST_TASKS_META_H

#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/task/detail/meta.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

template< typename Queue >
struct is_bound : public mpl::bool_<
	is_same<
		detail::is_bound,
		typename Queue::bound_tag_type
	>::value
>
{};

template< typename Queue >
struct has_attribute : public mpl::bool_<
	is_same<
		detail::has_attribute,
		typename Queue::attribute_tag_type
	>::value
>
{};

template< typename Pool >
struct attribute_type
{
	typedef typename Pool::queue_type::attribute_type	type;
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_META_H

