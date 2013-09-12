
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_STATIC_POOL_H
#define BOOST_TASKS_STATIC_POOL_H

#include <cstddef>

#include <boost/config.hpp>
#include <boost/context/stack_utils.hpp>
#include <boost/move/move.hpp>
#include <boost/result_of.hpp>

#include <boost/task/detail/pool_base.hpp>
#include <boost/task/detail/worker_group.hpp>
#include <boost/task/exceptions.hpp>
#include <boost/task/meta.hpp>
#include <boost/task/poolsize.hpp>
#include <boost/task/stacksize.hpp>
#include <boost/task/task.hpp>
#include <boost/task/watermark.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {

template typename Queue, bool Bound = is_bound< Queue >::value, bool HasAttr = has_attribute< Queue >::value >
class static_pool;

template< typename Queue >
class static_pool< Queue, false, false >
{
public:
	typedef Queue	queue_type;

private:
	typedef detail::pool_base< queue_type >     base_type;

	BOOST_MOVABLE_BUT_NOT_COPYABLE( static_pool);	

    base_type::ptr_t                    		pool_;

public:
    typedef void ( * unspecified_bool_type)( static_pool ***);

    static void unspecified_bool( static_pool ***) {}

	static_pool() :
		pool_()
	{}
	
	explicit static_pool(
			poolsize const& psize,
			stacksize const& stack_size = stacksize( ctx::default_stacksize() ) ) :
		pool_( new base_type( psize, stack_size) )
	{}

	static_pool( BOOST_RV_REF( static_pool) other) :
		pool_()
	{ pool_.swap( other.pool_); }

	static_pool & operator=( BOOST_RV_REF( static_pool) other)
	{
		static_pool tmp( other);
		swap( tmp);
		return * this;
	}

	operator unspecified_bool_type() const // throw()
	{ return pool_; }

	bool operator!() const // throw()
	{ return ! pool_; }

	void swap( static_pool & other) // throw()
	{ pool_.swap( other.pool_); }

	void interrupt_all_worker()
	{
        BOOST_ASSERT( pool_);
		pool_->interrupt_all_worker();
	}

	void shutdown()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown();
	}

	const void shutdown_now()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown_now();
	}

	std::size_t size() const
	{
        BOOST_ASSERT( pool_);
		return pool_->size();
	}

	bool closed() const
	{
        BOOST_ASSERT( pool_);
		return pool_->closed();
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( Fn fn)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( fn);
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( boost::move( fn) );
	}
};

template< typename Queue >
class static_pool< Queue, true, false >
{
public:
	typedef Queue	queue_type;

private:
	typedef detail::pool_base< queue_type >     base_type;

	BOOST_MOVABLE_BUT_NOT_COPYABLE( static_pool);	

    base_type::ptr_t                    		pool_;

public:
    typedef void ( * unspecified_bool_type)( static_pool ***);

    static void unspecified_bool( static_pool ***) {}

	static_pool() :
		pool_()
	{}

	explicit static_pool(
			poolsize const& psize,
			high_watermark const& hwm,
			low_watermark const& lwm,
			stacksize const& stack_size = stacksize( ctx::default_stacksize() ) ) :
		pool_( new base_type( psize, hwm, lwm, stack_size) )
	{}

	static_pool( BOOST_RV_REF( static_pool) other) :
		pool_()
	{ pool_.swap( other.pool_); }

	static_pool & operator=( BOOST_RV_REF( static_pool) other)
	{
		static_pool tmp( other);
		swap( tmp);
		return * this;
	}

	operator unspecified_bool_type() const // throw()
	{ return pool_; }

	bool operator!() const // throw()
	{ return ! pool_; }

	void swap( static_pool & other) // throw()
	{ pool_.swap( other.pool_); }

	void interrupt_all_worker()
	{
        BOOST_ASSERT( pool_);
		pool_->interrupt_all_worker();
	}

	void shutdown()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown();
	}

	const void shutdown_now()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown_now();
	}

	std::size_t size() const
	{
        BOOST_ASSERT( pool_);
		return pool_->size();
	}

	bool closed() const
	{
        BOOST_ASSERT( pool_);
		return pool_->closed();
	}

	std::size_t upper_bound() const
	{
        BOOST_ASSERT( pool_);
		return pool_->upper_bound();
	}

	void upper_bound( high_watermark const& hwm)
	{
        BOOST_ASSERT( pool_);
		pool_->upper_bound( hwm);
	}

	std::size_t lower_bound() const
	{
        BOOST_ASSERT( pool_);
		return pool_->lower_bound();
	}

	void lower_bound( low_watermark const lwm)
	{
        BOOST_ASSERT( pool_);
		pool_->lower_bound( lwm);
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( Fn fn)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( fn);
	}

	template< typename Fn >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( boost::move( fn) );
	}
};

