#ifndef _gpuCacheMakeSharedHelper_h_
#define _gpuCacheMakeSharedHelper_h_

//-
//**************************************************************************/
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

///////////////////////////////////////////////////////////////////////////////
//
// Make Shared Helper
//
// Helpers to declare boost::make_shared friend.
//
////////////////////////////////////////////////////////////////////////////////

#include <boost/make_shared.hpp>

namespace GPUCache {

// Zero-argument versions
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_0 template< class AA > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared();

#if !defined( BOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// Variadic templates, rvalue reference

#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_1 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_2 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_3 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_4 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_5 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_6 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_7 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_8 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_9 template< class AA, class Arg1, class... Args > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( Arg1 && arg1, Args && ... args );

#elif !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// For example MSVC 10.0

#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_1 template< class AA, class A1 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_2 template< class AA, class A1, class A2 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_3 template< class AA, class A1, class A2, class A3 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_4 template< class AA, class A1, class A2, class A3, class A4 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_5 template< class AA, class A1, class A2, class A3, class A4, class A5 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_6 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_7 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_8 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_9 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7, A8 && a8, A9 && a9 );

#else

// C++03 version

#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_1 template< class AA, class A1 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_2 template< class AA, class A1, class A2 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_3 template< class AA, class A1, class A2, class A3 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_4 template< class AA, class A1, class A2, class A3, class A4 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_5 template< class AA, class A1, class A2, class A3, class A4, class A5 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_6 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_7 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_8 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8 );
#define GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_9 template< class AA, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 > friend typename boost::detail::sp_if_not_array< AA >::type boost::make_shared( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9 );

#endif

} // namespace GPUCache

#endif

