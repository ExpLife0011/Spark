#pragma once

#ifndef __BASE_PACKET_T_H__
#define __BASE_PACKET_T_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable:4200)
#pragma pack(1)

typedef struct
{
    DWORD  Length;
    DWORD  Type;
    BYTE   Data[0];
} BASE_PACKET_T, *PBASE_PACKET_T;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
