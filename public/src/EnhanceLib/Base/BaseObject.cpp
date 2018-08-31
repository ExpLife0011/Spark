#include "Base/BaseObject.h"

using namespace enlib; 

CObject::CObject()
{
    m_lRef = 1;
}

CObject::~CObject()
{

}

ULONG CObject::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CObject::Release()
{
    ULONG Temp = InterlockedDecrement(&m_lRef);
    if (Temp == 0)
    {
        delete this;
    }

    return Temp;
}