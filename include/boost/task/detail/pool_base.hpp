
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_POOL_BASE_H
#define BOOST_TASKS_DETAIL_POOL_BASE_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/detail/move.hpp>
#include <boost/thread/future.hpp>

#include <boost/task/callable.hpp>
#include <boost/task/context.hpp>
#include <boost/task/detail/bind_processor.hpp>
#include <boost/task/detail/worker_group.hpp>
#include <boost/task/detail/worker.hpp>
#include <boost/task/exceptions.hpp>
#include <boost/task/fast_semaphore.hpp>
#include <boost/task/handle.hpp>
#include <boost/task/poolsize.hpp>
#include <boost/task/spin/future.hpp>
#include <boost/task/stacksize.hpp>
#include <boost/task/task.hpp>
#include <boost/task/utility.hpp>
#include <boost/task/watermark.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename Queue >
class pool_base
{
private:
	template< typename T, typename Z >
	friend class worker_object;

	typedef Queue							queue_type;
	typedef typename queue_type::value_type	value_type;

	enum state
	{
		ACTIVE = 0,
		DEACTIVE	
	};

	atomic< unsigned int >	use_count_;
	fast_semaphore			fsem_;
	worker_group			wg_;
	shared_mutex			mtx_wg_;
	atomic< state >			state_;
	queue_type				queue_;
	atomic< bool >			shtdwn_;
	atomic< bool >			shtdwn_now_;

	void create_worker_(
		poolsize const& psize,
		stacksize const& stack_size)
	{ wg_.add( worker::create( * this, psize) ); }

	std::size_t size_() const
	{ return wg_.size(); }

	bool deactivated_() const
	{ return DEACTIVE == state_.load(); }

	bool deactivate_()
	{ return ACTIVE == state_.exchange( DEACTIVE); }

    friend inline void intrusive_ptr_add_ref( pool_base * p)
    { p->use_count_.fetch_add( 1, memory_order_relaxed); }

    friend inline void intrusive_ptr_release( pool_base * p)
    {
        if ( 1 == p->use_count_.fetch_sub( 1, memory_order_release) )
        {
            atomic_thread_fence( memory_order_acquire);
            delete p;
        }
    }

public:
	pool_base(
			poolsize const& psize,
			stacksize const& stack_size) :
		use_count_( 0),
		fsem_( 0),
		wg_( * this, psize),
		mtx_wg_(),
		state_( ACTIVE),
		queue_( fsem_),
		shtdwn_( false),
		shtdwn_now_( false)
	{ wg_.start_all();	}

	pool_base(
			poolsize const& psize,
			high_watermark const& hwm,
			low_watermark const& lwm,
			stacksize const& stack_size) :
		use_count_( 0),
		fsem_( 0),
		wg_( * this, psize),
		mtx_wg_(),
		state_( ACTIVE),
		queue_( fsem_, hwm, lwm),
		shtdwn_( false),
		shtdwn_now_( false)
	{ wg_.start_all();	}

	~pool_base()
	{ shutdown(); }

	void interrupt_all_worker()
	{
		if ( deactivated_() ) return;

		shared_lock< shared_mutex > lk( mtx_wg_);
		wg_.interrupt_all();
	}

	void shutdown()
	{
		if ( deactivated_() || ! deactivate_() ) return;

		queue_.deactivate();
		fsem_.deactivate();
		shared_lock< shared_mutex > lk( mtx_wg_);
		shtdwn_.store( true);
		wg_.join_all();
	}

	void shutdown_now()
	{
		if ( deactivated_() || ! deactivate_() ) return;

		queue_.deactivate();
		fsem_.deactivate();
		shared_lock< shared_mutex > lk( mtx_wg_);
		shtdwn_now_.store( true);
		wg_.interrupt_all();
		wg_.join_all();
	}

	std::size_t size() const
	{
		shared_lock< shared_mutex > lk( mtx_wg_);
		return size_();
	}

	bool closed() const
	{ return deactivated_(); }

	std::size_t upper_bound() const
	{ return queue_.upper_bound(); }

	void upper_bound( high_watermark const& hwm)
	{ queue_.upper_bound( hwm); }

	std::size_t lower_bound() const
	{ return queue_.lower_bound(); }

	void lower_bound( low_watermark const lwm)
	{ queue_.lower_bound( lwm); }

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( Fn fn)
	{
        typedef typename result_of< Fn() >::result_type R;

		if ( deactivated_() )
			throw task_rejected("pool is closed");

		if ( this_task::runs_in_pool() )
		{
			detail::promise< R > prom;
			detail::shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put( callable( fn, boost::move( prom), ctx) );
			return t;
		}
		else
		{
			promise< R > prom;
			shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put( callable( fn, boost::move( prom), ctx) );
			return t;
		}
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn)
	{
        typedef typename result_of< Fn() >::result_type R;

		if ( deactivated_() )
			throw task_rejected("pool is closed");

		if ( this_task::runs_in_pool() )
		{
			detail::promise< R > prom;
			detail::shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put( callable( boost::move( fn), boost::move( prom), ctx) );
			return t;
		}
		else
		{
			promise< R > prom;
			shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put( callable( boost::move( fn), boost::move( prom), ctx) );
			return t;
		}
	}

	template< typename Fn, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( Fn fn, Attr const& attr)
	{
		if ( deactivated_() )
			throw task_rejected("pool is closed");

		if ( this_task::runs_in_pool() )
		{
			detail::promise< R > prom;
			detail::shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put(
				value_type(
					callable( fn, boost::move( prom), ctx),
					attr) );
			return t;
		}
		else
		{
			promise< R > prom;
			shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put(
				value_type(
					callable( fn, boost::move( prom), ctx),
					attr) );
			return t;
		}
	}

	template< typename Fn, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn, Attr const& attr)
	{
		if ( deactivated_() )
			throw task_rejected("pool is closed");

		if ( this_task::runs_in_pool() )
		{
			detail::promise< R > prom;
			detail::shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put(
				value_type(
					callable( boost::move( fn), boost::move( prom), ctx),
					attr) );
			return t;
		}
		else
		{
			promise< R > prom;
			shared_future< R > f( prom.get_future() );
			context ctx;
			task< R > t( f, ctx);
			queue_.put(
				value_type(
					callable( boost::move( fn), boost::move( prom), ctx),
					attr) );
			return t;
		}
	}
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_POOL_BASE_H

