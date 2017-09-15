/**
 * @file     WRdsLicenseConnection.h
 * @author   wangxu.st@centerm.com
 * @date     2016/1/27
 * @version  1.0
 * @brief    NEPЭ����Ȩ������ͷ�ļ�
 */
#pragma once

#ifndef __WRDS_LICENSE_CONNECTION_H__
#define __WRDS_LICENSE_CONNECTION_H__

#include <Windows.h>
#include <wtsprotocol.h>
#include "Windows\IPipeClient.h"

/**
 * @class CNepWRdsLicenseConnection
 * @brief NEPЭ����Ȩ���ӣ�COM���
 *
 *
 * ����CNepWRdsLicenseConnection����ϵͳTerimin Service���أ���ִ��
 * ���˵������https://msdn.microsoft.com/en-us/library/hh707224(v=vs.85).aspx
 * �̳���IWRdsProtocolLicenseConnection�ӿڣ�����IWRdsProtocolLicenseConnectionͬʱ������IUnknow�ӿڣ�������Ҫʵ�ִ�������
 * ���ӿڹ�ϵ�������˳�����https://msdn.microsoft.com/en-us/library/dd920049(v=vs.85).aspx
 */
class CNepWRdsLicenseConnection : public IWRdsProtocolLicenseConnection
{
public:
    /**
     * @brief ���캯��
     */
    CNepWRdsLicenseConnection();

    /**
     * @brief ��������
     */
    ~CNepWRdsLicenseConnection();

    /** < IUnknown��Ľӿ� */
    /**
     * @brief ���ݾ������ͣ�����ȡʵ�ʶ���
     *        ��������ΪIUnknow����IWRdsProtocolManager�࣬�ɷ����Լ�����������֧��
     * �������https://msdn.microsoft.com/en-us/library/windows/desktop/ms682521(v=vs.85).aspx
     *
     * @param[in]  rrid   ��ȡ������
     * @param[out] ppvObj ���صĶ���
     *
     * @return ����S_OK��ʾ��ȡ�ɹ����������ʾʧ��
     */
    virtual HRESULT WINAPI QueryInterface(const IID& riid, VOID** ppvObj);

    /**
     * @brief ��������
     * �������https://msdn.microsoft.com/en-us/library/windows/desktop/ms691379(v=vs.85).aspx
     *
     * @return ���Ӻ������
     */
    virtual ULONG WINAPI AddRef();

    /**
     * @brief �������ã����ٵ�0���Զ�����
     * �������https://msdn.microsoft.com/en-us/library/windows/desktop/ms682317(v=vs.85).aspx
     *
     * @return ���Ӻ������
     */
    virtual ULONG WINAPI Release();

    /**
     * @brief �ɿͻ�������license������
     *
     * @param[in] ppLicenseCapabilities  ����
     * @param[in] pcbLicenseCapabilities ������С
     *
     * @return ����S_OK��ʾ��ȡ�ɹ����������ʾʧ�� 
     */
    virtual HRESULT WINAPI RequestLicensingCapabilities(PWRDS_LICENSE_CAPABILITIES ppLicenseCapabilities, ULONG *pcbLicenseCapabilities);
  
    /**
     * @brief ��ͻ��˷�����Ȩ
     *
     * @param[in] pClientLicense        ��Ȩ
     * @param[in] cbClientLicense       ��Ȩ��С
     *
     * @return ����S_OK��ʾ��ȡ�ɹ����������ʾʧ�� 
     */
    virtual HRESULT WINAPI SendClientLicense(PBYTE pClientLicense, ULONG cbClientLicense);
   
    /**
     * @brief ��ͻ���������Ȩ
     *
     * @param[in] Reserve1               ����1
     * @param[in] Reserve2               ����2
     * @param[in] ppClientLicense        ��Ȩ
     * @param[in] pcbClientLicense       ��Ȩ��С
     *
     * @return ����S_OK��ʾ��ȡ�ɹ����������ʾʧ�� 
     */
    virtual HRESULT WINAPI RequestClientLicense(PBYTE Reserve1, ULONG Reserve2, PBYTE ppClientLicense, ULONG *pcbClientLicense);
   
    /**
     * @brief ֪ͨ��Ȩ���
     *
     * @param[in] ulComplete  1��ʾ��ȨOK,������ʾʧ��
     *
     * @return ����S_OK��ʾ��ȡ�ɹ����������ʾʧ�� 
     */
    virtual HRESULT WINAPI ProtocolComplete(ULONG ulComplete);

private:
    LONG                             m_lRef;               /** < ���ü���                         */
};

#endif