
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/tasklet.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <boost/task/all.hpp>

#include "test_functions.hpp"

namespace fi = boost::tasklets;
namespace pt = boost::posix_time;
namespace tsk = boost::tasks;

// check assignment
void test_case_1()
{
	fi::scheduler<> sched;
	tsk::task< int > t( fibonacci_fn, 10);
	tsk::handle< int > h1;
	tsk::handle< int > h2(
		tsk::async( boost::move( t), sched) );
	h1 = h2;
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK_EQUAL( h1.get(), 55);
	BOOST_CHECK_EQUAL( h2.get(), 55);
}

// check swap
void test_case_2()
{
	fi::scheduler<> sched;
	tsk::task< int > t1( fibonacci_fn, 5);
	tsk::task< int > t2( fibonacci_fn, 10);
	tsk::handle< int > h1(
		tsk::async( boost::move( t1), sched) );
	tsk::handle< int > h2(
		tsk::async( boost::move( t2), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK_EQUAL( h1.get(), 5);
	BOOST_CHECK_EQUAL( h2.get(), 55);
	BOOST_CHECK_NO_THROW( h1.swap( h2) );
	BOOST_CHECK_EQUAL( h1.get(), 55);
	BOOST_CHECK_EQUAL( h2.get(), 5);
}

// check runs not in pool
void test_case_3()
{
	fi::scheduler<> sched;
	tsk::task< bool > t( runs_in_pool_fn);
	tsk::handle< bool > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK_EQUAL( h.get(), false);
}

// check runtime_error throw inside task
void test_case_4()
{
	fi::scheduler<> sched;
	tsk::task< void > t( throwing_fn);
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK_THROW( h.get(), std::runtime_error);
}

// check task_uninitialized
void test_case_5()
{
	tsk::handle< int > h;
	BOOST_CHECK_THROW( h.get(), tsk::task_uninitialized);
	BOOST_CHECK_THROW( h.wait(), tsk::task_uninitialized);
	BOOST_CHECK_THROW( h.wait_for( pt::seconds( 1) ), tsk::task_uninitialized);
	BOOST_CHECK_THROW(
		h.wait_until( boost::get_system_time() + pt::seconds( 1) ),
		tsk::task_uninitialized);
	BOOST_CHECK( ! h.is_ready() );
	BOOST_CHECK( ! h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
}

// check wait
void test_case_6()
{
	fi::scheduler<> sched;
	tsk::task< int > t( fibonacci_fn, 10);
	tsk::handle< int > h(
		tsk::async( boost::move( t), sched) );
	BOOST_CHECK( ! h.is_ready() );
	BOOST_CHECK( ! h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	h.wait();
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
	BOOST_CHECK_EQUAL( h.get(), 55);
}

// check wait_for
void test_case_7()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 1) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.wait_for( pt::seconds( 2) ) );
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
}

// check wait_for
void test_case_8()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 2) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.wait_for( pt::seconds( 1) ) );
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
}

// check wait_for
void test_case_9()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 1) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.wait_until( boost::get_system_time() + pt::seconds( 3) ) );
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
}

// check wait_for
void test_case_10()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 2) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.wait_until( boost::get_system_time() + pt::seconds( 1) ) );
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
}

