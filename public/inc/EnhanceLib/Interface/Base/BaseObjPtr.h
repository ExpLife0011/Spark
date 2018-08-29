#pragma once

#ifndef __BASE_OBJ_PTR_H__
#define __BASE_OBJ_PTR_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

template <class T>
class CBaseObjPtr
{
public:
    CBaseObjPtr()
    {
        m_pPtr = NULL;
    };

    CBaseObjPtr(T* lp)
    {
        m_pPtr = lp;
        m_pPtr->AddRef();
    };

    CBaseObjPtr(const CBaseObjPtr<T>& objptr)
    {
        T* lp = objptr.GetPtr();
        m_pPtr = lp;
        m_pPtr->AddRef();
    };

    virtual ~CBaseObjPtr()
    {
        if (m_pPtr)
        {
            m_pPtr->Release();
        }
    };

    CBaseObjPtr<T>& operator=(T* lp)
    {
        if (m_pPtr != lp)
        {
            if (m_pPtr != NULL)
            {
                m_pPtr->Release();
                m_pPtr = NULL;
            }

            if (lp != NULL)
            {
                m_pPtr = lp;
                m_pPtr->AddRef();
            }
        }
        return *this;
    };

    CBaseObjPtr<T>& operator=(const CBaseObjPtr<T>& objptr)
    {
        T* lp = objptr.GetPtr();
        if (m_pPtr != lp)
        {
            if (m_pPtr != NULL)
            {
                m_pPtr->Release();
                m_pPtr = NULL;
            }

            if (lp != NULL)
            {
                m_pPtr = lp;
                m_pPtr->AddRef();
            }
        }
        return *this;
    };

    template <typename Q>
    CBaseObjPtr<T>& operator=(const CBaseObjPtr<Q>& objptr)
    {
        Q* lp2 = objptr.GetPtr();
        T* lp = dynamic_cast<T*>(lp2);

        if (m_pPtr != lp)
        {
            if (m_pPtr != NULL)
            {
                m_pPtr->Release();
                m_pPtr = NULL;
            }

            if (lp != NULL)
            {
                m_pPtr = lp;
                m_pPtr->AddRef();
            }
        }
        return *this;
    }

    T* operator->() const
    {
        return m_pPtr;
    };

    bool operator!() const
    {
        return (m_pPtr == NULL);
    };

    bool operator!=(T* pT) const
    {
        return !operator==(pT);
    }
    bool operator==(T* pT) const
    {
        return m_pPtr == pT;
    }

private:
    T* GetPtr() const
    {
        return m_pPtr;
    }

    T* m_pPtr;
};

#endif