/*
    Copyright 2005-2014 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef __TBB_mutex_padding_H
#define __TBB_mutex_padding_H

// wrapper for padding mutexes to be alone on a cache line, without requiring they be allocated
// from a pool.  Because we allow them to be defined anywhere they must be two cache lines in size.


namespace tbb {
namespace interface7 {
namespace internal {

static const size_t cache_line_size = 64;

// Pad a mutex to occupy a number of full cache lines sufficient to avoid false sharing
// with other data; space overhead is up to 2*cache_line_size-1.
template<typename Mutex, bool is_rw> class padded_mutex;

template<typename Mutex>
class padded_mutex<Mutex,false> : tbb::internal::mutex_copy_deprecated_and_disabled {
    typedef long pad_type;
    pad_type my_pad[((sizeof(Mutex)+cache_line_size-1)/cache_line_size+1)*cache_line_size/sizeof(pad_type)];

    Mutex *impl() { return (Mutex *)((uintptr_t(this)|(cache_line_size-1))+1);}

public:
    static const bool is_rw_mutex = Mutex::is_rw_mutex;
    static const bool is_recursive_mutex = Mutex::is_recursive_mutex;
    static const bool is_fair_mutex = Mutex::is_fair_mutex;

    padded_mutex() { new(impl()) Mutex(); }
    ~padded_mutex() { impl()->~Mutex(); }

    //! Represents acquisition of a mutex.
    class scoped_lock :  tbb::internal::no_copy {
        typename Mutex::scoped_lock my_scoped_lock;
    public:
        scoped_lock() : my_scoped_lock() {}
        scoped_lock( padded_mutex& m ) : my_scoped_lock(*m.impl()) { }
        ~scoped_lock() {  }

        void acquire( padded_mutex& m ) { my_scoped_lock.acquire(*m.impl()); }
        bool try_acquire( padded_mutex& m ) { return my_scoped_lock.try_acquire(*m.impl()); }
        void release() { my_scoped_lock.release(); }
    };
};

template<typename Mutex>
class padded_mutex<Mutex,true> : tbb::internal::mutex_copy_deprecated_and_disabled {
    typedef long pad_type;
    pad_type my_pad[((sizeof(Mutex)+cache_line_size-1)/cache_line_size+1)*cache_line_size/sizeof(pad_type)];

    Mutex *impl() { return (Mutex *)((uintptr_t(this)|(cache_line_size-1))+1);}

public:
    static const bool is_rw_mutex = Mutex::is_rw_mutex;
    static const bool is_recursive_mutex = Mutex::is_recursive_mutex;
    static const bool is_fair_mutex = Mutex::is_fair_mutex;

    padded_mutex() { new(impl()) Mutex(); }
    ~padded_mutex() { impl()->~Mutex(); }

    //! Represents acquisition of a mutex.
    class scoped_lock :  tbb::internal::no_copy {
        typename Mutex::scoped_lock my_scoped_lock;
    public:
        scoped_lock() : my_scoped_lock() {}
        scoped_lock( padded_mutex& m, bool write = true ) : my_scoped_lock(*m.impl(),write) { }
        ~scoped_lock() {  }

        void acquire( padded_mutex& m, bool write = true ) { my_scoped_lock.acquire(*m.impl(),write); }
        bool try_acquire( padded_mutex& m, bool write = true ) { return my_scoped_lock.try_acquire(*m.impl(),write); }
        bool upgrade_to_writer() { return my_scoped_lock.upgrade_to_writer(); }
        bool downgrade_to_reader() { return my_scoped_lock.downgrade_to_reader(); }
        void release() { my_scoped_lock.release(); }
    };
};

} // namespace internal
} // namespace interface7
} // namespace tbb

#endif /* __TBB_mutex_padding_H */
