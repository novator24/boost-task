
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//  parts are based on boost.future

#ifndef BOOST_TASKS_TASK_H
#define BOOST_TASKS_TASK_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/task/context.hpp>
#include <boost/task/detail/future.hpp>
#include <boost/task/detail/task_base.hpp>
#include <boost/task/detail/task_object.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

template< typename R >
class task
{
private:
    task_base< R >::ptr_t    impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( task);

	task( detail::unique_future< R > const& fut, context const& ctx)
        impl_(
            new detail::task_object< R, detail::unique_future< R >(
                boost::move( fut), ctx) )
	{}

	task( unique_future< R > const& fut, context const& ctx)
        impl_(
            new detail::task_object< R, unique_future< R >(
                boost::move( fut), ctx) )
	{}

public:
    typedef void ( * unspecified_bool_type)( task ***);

    static void unspecified_bool( task ***) {}

	task() :
		impl_()
	{}

    task( BOOST_RV_REF( task) other) :
        impl_()
    { impl_.swap( other.impl_); }

    task & operator=( BOOST_RV_REF( task) other)
    {
        task tmp( other);
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return impl_ : unspecified_bool : 0; }

    bool operator!() const
    { return ! impl_; }

    void swap( task & other)
    { impl_.swap( other.impl_); }

    bool operator==( task const& other) const
    { return impl_ == other.impl_; }

    bool operator!=( task const& other) const
    { return impl_ != other.impl_; }

	bool interruption_requested() const
	{
        BOOST_ASSERT( impl_);
        return impl_->interruption_requested();
    }

	void interrupt()
	{
        BOOST_ASSERT( impl_);
        impl_->interrupt();
    }

	void interrupt_and_wait()
	{
        BOOST_ASSERT( impl_);
        impl_->interrupt();
        impl_->wait();
	}

	bool interrupt_and_wait_until( system_time const& abs_time)
	{
        BOOST_ASSERT( impl_);
        impl_->interrupt();
		return impl_->wait_until( abs_time);
	}

    template< typename TimeDuration >
	bool interrupt_and_wait_for( TimeDuration const& dt)
	{ return interrupt_and_wait_until( get_system_time() + dt); }

	void wait()
	{
        BOOST_ASSERT( impl_);
		impl_->wait();
	}

	bool wait_until( system_time const& abs_time)
	{
        BOOST_ASSERT( impl_);
		return impl_->wait_until( abs_time);
	}

    template< typename TimeDuration >
	bool wait_for( TimeDuration const& dt)
	{ return wait_until( get_system_time() + dt); }
};

inline
void swap( task & l, task & r)
{ return l.swap( r); }

template< typename T >
struct is_task_type;

template< typename T >
struct is_task_type
{ BOOST_STATIC_CONSTANT( bool, value = false); };

template< typename T >
struct is_task_type< task< T > >
{ BOOST_STATIC_CONSTANT( bool, value = true); };

template< typename Iterator >
typename disable_if< is_task_type< Iterator >, void >::type waitfor_all(
	Iterator begin, Iterator end)
{
	for ( Iterator i = begin; i != end; ++i)
		i->future().wait();
}

template< typename R1, typename R2 >
void waitfor_all( task< R1 > const& t1, task< R2 > const& t2)
{
	t1.future().wait();
	t2.future().wait();
}

template< typename R1, typename R2, typename R3 >
void waitfor_all( task< R1 > const& t1, task< R2 > const& t2, task< R3 > const& t3)
{
	t1.future().wait();
	t2.future().wait();
	t3.future().wait();
}

template< typename R1, typename R2, typename R3, typename R4 >
void waitfor_all(
	task< R1 > const& t1, task< R2 > const& t2, task< R3 > const& t3, task< R4 > const& t4)
{
	t1.future().wait();
	t2.future().wait();
	t3.future().wait();
	t4.future().wait();
}

template< typename R1, typename R2, typename R3, typename R4, typename R5 >
void waitfor_all(
	task< R1 > const& t1, task< R2 > const& t2, task< R3 > const& t3, task< R4 > const& t4,
	task< R5 > const& t5)
{
	t1.future().wait();
	t2.future().wait();
	t3.future().wait();
	t4.future().wait();
	t5.future().wait();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_TASK_H
