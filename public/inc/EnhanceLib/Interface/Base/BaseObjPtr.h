#pragma once

#ifndef __ENLIB_OBJ_PTR_H__
#define __ENLIB_OBJ_PTR_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

namespace enlib
{
    template <class T>
    class CObjPtr
    {
    public:
        CObjPtr()
        {
            m_pPtr = NULL;
        };

        CObjPtr(T* lp)
        {
            m_pPtr = lp;
            if (m_pPtr)
            {
                m_pPtr->AddRef();
            }
        };

        CObjPtr(const CObjPtr<T>& objptr)
        {
            T* lp = objptr.GetPtr();
            m_pPtr = lp;
            if (m_pPtr)
            {
                m_pPtr->AddRef();
            }
        };

        virtual ~CObjPtr()
        {
            if (m_pPtr)
            {
                m_pPtr->Release();
            }
        };

        CObjPtr<T>& operator=(T* lp)
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

        CObjPtr<T>& operator=(const CObjPtr<T>& objptr)
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
        CObjPtr<T>& operator=(const CObjPtr<Q>& objptr)
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

        operator T*() const throw()
        {
            return m_pPtr;
        }

        T& operator*() const
        {
            return *m_pPtr;
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

        T* GetPtr() const
        {
            return m_pPtr;
        }

        T* m_pPtr;
    };
};

#endif