// check interrupt
void test_case_11()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 3) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	h.interrupt();
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.interruption_requested() );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check interrupt_and_wait
void test_case_12()
{
	fi::scheduler<> sched;
	bool finished( false);
	tsk::task< void > t(
		interrupt_fn,
		pt::seconds( 3),
		boost::ref( finished) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	h.interrupt_and_wait();
	BOOST_CHECK( ! finished);
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.interruption_requested() );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check interrupt_and_wait_for
void test_case_13()
{
	fi::scheduler<> sched;
	bool finished( false);
	tsk::task< void > t(
		interrupt_fn,
		pt::seconds( 1),
		boost::ref( finished) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.interrupt_and_wait_for( pt::seconds( 2) ) );
	BOOST_CHECK( ! finished);
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
	BOOST_CHECK( h.interruption_requested() );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check interrupt_and_wait_for
void test_case_14()
{
	fi::scheduler<> sched;
	tsk::task< void > t( non_interrupt_fn, 2);
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.interrupt_and_wait_for( pt::seconds( 1) ) );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check interrupt_and_wait_until
void test_case_15()
{
	fi::scheduler<> sched;
	bool finished( false);
	tsk::task< void > t(
		interrupt_fn,
		pt::seconds( 1),
		boost::ref( finished) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.interrupt_and_wait_until( boost::get_system_time() + pt::seconds( 2) ) );
	BOOST_CHECK( ! finished);
	BOOST_CHECK( h.is_ready() );
	BOOST_CHECK( h.has_value() );
	BOOST_CHECK( ! h.has_exception() );
	BOOST_CHECK( h.interruption_requested() );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check interrupt_and_wait_until
void test_case_16()
{
	fi::scheduler<> sched;
	tsk::task< void > t( non_interrupt_fn, 2);
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	BOOST_CHECK( h.interrupt_and_wait_until( boost::get_system_time() + pt::seconds( 1) ) );
	BOOST_CHECK_NO_THROW( h.get() );
}

// check waitfor_all()
void test_case_17()
{
	fi::scheduler<> sched;
	std::vector< tsk::handle< int > > vec;
	for ( int i = 0; i <= 5; ++i)
	{
		tsk::task< int > t( fibonacci_fn, i);
		vec.push_back(
			tsk::async( boost::move( t), sched) );
	}
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	tsk::waitfor_all( vec.begin(), vec.end() );
	BOOST_CHECK( vec[0].is_ready() );
	BOOST_CHECK( vec[1].is_ready() );
	BOOST_CHECK( vec[2].is_ready() );
	BOOST_CHECK( vec[3].is_ready() );
	BOOST_CHECK( vec[4].is_ready() );
	BOOST_CHECK( vec[5].is_ready() );
	BOOST_CHECK_EQUAL( vec[0].get(), 0);
	BOOST_CHECK_EQUAL( vec[1].get(), 1);
	BOOST_CHECK_EQUAL( vec[2].get(), 1);
	BOOST_CHECK_EQUAL( vec[3].get(), 2);
	BOOST_CHECK_EQUAL( vec[4].get(), 3);
	BOOST_CHECK_EQUAL( vec[5].get(), 5);
}
/*
// check waitfor_any()
void test_case_18()
{
	fi::scheduler<> sched;
	tsk::task< void > t1( delay_fn, pt::seconds( 2) );
	tsk::task< void > t2( delay_fn, pt::seconds( 1) );
	tsk::handle< void > h1(
		tsk::async( boost::move( t1), sched) );
	tsk::handle< void > h2(
		tsk::async( boost::move( t2), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	tsk::waitfor_any( h1, h2);
	BOOST_CHECK( h1.is_ready() );
	BOOST_CHECK( h2.is_ready() );
}
*/
// check interrupt + wait
void test_case_19()
{
	fi::scheduler<> sched;
	tsk::task< void > t( delay_fn, pt::seconds( 3) );
	tsk::handle< void > h(
		tsk::async( boost::move( t), sched) );
	for (;;)
	{
		while ( sched.run() );
		if ( sched.empty() ) break;
	}
	h.interrupt();
	BOOST_CHECK_NO_THROW( h.wait() );
	BOOST_CHECK_NO_THROW( h.get() );
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* [])
{
	boost::unit_test::test_suite * test =
		BOOST_TEST_SUITE("Boost.Task: tasklet test suite");

	test->add( BOOST_TEST_CASE( & test_case_1) );
	test->add( BOOST_TEST_CASE( & test_case_2) );
	test->add( BOOST_TEST_CASE( & test_case_3) );
	test->add( BOOST_TEST_CASE( & test_case_4) );
	test->add( BOOST_TEST_CASE( & test_case_5) );
	test->add( BOOST_TEST_CASE( & test_case_6) );
	test->add( BOOST_TEST_CASE( & test_case_7) );
	test->add( BOOST_TEST_CASE( & test_case_8) );
	test->add( BOOST_TEST_CASE( & test_case_9) );
	test->add( BOOST_TEST_CASE( & test_case_10) );
	test->add( BOOST_TEST_CASE( & test_case_11) );
	test->add( BOOST_TEST_CASE( & test_case_12) );
	test->add( BOOST_TEST_CASE( & test_case_13) );
	test->add( BOOST_TEST_CASE( & test_case_14) );
	test->add( BOOST_TEST_CASE( & test_case_15) );
	test->add( BOOST_TEST_CASE( & test_case_16) );
	test->add( BOOST_TEST_CASE( & test_case_17) );
//	test->add( BOOST_TEST_CASE( & test_case_18) );
	test->add( BOOST_TEST_CASE( & test_case_19) );

	return test;
}
