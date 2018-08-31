#include "Base/BaseObject.h"

using namespace enlib; 

CObject::CObject()
{
    m_lRef = 1;
}

CObject::~CObject()
{

}

LONG CObject::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

LONG CObject::Release()
{
    LONG Temp = InterlockedDecrement(&m_lRef);
    if (Temp == 0)
    {
        delete this;
    }

    return Temp;
}