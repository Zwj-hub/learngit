

//#define WOLFSSL_TRUST_PEER_CERT
//#define WOLFSSL_NO_TRUSTED_CERTS_VERIFY
#define IGNORE_KEY_EXTENSIONS

#define WOLFSSL_USER_IO

#define WC_NO_HARDEN

#define SINGLE_THREADED

#define NO_WOLFSSL_DIR

#define WOLFSSL_NO_SOCK

#define NO_WRITEV

#define NO_DEV_RANDOM
#define CUSTOM_RAND_GENERATE_BLOCK myRngFunc
int myRngFunc(unsigned char* output, unsigned int sz);

#define WC_NO_HASHDRBG

#define SIZEOF_LONG       4
#define SIZEOF_LONG_LONG  8

#define LARGE_STATIC_BUFFERS
#define STATIC_CHUNKS_ONLY

#define NO_FILESYSTEM

/*
#define XFILE      FS_FILE*
#define XFOPEN     wolfSSLfopen
#define XFSEEK     wolfSSLfseek
#define XFTELL     wolfSSLftell
#define XREWIND    wolfSSLrewind
#define XFREAD     wolfSSLfread
#define XFCLOSE    wolfSSLfclose
#define XSEEK_END  wolfSSL_SEEK_END
#define XBADFILE   NULL
*/

#include <time.h>
#define TIME_OVERRIDES

#define XTIME(a) time(a)
#define XGMTIME(a,b) localtime_r(a,b)
struct tm *localtime_r(const time_t *timep, struct tm *result);
#define USE_WOLF_VALIDDATE
//#define XVALIDATE_DATE(a,b,c)  ValidateDate(a,b,c)
//int ValidateDate(const unsigned char* date, unsigned char format, int dateType);
#define HAVE_TIME_T_TYPE
#define HAVE_TM_TYPE
//#define NEED_TMP_TIME

#define NO_WOLFSSL_SERVER 
//#define NO_DES3 
//#define NO_DSA 
//#define NO_MD4 
#define SMALL_SESSION_CACHE 

#define WOLFSSL_SMALL_STACK

//#define WOLFSSL_TLS13

//#define WOLFSSL_SHA512

//#define WOLFSSL_SHA384
//#define BUILD_TLS_PSK_WITH_AES_256_GCM_SHA384

#define WOLFSSL_TLS12

//#define WOLFSSL_SESSION_EXPORT
//#define HAVE_TLS_EXTENSIONS
//#define HAVE_HKDF
//#define WC_RSA_PSS
//#define HAVE_ECC
//#define HAVE_AESCCM

//#define NO_RC4

//#define HAVE_FIPS
#define WOLFSSL_AES_256
#define WOLFSSL_AES_128

#define HAVE_ECC 
//#define HAVE_AESGCM
//#define HAVE_AESCCM
//#define HAVE_POLY1305
//#define HAVE_CHACHA
//#define HAVE_ONE_TIME_AUTH

#define WOLFSSL_STATIC_RSA
#define HAVE_FFDHE_2048

//#define WOLFSSL_NO_SERVER_GROUPS_EXT

#define HAVE_CURVE25519

#define WOLFSSL_KEY_GEN
