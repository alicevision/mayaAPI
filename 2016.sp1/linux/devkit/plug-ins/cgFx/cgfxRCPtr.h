#ifndef _cgfxRCPtr_h_
#define _cgfxRCPtr_h_

//
// File: cgfxRCPtr.h
//
// Smart pointers to reference counted objects.
//
//-
// ==========================================================================
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

template <class T>
class cgfxRCPtr 
{
public:
    typedef T element_type;

    cgfxRCPtr()
        : fPointee(0)
    {}

    explicit cgfxRCPtr(element_type* pointee) 
        : fPointee(pointee) 
    {
        if (fPointee) {
            fPointee->addRef();
        }
    }
    
    cgfxRCPtr(const cgfxRCPtr<T>& rhs) 
        : fPointee(rhs.fPointee) 
    {
        if (fPointee) {
            fPointee->addRef();
        }
    }
    
    const cgfxRCPtr<T>& operator=(const cgfxRCPtr<T>& rhs) 
    {
        if (rhs.fPointee != fPointee) {
            if (fPointee) {
                fPointee->release();
            }

            fPointee = rhs.fPointee;
            
            if (fPointee) {
                fPointee->addRef();
            }
        }

        return *this;
    }
    

    ~cgfxRCPtr()
    {
        if (fPointee) {
            fPointee->release();
            fPointee = 0;
        }
    }


    element_type* operator->() const 
    {
        return fPointee;
    }
    
    bool isNull() const
    {
        return fPointee == 0;
    }


    bool isEqualTo(const cgfxRCPtr<T>& rhs) const
    {
        return fPointee == rhs.fPointee;
    }


private:
    element_type* fPointee;
};

template <class T> bool operator==(
    const cgfxRCPtr<T>& lhs,
    const cgfxRCPtr<T>& rhs)
{
    return lhs.isEqualTo(rhs);
}

template <class T> bool operator!=(
    const cgfxRCPtr<T>& lhs,
    const cgfxRCPtr<T>& rhs)
{
    return !lhs.isEqualTo(rhs);
}

#endif /* _cgfxRCPtr_h_ */
