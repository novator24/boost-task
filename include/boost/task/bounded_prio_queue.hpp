
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_BOUNDED_PRIO_QUEUE_H
#define BOOST_TASKS_BOUNDED_PRIO_QUEUE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <queue>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/exception/all.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <boost/task/detail/meta.hpp>
#include <boost/task/exceptions.hpp>
#include <boost/task/fast_semaphore.hpp>
#include <boost/task/watermark.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template<
    typename T,
	typename Attr,
	typename Comp = std::less< Attr >
>
class bounded_prio_queue_base
{
public:
	typedef detail::has_attribute	attribute_tag_type;
	typedef Attr					attribute_type;
	typedef intrusive_ptr<
		bounded_prio_queue_base< T, Attr, Comp >
	>								ptr_type;

	struct value_type
	{
		T       		t;
		attribute_type	attr;

		value_type(
				T const& t_,
				attribute_type const& attr_) :
			t( t_), attr( attr_)
		{ BOOST_ASSERT( ! t.empty() ); }

		void swap( value_type & other)
		{
			t.swap( other.t);
			std::swap( attr, other.attr);
		}
	};

private:
	struct compare : public std::binary_function< value_type, value_type, bool >
	{
		bool operator()( value_type const& va1, value_type const& va2)
		{ return Comp()( va1.attr, va2.attr); }
	};

	typedef std::priority_queue<
		value_type,
		std::deque< value_type >,
		compare
	>								queue_type;

	enum state
	{
		ACTIVE = 0,
		DEACTIVE
	};

	std::size_t				use_count_;
	atomic< state >			state_;
	queue_type				queue_;
	mutable shared_mutex	mtx_;
	condition				not_full_cond_;
	std::size_t				hwm_;
	std::size_t				lwm_;
	fast_semaphore		*	fsem_;

	bool active_() const
	{ return ACTIVE == state_.load(); }

	void deactivate_()
	{ state_.store( DEACTIVE); }

	bool empty_() const
	{ return queue_.empty(); }

	bool full_() const
	{ return size_() >= hwm_; }

	std::size_t size_() const
	{ return queue_.size(); }

	void put_(
		value_type const& va,
		unique_lock< shared_mutex > & lk)
	{
		if ( full_() )
		{
			not_full_cond_.wait(
				lk,
				bind(
					& bounded_prio_queue::producers_activate_,
					this) );
		}
		if ( ! active_() )
			BOOST_THROW_EXCEPTION( task_rejected("queue is not active") );
		queue_.push( va);
		if ( fsem_) fsem_->post();
	}

	template< typename TimeDuration >
	void put_(
		value_type const& va,
		TimeDuration const& rel_time,
		unique_lock< shared_mutex > & lk)
	{
		if ( full_() )
		{
			if ( ! not_full_cond_.timed_wait(
				lk,
				rel_time,
				bind(
					& bounded_prio_queue::producers_activate_,
					this) ) )
				BOOST_THROW_EXCEPTION( task_rejected("timed out") );
		}
		if ( ! active_() )
			BOOST_THROW_EXCEPTION( task_rejected("queue is not active") );
		queue_.push( va);
		if ( fsme_) fsem_->post();
	}

	bool try_take_( T & t)
	{
		if ( empty_() ) return false;
		t = queue_.top().t;
		queue_.pop();
		if ( size_() <= lwm_)
		{
			if ( lwm_ == hwm_)
				not_full_cond_.notify_one();
			else
				// more than one producer could be waiting
				// in order to submit an task
				not_full_cond_.notify_all();
		}
		return true;
	}

	bool producers_activate_() const
	{ return ! active_() || ! full_(); }

public:
	bounded_prio_queue_base(
			high_watermark const& hwm,
			low_watermark const& lwm) :
		state_( ACTIVE),
		queue_(),
		mtx_(),
		not_full_cond_(),
		hwm_( hwm),
		lwm_( lwm),
		fsem_( 0)
	{
		if ( lwm_ > hwm_ )
			BOOST_THROW_EXCEPTION( invalid_watermark() );
	}

