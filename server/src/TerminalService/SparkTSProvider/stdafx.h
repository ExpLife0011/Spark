#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <initguid.h>
#include <list>

extern TCHAR* GUIDToString(const GUID &guid, TCHAR* guid_str);

/**
 * @brief 增加全局对象引用个数
 */
extern void DllAddRef();

/**
 * @brief 减少全局对象引用个数
 */
extern void DllRelease();

#endif