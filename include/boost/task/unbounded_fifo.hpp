
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_UNBOUNDED_FIFO_H
#define BOOST_TASKS_UNBOUNDED_FIFO_H

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

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename T >
class unbounded_fifo_base
{
public:
	typedef detail::has_no_attribute	attribute_tag_type;
	typedef T       					value_type;

private:
	struct node
	{
		typedef intrusive< node >	ptr_type;

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

    std::size_t         use_count_;
	atomic< state >		state_;
	node::sptr_t		head_;
	mutable mutex		head_mtx_;
	node::sptr_t		tail_;
	mutable mutex		tail_mtx_;
	fast_semaphore	*	fsem_;

	bool active_() const
	{ return ACTIVE == state_.load(); }

	void deactivate_()
	{ state_.store( DEACTIVE); }

	bool empty_() const
	{ return head_ == get_tail_(); }

	node::sptr_t get_tail_() const
	{
		lock_guard< mutex > lk( tail_mtx_);	
		node::sptr_t tmp = tail_;
		return tmp;
	}

	node::sptr_t pop_head_()
	{
		node::sptr_t old_head = head_;
		head_ = old_head->next;
		return old_head;
	}

public:
	unbounded_fifo_base() :
        use_count_( 0),
		state_( ACTIVE),
		head_( new node),
		head_mtx_(),
		tail_( head_),
		tail_mtx_(),
		fsem_( 0)
	{}

	unbounded_fifo_base( fast_semaphore & fsem) :
        use_count_( 0),
		state_( ACTIVE),
		head_( new node),
		head_mtx_(),
		tail_( head_),
		tail_mtx_(),
		fsem_( & fsem)
	{}

	bool active() const
	{ return active_(); }

	void deactivate()
	{
		unique_lock< mutex > lk( head_mtx_);
		deactivate_();
	}

	bool empty() const
	{
		unique_lock< mutex > lk( head_mtx_);
		return empty_();
	}

	void put( value_type const& va)
	{
		node::sptr_t new_node( new node);
		{
			unique_lock< mutex > lk( tail_mtx_);
			if ( ! active_() )
				BOOST_THROW_EXCEPTION( task_rejected("queue is not active") );
			tail_->va = va;
			tail_->next = new_node;
			tail_ = new_node;
		}
		if( fsem_) fsem_->post();
	}

	bool try_take( value_type & va)
	{
		unique_lock< mutex > lk( head_mtx_);
		if ( empty_() )
			return false;
		va.swap( head_->va);
		pop_head_();
		return ! va.empty();
	}

    friend
    inline void intrusive_ptr_add_ref( unbounded_fifo_base< T > * p)
    { p->use_count_.fetch_add( 1, memory_order_relaxed); }

    friend
    inline void intrusive_ptr_release( unbounded_fifo_base< T > * p)
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
class unbounded_fifo
{
private:
    typedef unbounded_fifo< T >   queue_type;

    typename detail::unbounded_fifo_base< T >::ptr_type   impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( queue_type);

public:
	typedef typename detail::unbounded_fifo_base< T >::attribute_tag_type	attribute_tag_type;
	typedef typename detail::unbounded_fifo_base< T >::value_type			value_type;
    typedef void ( * unspecified_bool_type)( unbounded_fifo< T > ***);

    static void unspecified_bool( unbounded_fifo< T > ***) {}

	unbounded_fifo_base() :
		impl_( new detail::unbounded_fifo_base< T >() )
	{}

	unbounded_fifo_base( fast_semaphore & fsem) :
		impl_( new detail::unbounded_fifo_base< T >( fsem) )
	{}

    unbounded_fifo( BOOST_RV_REF( queue_type) other) :
       impl_()
    { swap( other); } 

    unbounded_fifo & operator=( BOOST_RV_REF( queue_type) other)
    {
        if ( this == other) return * this;
        unbounded_fifo tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return impl_ ? unspecified_bool : 0; }

    bool operator!() const
    { return ! impl_; }

    void swap( unbounded_fifo & other)
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
void swap( unbounded_fifo< T > & l, unbounded_fifo< T > & r)
{ l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_UNBOUNDED_FIFO_H
