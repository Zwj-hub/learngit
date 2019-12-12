/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2019   ABB, DRIVES, Beijing                 *
*                                                                             *
*******************************************************************************/

#ifndef MIA_CERTIFICATE_INC_
#define MIA_CERTIFICATE_INC_

typedef struct MIA_DeviceDescriptor
{
	const char * deviceInfo;
	const char * caCert;
	const char * pubKey;
	const char * thumbPrint;
	const char * privateKey;
	const char * idScope;
	const char * globalProvUrl;
	const char * name;
	const char * serverCert;

} MIA_DeviceDescriptor;

#define DEVICE_INFO_START_ADDR 0x081E0000
#define SERVER_CERT_START_ADDR 0x08004000
#define DEVICE_CERT_START_ADDR 0x08005400
#define ISSUED_CERT_START_ADDR 0x08005C00
#define PRIVATE_KEY_START_ADDR 0x08006400

void Mia_CertificateInit();
const char * Mia_CertificateGetServerCert();
const char * Mia_CertificateGetIdScope();
const char * Mia_CertificateGetName();
const char * Mia_CertificateGetPubKey();
const char * Mia_CertificateGetPrivateKey();
const char * Mia_CertificateGetThumbPrint();
const char * Mia_CertificateGetServerCert();
const char * Mia_CertificateGetUrl();

#endif

