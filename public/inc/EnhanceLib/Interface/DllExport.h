#pragma once

#ifndef __DLLEXPORT_H__
#define __DLLEXPORT_H__

#ifdef WIN32
#define DLL_COMMONLIB_API __declspec(dllexport)
#else
#define DLL_COMMONLIB_API 
#endif

#endif