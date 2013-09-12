
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_BOUNDED_FIFO_H
#define BOOST_TASKS_BOUNDED_FIFO_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/exception/all.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

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

template< typename T >
class bounded_fifo_base
{
public:
	typedef is_bound        	                bound_tag_type;
	typedef has_no_attribute	                attribute_tag_type;
	typedef T  				                	value_type;
    typedef intrusive_ptr< bouded_fifo_base>    ptr_type;

private:
	struct node
	{
		typedef intrusive_ptr< node >	ptr_type;

        std::size_t use_count;
		value_type	va;
		ptr_type	next;

        node() :
            use_count( 0), va(), next()
        {}

        friend
        inline void intrusive_ptr_add_ref( node * p)
        { ++p->use_count; }

        friend
        inline void intrusive_ptr_release( node * p)
        { if ( 0 == --p->use_count) delete p; }
	};

	enum state
	{
		ACTIVE = 0,
		DEACTIVE
	};

    std::size_t             use_count_;
	atomic< state >			state_;
	atomic< std::size_t >	count_;
	node::ptr_type		    head_;
	mutable mutex			head_mtx_;
	node::ptr_type		    tail_;
	mutable mutex			tail_mtx_;
	condition				not_full_cond_;
	std::size_t				hwm_;
	std::size_t				lwm_;
	fast_semaphore		*	fsem_;

	bool active_() const
	{ return ACTIVE == state_.load(); }

	void deactivate_()
	{ state_.store( DEACTIVE); }

	std::size_t size_() const
	{ return count_.load(); }

	bool empty_() const
	{ return head_ == get_tail_(); }

	bool full_() const
	{ return size_() >= hwm_; }

	node::ptr_type get_tail_() const
	{
		lock_guard< mutex > lk( tail_mtx_);	
		node::ptr_type tmp = tail_;
		return tmp;
	}

	node::ptr_type pop_head_()
	{
		node::ptr_type old_head = head_;
		head_ = old_head->next;
		count_.fetch_sub( 1);
		return old_head;
	}

public:
	bounded_fifo_base(
			high_watermark const& hwm,
			low_watermark const& lwm) :
        use_count_( 0),
		state_( ACTIVE),
		count_( 0),
		head_( new node),
		head_mtx_(),
		tail_( head_),
		tail_mtx_(),
		not_full_cond_(),
		hwm_( hwm),
		lwm_( lwm),
		fsem_( 0)
	{}

	bounded_fifo_base(
			high_watermark const& hwm,
			low_watermark const& lwm,
			fast_semaphore & fsem) :
        use_count_( 0),
		state_( ACTIVE),
		count_( 0),
		head_( new node),
		head_mtx_(),
		tail_( head_),
		tail_mtx_(),
		not_full_cond_(),
		hwm_( hwm),
		lwm_( lwm),
		fsem_( & fsem)
	{}

	std::size_t upper_bound() const
	{ return hwm_; }

	std::size_t lower_bound() const
	{ return lwm_; }

	bool active() const
	{ return active_(); }

	void deactivate()
	{
		unique_lock< mutex > lk( head_mtx_);
		deactivate_();
		not_full_cond_.notify_all();
	}

	bool empty() const
	{
		unique_lock< mutex > lk( head_mtx_);
		return empty_();
	}

	void put( value_type const& va)
	{
		node::ptr_type new_node( new node);
		{
			unique_lock< mutex > lk( tail_mtx_);

			if ( full_() )
			{
				while ( active_() && full_() )
					not_full_cond_.wait( lk);
			}
			if ( ! active_() )
				BOOST_THROW_EXCPETION( task_rejected("queue is not active") );

			tail_->va = va;
			tail_->next = new_node;
			tail_ = new_node;
			count_.fetch_add( 1);
		}
		if ( fsem_) fsem_->post();
	}

	template< typename TimeDuration >
	void put(
		value_type const& va,
		TimeDuration const& rel_time)
	{
		node::ptr_type new_node( new node);
		{
			unique_lock< mutex > lk( tail_mtx_);

			if ( full_() )
			{
				while ( active_() && full_() )
					if ( ! not_full_cond_.wait( lk, rel_time) )
						BOOST_THROW_EXCEPTION( task_rejected("timed out") );
			}
			if ( ! active_() )
				BOOST_THROW_EXCEPTION( task_rejected("queue is not active") );

			tail_->va = va;
			tail_->next = new_node;
			tail_ = new_node;
			count_.fetch_add( 1);
		}
		if ( fsem_) fsem_->post();
	}

	bool try_take( value_type & va)
	{
		unique_lock< mutex > lk( head_mtx_);
		if ( empty_() ) return false;
		va.swap( head_->va);
		pop_head_();
		bool valid = ! va.empty();
		if ( valid && size_() <= lwm_)
		{
			if ( lwm_ == hwm_)
				not_full_cond_.notify_one();
			else
				// more than one producer could be waiting
				// in order to submit an task
				not_full_cond_.notify_all();
		}
		return valid;
	}

    friend
    inline void intrusive_ptr_add_ref( bounded_fifo_base< T > * p)
    { p->use_count_.fetch_add( 1, memory_order_relaxed); }

    friend
    inline void intrusive_ptr_release( bounded_fifo_base< T > * p)
    {
        if ( p->use_count_.fetch_sub( 1, memory_order_release) == 1)
        {
            atomic_thread_fence( memory_order_acquire);
            delete p;
        }
    }
};

}

template< typename T >
class bounded_fifo
{
private:
    typename detail::bounded_fifo_base< T >::ptr_type   impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( bounded_fifo);

public:
	typedef typename detail::bounded_fifo_base< T >::attribute_tag_type	attribute_tag_type;
	typedef typename detail::bounded_fifo_base< T >::value_type			value_type;
    typedef void ( * unspecified_bool_type)( bounded_fifo< T > ***);

    static void unspecified_bool( bounded_fifo< T > ***) {}

	bounded_fifo(
			high_watermark const& hwm,
			low_watermark const& lwm) :
		impl_( new detail::bounded_fifo_base< T >( hwm, lwm) )
	{}

	bounded_fifo(
			high_watermark const& hwm,
			low_watermark const& lwm,
			fast_semaphore & fsem) :
		impl_( new detail::bounded_fifo_base< T >( hwm, lwm, fsem) )
	{}

    bounded_fifo( BOOST_RV_REF( bounded_fifo) other) :
       impl_()
    { swap( other); } 

    bounded_fifo & operator=( BOOST_RV_REF( bounded_fifo) other)
    {
        if ( this == other) return * this;
        bounded_fifo tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return impl_ ? unspecified_bool : 0; }

    bool operator!() const
    { return ! impl_; }

    void swap( bounded_fifo & other)
    { impl_.swap( other.impl_); }

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

	void put( value_type const& va)
	{
        BOOST_ASSERT( impl_);
        impl_->put( va);
	}

	template< typename TimeDuration >
	void put(
		value_type const& va,
		TimeDuration const& rel_time)
	{
        BOOST_ASSERT( impl_);
        impl_->put( va, rel_time);
	}

	bool try_take( value_type & va)
	{
        BOOST_ASSERT( impl_);
		return impl_->try_take( va);
	}
};

template< typename T >
void swap( bounded_fifo< T > & l, bounded_fifo< T > & r)
{ l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_BOUNDED_FIFO_H
