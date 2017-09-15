/**
 * @file     WRdsLicenseConnection.h
 * @author   wangxu.st@centerm.com
 * @date     2016/1/27
 * @version  1.0
 * @brief    NEP协议授权连接器头文件
 */
#pragma once

#ifndef __WRDS_LICENSE_CONNECTION_H__
#define __WRDS_LICENSE_CONNECTION_H__

#include <Windows.h>
#include <wtsprotocol.h>
#include "Windows\IPipeClient.h"

/**
 * @class CNepWRdsLicenseConnection
 * @brief NEP协议授权连接，COM组件
 *
 *
 * 用于CNepWRdsLicenseConnection，由系统Terimin Service加载，并执行
 * 相关说明见：https://msdn.microsoft.com/en-us/library/hh707224(v=vs.85).aspx
 * 继承自IWRdsProtocolLicenseConnection接口，由于IWRdsProtocolLicenseConnection同时继续自IUnknow接口，所以需要实现大量函数
 * 各接口关系，与加载顺序见：https://msdn.microsoft.com/en-us/library/dd920049(v=vs.85).aspx
 */
class CNepWRdsLicenseConnection : public IWRdsProtocolLicenseConnection
{
public:
    /**
     * @brief 构造函数
     */
    CNepWRdsLicenseConnection();

    /**
     * @brief 析构函数
     */
    ~CNepWRdsLicenseConnection();

    /** < IUnknown类的接口 */
    /**
     * @brief 根据具体类型，来获取实际对象
     *        对于类型为IUnknow类与IWRdsProtocolManager类，可返回自己本身，他类则不支持
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms682521(v=vs.85).aspx
     *
     * @param[in]  rrid   获取的类型
     * @param[out] ppvObj 返回的对象
     *
     * @return 返回S_OK表示获取成功，其他则表示失败
     */
    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    /**
     * @brief 增加引用
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms691379(v=vs.85).aspx
     *
     * @return 增加后的引用
     */
    virtual ULONG WINAPI AddRef();

    /**
     * @brief 减少引用，减少到0，自动析构
     * 定义见：https://msdn.microsoft.com/en-us/library/windows/desktop/ms682317(v=vs.85).aspx
     *
     * @return 增加后的引用
     */
    virtual ULONG WINAPI Release();

    /**
     * @brief 由客户端请求license的能力
     *
     * @param[in] ppLicenseCapabilities  能力
     * @param[in] pcbLicenseCapabilities 能力大小
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI RequestLicensingCapabilities(PWRDS_LICENSE_CAPABILITIES ppLicenseCapabilities, ULONG *pcbLicenseCapabilities);
  
    /**
     * @brief 向客户端发送授权
     *
     * @param[in] pClientLicense        授权
     * @param[in] cbClientLicense       授权大小
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI SendClientLicense(PBYTE pClientLicense, ULONG cbClientLicense);
   
    /**
     * @brief 向客户端请求授权
     *
     * @param[in] Reserve1               保留1
     * @param[in] Reserve2               保留2
     * @param[in] ppClientLicense        授权
     * @param[in] pcbClientLicense       授权大小
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI RequestClientLicense(PBYTE Reserve1, ULONG Reserve2, PBYTE ppClientLicense, ULONG *pcbClientLicense);
   
    /**
     * @brief 通知授权情况
     *
     * @param[in] ulComplete  1表示授权OK,其他表示失败
     *
     * @return 返回S_OK表示获取成功，其他则表示失败 
     */
    virtual HRESULT WINAPI ProtocolComplete(ULONG ulComplete);

private:
    LONG                             m_lRef;               /** < 引用计数                         */
};

#endif