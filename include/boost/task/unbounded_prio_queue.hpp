
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_UNBOUNDED_PRIO_QUEUE_H
#define BOOST_TASKS_UNBOUNDED_PRIO_QUEUE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <queue>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <boost/task/detail/meta.hpp>
#include <boost/task/exceptions.hpp>
#include <boost/task/fast_semaphore.hpp>

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
class unbounded_prio_queue_base
{
public:
	typedef detail::has_attribute	attribute_tag_type;
	typedef Attr					attribute_type;
	typedef intrusive_ptr<
		unbounded_prio_queue_base< T, Attr, Comp >
	>								ptr_type;

	struct value_type
	{
		T       		ca;
		attribute_type	attr;

		value_type(
				T const& ca_,
				attribute_type const& attr_) :
			ca( ca_), attr( attr_)
		{ BOOST_ASSERT( ! ca.empty() ); }

		void swap( value_type & other)
		{
			ca.swap( other.ca);
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
	>						queue_type;

	enum state
	{
		ACTIVE = 0,
		DEACTIVE
	};

	atomic< state >			state_;
	queue_type				queue_;
	mutable shared_mutex	mtx_;
	fast_semaphore		&	fsem_;

	bool active_() const
	{ return ACTIVE == state_.load(); }

	void deactivate_()
	{ state_.store( DEACTIVE); }

	bool empty_() const
	{ return queue_.empty(); }

	void put_( value_type const& va)
	{
		if ( ! active_() )
			BOOST_THROW_EXCEPTION( task_rejected("queue is not active") );
		queue_.push( va);
		fsem_.post();
	}

	bool try_take_( T & ca)
	{
		if ( empty_() )
			return false;
		T tmp( queue_.top().ca);
		queue_.pop();
		ca.swap( tmp);
		return ! ca.empty();
	}

public:
	unbounded_prio_queue_base( fast_semaphore & fsem) :
		state_( ACTIVE),
		queue_(),
		mtx_(),
		fsem_( fsem)
	{}

	bool active() const
	{ return active_(); }

	void deactivate()
	{
		unique_lock< shared_mutex > lk( mtx_);
		deactivate_();
	}

	bool empty() const
	{
		shared_lock< shared_mutex > lk( mtx_);
		return empty_();
	}

	void put( value_type const& va)
	{
		unique_lock< shared_mutex > lk( mtx_);
		put_( va);
	}

	bool try_take( T & ca)
	{
		unique_lock< shared_mutex > lk( mtx_);
		return try_take_( ca);
	}
};

}

template<
    typename T,
	typename Attr,
	typename Comp = std::less< Attr >
>
class unbounded_prio_queue
{
private:
    typedef unbounded_prio_queue< T >   queue_type;

    typename detail::unbounded_prio_queue_base< T >::ptr_type   impl_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( queue_type);

public:
	typedef typename detail::unbounded_prio_queue_base< T >::attribute_tag_type	attribute_tag_type;
	typedef typename detail::unbounded_prio_queue_base< T >::value_type			value_type;
    typedef void ( * unspecified_bool_type)( unbounded_prio_queue< T > ***);

    static void unspecified_bool( unbounded_prio_queue< T > ***) {}

	unbounded_prio_queue_base() :
		impl_( new detail::unbounded_prio_queue_base< T >() )
	{}

	unbounded_prio_queue( fast_semaphore & fsem) :
        impl_( new detail::unbounded_prio_queue_base< T, Attr, Comp >( fsem) )
	{}

    unbounded_prio_queue( BOOST_RV_REF( queue_type) other) :
       impl_()
    { swap( other); } 

    unbounded_prio_queue & operator=( BOOST_RV_REF( queue_type) other)
    {
        if ( this == other) return * this;
        unbounded_prio_queue tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    operator unspecified_bool_type() const
    { return impl_ ? unspecified_bool : 0; }

    bool operator!() const
    { return ! impl_; }

    void swap( unbounded_prio_queue & other)
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

	bool try_take( T & ca)
	{
		BOOST_ASSERT( impl_);
		return impl_->try_take( ca);
	}
};

template< typename T, typename Attr, typename Comp >
void swap( unbounded_prio_queue< T, Attr, Comp > & l, unbounded_prio_queue< T, Attr, Comp > & r)
{ return l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_UNBOUNDED_PRIO_QUEUE_H
