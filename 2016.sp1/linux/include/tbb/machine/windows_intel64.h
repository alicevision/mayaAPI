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

#if !defined(__TBB_machine_H) || defined(__TBB_machine_windows_intel64_H)
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#define __TBB_machine_windows_intel64_H

#define __TBB_WORDSIZE 8
#define __TBB_ENDIANNESS __TBB_ENDIAN_LITTLE

#include <intrin.h>
#include "msvc_ia32_common.h"

//TODO: Use _InterlockedXXX16 intrinsics for 2 byte operations
#if !__INTEL_COMPILER
    #pragma intrinsic(_InterlockedOr64)
    #pragma intrinsic(_InterlockedAnd64)
    #pragma intrinsic(_InterlockedCompareExchange)
    #pragma intrinsic(_InterlockedCompareExchange64)
    #pragma intrinsic(_InterlockedExchangeAdd)
    #pragma intrinsic(_InterlockedExchangeAdd64)
    #pragma intrinsic(_InterlockedExchange)
    #pragma intrinsic(_InterlockedExchange64)
#endif /* !(__INTEL_COMPILER) */

#if __INTEL_COMPILER && (__INTEL_COMPILER < 1100)
    #define __TBB_compiler_fence()    __asm { __asm nop }
    #define __TBB_full_memory_fence() __asm { __asm mfence }
#elif _MSC_VER >= 1300 || __INTEL_COMPILER
    #pragma intrinsic(_ReadWriteBarrier)
    #pragma intrinsic(_mm_mfence)
    #define __TBB_compiler_fence()    _ReadWriteBarrier()
    #define __TBB_full_memory_fence() _mm_mfence()
#endif

#define __TBB_control_consistency_helper() __TBB_compiler_fence()
#define __TBB_acquire_consistency_helper() __TBB_compiler_fence()
#define __TBB_release_consistency_helper() __TBB_compiler_fence()

// ATTENTION: if you ever change argument types in machine-specific primitives,
// please take care of atomic_word<> specializations in tbb/atomic.h
extern "C" {
    __int8 __TBB_EXPORTED_FUNC __TBB_machine_cmpswp1 (volatile void *ptr, __int8 value, __int8 comparand );
    __int8 __TBB_EXPORTED_FUNC __TBB_machine_fetchadd1 (volatile void *ptr, __int8 addend );
    __int8 __TBB_EXPORTED_FUNC __TBB_machine_fetchstore1 (volatile void *ptr, __int8 value );
    __int16 __TBB_EXPORTED_FUNC __TBB_machine_cmpswp2 (volatile void *ptr, __int16 value, __int16 comparand );
    __int16 __TBB_EXPORTED_FUNC __TBB_machine_fetchadd2 (volatile void *ptr, __int16 addend );
    __int16 __TBB_EXPORTED_FUNC __TBB_machine_fetchstore2 (volatile void *ptr, __int16 value );
}

inline long __TBB_machine_cmpswp4 (volatile void *ptr, __int32 value, __int32 comparand ) {
    return _InterlockedCompareExchange( (long*)ptr, value, comparand );
}
inline long __TBB_machine_fetchadd4 (volatile void *ptr, __int32 addend ) {
    return _InterlockedExchangeAdd( (long*)ptr, addend );
}
inline long __TBB_machine_fetchstore4 (volatile void *ptr, __int32 value ) {
    return _InterlockedExchange( (long*)ptr, value );
}

inline __int64 __TBB_machine_cmpswp8 (volatile void *ptr, __int64 value, __int64 comparand ) {
    return _InterlockedCompareExchange64( (__int64*)ptr, value, comparand );
}
inline __int64 __TBB_machine_fetchadd8 (volatile void *ptr, __int64 addend ) {
    return _InterlockedExchangeAdd64( (__int64*)ptr, addend );
}
inline __int64 __TBB_machine_fetchstore8 (volatile void *ptr, __int64 value ) {
    return _InterlockedExchange64( (__int64*)ptr, value );
}

#define __TBB_USE_FETCHSTORE_AS_FULL_FENCED_STORE           1
#define __TBB_USE_GENERIC_HALF_FENCED_LOAD_STORE            1
#define __TBB_USE_GENERIC_RELAXED_LOAD_STORE                1
#define __TBB_USE_GENERIC_SEQUENTIAL_CONSISTENCY_LOAD_STORE 1

inline void __TBB_machine_OR( volatile void *operand, intptr_t addend ) {
    _InterlockedOr64((__int64*)operand, addend); 
}

inline void __TBB_machine_AND( volatile void *operand, intptr_t addend ) {
    _InterlockedAnd64((__int64*)operand, addend); 
}

#define __TBB_AtomicOR(P,V) __TBB_machine_OR(P,V)
#define __TBB_AtomicAND(P,V) __TBB_machine_AND(P,V)