	bounded_prio_queue_base(
			high_watermark const& hwm,
			low_watermark const& lwm,
			fast_semaphore & fsem) :
		state_( ACTIVE),
		queue_(),
		mtx_(),
		not_full_cond_(),
		hwm_( hwm),
		lwm_( lwm),
		fsem_( & fsem)
	{
		if ( lwm_ > hwm_ )
			BOOST_THROW_EXCEPTION( invalid_watermark() );
	}

	bool active() const
	{ return active_(); }

	void deactivate()
	{
		unique_lock< shared_mutex > lk( mtx_);
		deactivate_();
		not_full_cond_.notify_all();
	}

	bool empty() const
	{
		shared_lock< shared_mutex > lk( mtx_);
		return empty_();
	}

	std::size_t upper_bound() const
	{
		shared_lock< shared_mutex > lk( mtx_);
		return hwm_;
	}

	std::size_t lower_bound() const
	{
		shared_lock< shared_mutex > lk( mtx_);
		return lwm_;
	}

	void put( value_type const& va)
	{
		unique_lock< shared_mutex > lk( mtx_);
		put_( va, lk);
	}

	template< typename TimeDuration >
	void put(
		value_type const& va,
		TimeDuration const& rel_time)
	{
		unique_lock< shared_mutex > lk( mtx_);
		put_( va, rel_time, lk);
	}

	bool try_take( T & t)
	{
		unique_lock< shared_mutex > lk( mtx_);
		return try_take_( t);
	}
};

}

template<
    typename T,
	typename Attr,
	typename Comp = std::less< Attr >
>
class bounded_prio_queue
{
private:
    typedef bounded_prio_queue< T, Attr, Comp >                             queue_type;

    typename detail::bounded_prio_queue_base< T, Attr, Comp >::ptr_type     impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( queue_type);

public:
	typedef typename detail::bounded_prio_queue_base< T, Attr, Comp >::attribute_tag_type	attribute_tag_type;
	typedef typename detail::bounded_prio_queue_base< T, Attr, Comp >::value_type			value_type;
    typedef void ( * unspecified_bool_type)( bounded_prio_queue< T, Attr, Comp > ***);

    static void unspecified_bool( bounded_prio_queue< T, Attr, Comp > ***) {}

	bounded_prio_queue(
			high_watermark const& hwm,
			low_watermark const& lwm) :
        impl_(
	        new detail::bounded_prio_queue_base( hwm, lwm) )
	{}

	bounded_prio_queue(
			high_watermark const& hwm,
			low_watermark const& lwm,
			fast_semaphore & fsem) :
        impl_(
	        new detail::bounded_prio_queue_base( hwm, lwm, fsem) )
	{}

    bounded_prio_queue( BOOST_RV_REF( queue_type) other) :
       impl_()
    { swap( other); } 

    bounded_prio_queue & operator=( BOOST_RV_REF( queue_type) other)
    {
        if ( this == other) return * this;
        bounded_prio_queue tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return impl_ ? unspecified_bool : 0; }

    bool operator!() const
    { return ! impl_; }

    void swap( bounded_prio_queue & other)
    { impl_.swap( other.impl_); }

	bool active() const
	{
		BOOST_ASSERT( impl_);
        return impl_->active();
    }

	void deactivate()
	{
		BOOST_ASSERT( impl_);
        impl_->deactivate();
	}

	bool empty() const
	{
		BOOST_ASSERT( impl_);
		return impl_->empty();
	}

	std::size_t upper_bound() const
	{
		BOOST_ASSERT( impl_);
		return impl_->upper_bound();
	}

	std::size_t lower_bound() const
	{
		BOOST_ASSERT( impl_);
		return impl_->lower_bound();
	}

	void put( value_type const& va)
	{
		BOOST_ASSERT( impl_);
		impl_->put( va, lk);
	}

	template< typename TimeDuration >
	void put(
		value_type const& va,
		TimeDuration const& rel_time)
	{
		BOOST_ASSERT( impl_);
		impl_->put( va, rel_time, lk);
	}

	bool try_take( T & t)
	{
		BOOST_ASSERT( impl_);
		return impl_->try_take( t);
	}
};

template< typename T, typename Attr, typename Comp >
void swap( bounded_prio_queue< T, Attr, Comp > & l, bounded_prio_queue< T, Attr, Comp > & r)
{ return l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_BOUNDED_PRIO_QUEUE_H
