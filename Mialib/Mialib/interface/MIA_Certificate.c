/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2019    ABB, DRIVES, Beijing                *
*                                                                             *
*******************************************************************************/

#include "MIA_Certificate.h"
#include <string.h>
static MIA_DeviceDescriptor s_deviceInfo;

static const char * nextString(const char * source)
{
/*	while(*source++);
	return source;
    */
    return NULL;
}

void Mia_CertificateInit()
{

    /*const char * infoArea = (const char *)DEVICE_INFO_START_ADDR;

	s_deviceInfo.deviceInfo = infoArea;
	s_deviceInfo.name = nextString(s_deviceInfo.deviceInfo);
	s_deviceInfo.globalProvUrl = nextString(s_deviceInfo.name);
	s_deviceInfo.idScope = nextString(s_deviceInfo.globalProvUrl);

	s_deviceInfo.serverCert = (const char *)SERVER_CERT_START_ADDR;
	s_deviceInfo.caCert = (const char *)DEVICE_CERT_START_ADDR;
	s_deviceInfo.pubKey = (const char *)ISSUED_CERT_START_ADDR;
	s_deviceInfo.thumbPrint = nextString(s_deviceInfo.pubKey);
	s_deviceInfo.privateKey = (const char *)PRIVATE_KEY_START_ADDR;
*/
}

const char * Mia_CertificateGetIdScope()
{
	return s_deviceInfo.idScope;
}

const char * Mia_CertificateGetName()
{
	return s_deviceInfo.name;
}

const char * Mia_CertificateGetPubKey()
{
	return s_deviceInfo.pubKey;
}

const char * Mia_CertificateGetPrivateKey()
{
	return s_deviceInfo.privateKey;
}

const char * Mia_CertificateGetThumbPrint()
{
	return s_deviceInfo.thumbPrint;
}

const char * Mia_CertificateGetServerCert()
{
	return s_deviceInfo.serverCert;
}

const char * Mia_CertificateGetUrl()
{
	return s_deviceInfo.globalProvUrl;
}
