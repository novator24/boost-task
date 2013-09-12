
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_DETAIL_WORKER_H
#define BOOST_TASKS_DETAIL_WORKER_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/random.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <boost/task/detail/config.hpp>
#include <boost/task/detail/work.hpp>
#include <boost/task/detail/wsq.hpp>
#include <boost/task/poolsize.hpp>
#include <boost/task/stacksize.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

template< typename Worker >
void worker_function( typename Worker::ptr_t & worker)
{	
	while ( ! worker->shutdown_() )
	{
		work w;
		if ( ! ( worker->try_take_local_work_( w) || 
			 worker->try_take_global_work_( w) ||
			 worker->try_steal_other_work_( w) ) )
		{
			worker->pool_.fsem_.wait();
			continue;
		}

		work & active( * tss_);
		active = move( w);
		active.run();
		w = move( active);
		if ( ! w.is_complete() ) worker->wsq_.put( w);
	}
}

class worker : private noncopyable
{
public:
	typedef intrusive_ptr_t		ptr_t;
	typedef thread::id			id;

	virtual ~worker() {}

	virtual const id get_id() const = 0;

	virtual void join() const = 0;

	virtual void start() = 0;

	virtual void interrupt() const = 0;

	virtual bool try_steal( work &) = 0;

    static virtual worker * instance();

protected:
	worker() :
		use_count_( 0)
	{ tss_.reset( new work() ); }

	static thread_specific_ptr< work >	tss_; 

private:
    friend inline void intrusive_ptr_add_ref( worker * p)
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( worker * p)
    { if ( --p->use_count_ == 0) delete p; }

	std::size_t		use_count_;
};

template< typename Pool >
class worker_object : public worker
{
public:
	static ptr_t create( Pool & pool, poolsize const& psize)
	{ return ptr_t( new worker_object( pool, psize) ); }

	const id get_id() const
	{ return thrd_.get_id(); }

	void join() const
	{ thrd_.join(); }

	void start()
	{ thrd_ = thread( bind( & worker_function< worker_objecty< Pool > >, this) ); }

	void interrupt() const
	{ thrd_.interrupt(); }

	bool try_steal( work & w)
	{ return wsq_.try_steal( w); }

private:
    template< typename Worker >
	friend void worker_function( typename Worker::ptr_t &);

	class random_idx
	{
	private:
		rand48											rng_;
		uniform_int<>									six_;
		variate_generator< rand48 &, uniform_int<> >	die_;

	public:
		random_idx( std::size_t size) :
			rng_(), six_( 0, size - 1), die_( rng_, six_)
		{}

		std::size_t operator()()
		{ return die_(); }
	};

	worker_object( Pool & pool, poolsize const& psize) :
		worker(),
		pool_( pool),
		thrd_(),
		wsq_( pool_.fsem_),
		shtdwn_( false),
		rnd_idx_( psize)
	{}

	bool try_take_global_work_( work & w)
	{ return pool_.queue_.try_take( w); }

	bool try_take_local_work_( work & w)
	{ return wsq_.try_take( w); }
	
	bool try_steal_other_work_( work & w)
	{
		std::size_t idx( rnd_idx_() );
		for ( std::size_t j = 0; j < pool_.wg_.size(); ++j)
		{
			worker_object other( pool_.wg_[idx]);
			if ( this_thread::get_id() == other.get_id() ) continue;
			if ( ++idx >= pool_.wg_.size() ) idx = 0;
			if ( other.try_steal( w) )
				return true;
		}
		return false;
	}

	bool shutdown_()
	{
		if ( shutdown__() && pool_.queue_.empty() )
			return true;
		else if ( shutdown_now__() )
			return true;
		return false;
	}

	bool shutdown__()
	{
		if ( ! shtdwn_) shtdwn_ = pool_.shtdwn_;
		return shtdwn_;
	}
	
	bool shutdown_now__()
	{ return pool_.shtdwn_now_; }

	Pool		&	pool_;
	thread			thrd_;
	wsq				wsq_;
	bool			shtdwn_;
	random_idx		rnd_idx_;
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TASKS_DETAIL_WORKER_H

