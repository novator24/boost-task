
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TASKS_CALLABLE_H
#define BOOST_TASKS_CALLABLE_H

#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <boost/task/context.hpp>
#include <boost/task/detail/config.hpp>

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace tasks {
namespace detail {

struct BOOST_TASK_DECL callable_base
{
	atomic< unsigned int >	use_count;

	callable_base() :
		use_count( 0)
	{}

	virtual ~callable_base() {}

	virtual void run() = 0;

	virtual void reset( shared_ptr< thread > const&) = 0;

	inline friend void intrusive_ptr_add_ref( callable_base * p)
	{ p->use_count.fetch_add( 1, memory_order_relaxed); }
	
	inline friend void intrusive_ptr_release( callable_base * p)
	{
		if ( p->use_count.fetch_sub( 1, memory_order_release) == 1)
		{
			atomic_thread_fence( memory_order_acquire);
			delete p;
		}
	}
};

template< typename R, typename D >
struct exec
{
	void run()
	{
        D * d = static_cast< D >( this);
        d->prom_.set( d->fn_() );
    }
};

template< typename D >
struct exec< void, D >
{
	void run()
	{
        D * d = static_cast< D >( this);
        d->fn_();
        d->prom_.set();
    }
};

template< typename Fn, typename Promise >
class callable_object : public callable_base,
                        public exec< Fn, callabl_object< Fn, Promise > >
{
private:
	Fn		fn_;
    Promise prom_;
	context	ctx_;

public:
	callable_object( Fn fn,
			         BOOST_RV_REF( Promise) prom,
			         context const& ctx) :
		fn_( boost::move( fn) ),
        prom_( boost::move( prom) ),
        ctx_( ctx)
	{}

	callable_object( BOOST_RV_REF( Fn) fn,
			         BOOST_RV_REF( Promise) prom,
			         context const& ctx) :
		fn_( boost::move( fn) ),
        prom_( boost::move( prom) ),
        ctx_( ctx)
	{}

	void reset( shared_ptr< thread > const& thrd)
	{ ctx_.reset( thrd); }
};

}

class BOOST_TASK_DECL callable
{
private:
	intrusive_ptr< detail::callable_base >	base_;

public:
	callable();

	template< typename Fn, typename Promise >
	callable( Fn fn,
			  BOOST_RV_REF( Promise) prom,
			  context const& ctx) :
		base_( new detail::callable_object< Fn, Promise >(
                fn, prom, ctx) )
	{}

	template< typename Fn, typename Promise >
	callable( BOOST_RV_REF( Fn) fn,
			  BOOST_RV_REF( Promise) prom,
			  context const& ctx) :
		base_( new detail::callable_object< Fn, Promise >(
                boost::move( fn), boost::move( prom), ctx) )
	{}

	void operator()();

	bool empty() const;

	void reset( shared_ptr< thread > const&);

	void swap( callable &);
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#endif // BOOST_TASKS_CALLABLE_H

