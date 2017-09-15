#include "Base/BaseObject.h"

CBaseObject::CBaseObject()
{
    m_lRef = 1;
}

CBaseObject::~CBaseObject()
{

}

LONG CBaseObject::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

LONG CBaseObject::Release()
{
    LONG Temp = InterlockedDecrement(&m_lRef);
    if (Temp == 0)
    {
        delete this;
    }

    return Temp;
}