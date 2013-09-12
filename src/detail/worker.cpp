
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/task/detail/worker.hpp"

namespace boost {
namespace tasks {
namespace detail {

thread_specific_ptr< worker > worker::tss_;

}}}