template< typename Queue >
class static_pool< Queue, false, true >
{
public:
	typedef Queue	queue_type;

private:
	typedef detail::pool_base< queue_type >     base_type;

	BOOST_MOVABLE_BUT_NOT_COPYABLE( static_pool);	

    base_type::ptr_t                    		pool_;

public:
    typedef void ( * unspecified_bool_type)( static_pool ***);

    static void unspecified_bool( static_pool ***) {}

	static_pool() :
		pool_()
	{}
	
	explicit static_pool(
			poolsize const& psize,
			stacksize const& stack_size = stacksize( ctx::default_stacksize() ) ) :
		pool_( new base_type( psize, stack_size) )
	{}

	static_pool( BOOST_RV_REF( static_pool) other) :
		pool_()
	{ pool_.swap( other.pool_); }

	static_pool & operator=( BOOST_RV_REF( static_pool) other)
	{
		static_pool tmp( other);
		swap( tmp);
		return * this;
	}

	operator unspecified_bool_type() const // throw()
	{ return pool_; }

	bool operator!() const // throw()
	{ return ! pool_; }

	void swap( static_pool & other) // throw()
	{ pool_.swap( other.pool_); }

	void interrupt_all_worker()
	{
        BOOST_ASSERT( pool_);
		pool_->interrupt_all_worker();
	}

	void shutdown()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown();
	}

	const void shutdown_now()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown_now();
	}

	std::size_t size() const
	{
        BOOST_ASSERT( pool_);
		return pool_->size();
	}

	bool closed() const
	{
        BOOST_ASSERT( pool_);
		return pool_->closed();
	}

	template< typename R, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( Fn fn, Attr const& attr)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( fn, attr);
	}

	template< typename R, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn, Attr const& attr)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( boost::move( fn), attr);
	}
};

template< typename Queue >
class static_pool< Queue, true, true >
{
public:
	typedef Queue	queue_type;

private:
	typedef detail::pool_base< queue_type >     base_type;

	BOOST_MOVABLE_BUT_NOT_COPYABLE( static_pool);	

    base_type::ptr_t                    		pool_;

public:
    typedef void ( * unspecified_bool_type)( static_pool ***);

    static void unspecified_bool( static_pool ***) {}

	static_pool() :
		pool_()
	{}

	explicit static_pool(
			poolsize const& psize,
			high_watermark const& hwm,
			low_watermark const& lwm,
			stacksize const& stack_size = stacksize( ctx::default_stacksize() ) ) :
		pool_( new base_type( psize, hwm, lwm, stack_size) )
	{}

	static_pool( BOOST_RV_REF( static_pool) other) :
		pool_()
	{ pool_.swap( other.pool_); }

	static_pool & operator=( BOOST_RV_REF( static_pool) other)
	{
		static_pool tmp( other);
		swap( tmp);
		return * this;
	}

	operator unspecified_bool_type() const // throw()
	{ return pool_; }

	bool operator!() const // throw()
	{ return ! pool_; }

	void swap( static_pool & other) // throw()
	{ pool_.swap( other.pool_); }

	void interrupt_all_worker()
	{
        BOOST_ASSERT( pool_);
		pool_->interrupt_all_worker();
	}

	void shutdown()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown();
	}

	const void shutdown_now()
	{
        BOOST_ASSERT( pool_);
		pool_->shutdown_now();
	}

	std::size_t size() const
	{
        BOOST_ASSERT( pool_);
		return pool_->size();
	}

	bool closed() const
	{
        BOOST_ASSERT( pool_);
		return pool_->closed();
	}

	std::size_t upper_bound() const
	{
        BOOST_ASSERT( pool_);
		return pool_->upper_bound();
	}

	void upper_bound( high_watermark const& hwm)
	{
        BOOST_ASSERT( pool_);
		pool_->upper_bound( hwm);
	}

	std::size_t lower_bound() const
	{
        BOOST_ASSERT( pool_);
		return pool_->lower_bound();
	}

	void lower_bound( low_watermark const lwm)
	{
        BOOST_ASSERT( pool_);
		pool_->lower_bound( lwm);
	}

	template< typename R, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( Fn fn, Attr const& attr)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( fn, attr);
	}

	template< typename R, typename Attr >
	task< typename result_of< Fn() >::result_type > submit( BOOST_RV_REF( Fn) fn, Attr const& attr)
	{
        BOOST_ASSERT( pool_);
		return pool_->submit( boost::move( fn), attr);
	}
};

template< typename Queue >
void swap( tasks::static_pool< Queue > & l, tasks::static_pool< Queue > & r)
{ return l.swap( r); }

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_STATIC_POOL_H

