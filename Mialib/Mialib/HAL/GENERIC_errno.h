/*! @addtogroup GENERIC
 *  @{
 */
/******************************************************************************
*                                                                             *
*                   COPYRIGHT (C) 2009    ABB OY, DRIVES                      *
*                                                                             *
*******************************************************************************
*                                                                             *
*                                                                             *
*                     Daisy Assistant Panel SW                                *
*                                                                             *
*                                                                             *
*                  Subsystem:   Generic                                       *
*                                                                             *
*                   Filename:   GENERIC_errno.h                               *
*                       Date:   2009-02-18                                    *
*                                                                             *
*                 Programmer:   ?Janne Mäkihonka / Espotel Oy Nivala          *
*                     Target:   ?                                             *
*                                                                             *
*******************************************************************************/
/*!
 *  @file GENERIC_errno.h
 *  @par File description:
 *    System wide return code (errno) definitions
 *
 *  @par Related documents:
 *    List here...
 *
 *  @note
 *    START HERE!!!
 */
/******************************************************************************/

#ifndef GENERIC_ERRNO_INC  /* Allow multiple inclusions */
#define GENERIC_ERRNO_INC

//-----------------------------------------------------------------------------
// 1) INCLUDE FILES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 2) ABBREVIATIONS AND CONSTANTS
//-----------------------------------------------------------------------------

typedef enum  {
    ER_OK			= 0,	  /* No error */
    ERPERM              = -1,   /* Operation not permitted */
    ERNOENT             = -2,   /* No such file or directory */
    ERSRCH              = -3,   /* No such process */
    ERINTR              = -4,   /* Interrupted system call */
    ERIO                = -5,   /* I/O error */
    ERNXIO              = -6,   /* No such device or address */
    ER2BIG              = -7,   /* Argument list too long */
    ERNOEXEC            = -8,   /* Exec format error */
    ERBADF              = -9, /* Bad file number */
    ERCHILD             = -10, /* No child processes */
    ERAGAIN             = -11, /* Try again (= EWOULDBLOCK in some systems) */
    ERNOMEM             = -12, /* Out of memory */
    ERACCES             = -13, /* Permission denied */
    ERFAULT             = -14, /* Bad address */
    ERNOTBLK            = -15, /* Block device required */
    ERBUSY              = -16, /* Device or resource busy */
    EREXIST             = -17, /* File exists */
    ERXDEV              = -18, /* Cross-device link */
    ERNODEV             = -19, /* No such device */
    ERNOTDIR            = -20, /* Not a directory */
    ERISDIR             = -21, /* Is a directory */
    ERINVAL             = -22, /* Invalid argument */
    ERNFILE             = -23, /* File table overflow */
    ERMFILE             = -24, /* Too many open files */
    ERNOTTY             = -25, /* Not a typewriter */
    ERTXTBSY            = -26, /* Text file busy */
    ERFBIG              = -27, /* File too large */
    ERNOSPC             = -28, /* No space left on device */
    ERSPIPE             = -29, /* Illegal seek */
    ERROFS              = -30, /* Read-only file system */
    ERMLINK             = -31, /* Too many links */
    ERPIPE              = -32, /* Broken pipe */
    ERDOM               = -33, /* Math argument out of domain of func */
    ERRANGE             = -34, /* Math result not representable */
    ERDEADLK            = -35, /* Resource deadlock would occur */
    ERNAMETOOLONG       = -36, /* File name too long */
    ERNOLCK             = -37, /* No record locks available */
    ERNOSYS             = -38, /* Function not implemented */
    ERNOTEMPTY          = -39, /* Directory not empty */
    ERLOOP              = -40, /* Too many symbolic links encountered */
    //ERWOULDBLOCK      = ERAGAIN, /* Operation would block */
    ERNOMSG             = -42, /* No message of desired type */
    ERIDRM              = -43, /* Identifier removed */
    ERCHRNG             = -44, /* Channel number out of range */
    ERL2NSYNC           = -45, /* Level 2 not synchronized */
    ERL3HLT             = -46, /* Level 3 halted */
    ERL3RST             = -47, /* Level 3 reset */
    ERLNRNG             = -48, /* Link number out of range */
    ERUNATCH            = -49, /* Protocol driver not attached */
    ERNOCSI             = -50, /* No CSI structure available */
    ERL2HLT             = -51, /* Level 2 halted */
    ERBADE              = -52, /* Invalid exchange */
    ERBADR              = -53, /* Invalid request descriptor */
    ERXFULL             = -54, /* Exchange full */
    ERNOANO             = -55, /* No anode */
    ERBADRQC            = -56, /* Invalid request code */
    ERBADSLT            = -57, /* Invalid slot */
    //ERDEADLOCK        = ERDEADLK
    ERBFONT             = -59, /* Bad font file format */
    ERNOSTR             = -60, /* Device not a stream */
    ERNODATA            = -61, /* No data available */
    ERTIME              = -62, /* Timer expired */
    ERNOSR              = -63, /* Out of streams resources */
    ERNONET             = -64, /* Machine is not on the network */
    ERNOPKG             = -65, /* Package not installed */
    ERREMOTE            = -66, /* Object is remote */
    ERNOLINK            = -67, /* Link has been severed */
    ERADV               = -68, /* Advertise error */
    ERSRMNT             = -69, /* Srmount error */
    ERCOMM              = -70, /* Communication error on send */
    ERPROTO             = -71, /* Protocol error */
    ERMULTIHOP          = -72, /* Multihop attempted */
    ERDOTDOT            = -73, /* RFS specific error */
    ERBADMSG            = -74, /* Not a data message */
    EROVERFLOW          = -75, /* Value too large for defined data type */
    ERNOTUNIQ           = -76, /* Name not unique on network */
    ERBADFD             = -77, /* File descriptor in bad state */
    ERREMCHG            = -78, /* Remote address changed */
    ERLIBACC            = -79, /* Can not access a needed shared library */
    ERLIBBAD            = -80, /* Accessing a corrupted shared library */
    ERLIBSCN            = -81, /* .lib section in a.out corrupted */
    ERLIBMAX            = -82, /* Attempting to link in too many shared libraries */
    ERLIBEXEC           = -83, /* Cannot exec a shared library directly */
    ERILSEQ             = -84, /* Illegal byte sequence */
    ERRESTART           = -85, /* Interrupted system call should be restarted */
    ERSTRPIPE           = -86, /* Streams pipe error */
    ERUSERS             = -87, /* Too many users */
    ERNOTSOCK           = -88, /* Socket operation on non-socket */
    ERDESTADDRREQ       = -89, /* Destination address required */
    ERMSGSIZE           = -90, /* Message too long */
    ERPROTOTYPE         = -91, /* Protocol wrong type for socket */
    ERNOPROTOOPT        = -92, /* Protocol not available */
    ERPROTONOSUPPORT    = -93, /* Protocol not supported */
    ERSOCKTNOSUPPORT    = -94, /* Socket type not supported */
    EROPNOTSUPP         = -95, /* Operation not supported on transport endpoint */
    ERPFNOSUPPORT       = -96, /* Protocol family not supported */
    ERAFNOSUPPORT       = -97, /* Address family not supported by protocol */
    ERADDRINUSE         = -98, /* Address already in use */
    ERADDRNOTAVAIL      = -99, /* Cannot assign requested address */
    ERNETDOWN           = -100, /* Network is down */
    ERNETUNREACH        = -101, /* Network is unreachable */
    ERNETRESET          = -102, /* Network dropped connection because of reset */
    ERCONNABORTED       = -103, /* Software caused connection abort */
    ERCONNRESET         = -104, /* Connection reset by peer */
    ERNOBUFS            = -105, /* No buffer space available */
    ERISCONN            = -106, /* Transport endpoint is already connected */
    ERNOTCONN           = -107, /* Transport endpoint is not connected */
    ERSHUTDOWN          = -108, /* Cannot send after transport endpoint shutdown */
    ERTOOMANYREFS       = -109, /* Too many references: cannot splice */
    ERTIMEDOUT          = -110, /* Connection timed out */
    ERCONNREFUSED       = -111, /* Connection refused */
    ERHOSTDOWN          = -112, /* Host is down */
    ERHOSTUNREACH       = -113, /* No route to host */
    ERALREADY           = -114, /* Operation already in progress */
    ERINPROGRESS        = -115, /* Operation now in progress */
    ERSTALE             = -116, /* Stale NFS file handle */
    ERUCLEAN            = -117, /* Structure needs cleaning */
    ERNOTNAM            = -118, /* Not a XENIX named type file */
    ERNAVAIL            = -119, /* No XENIX semaphores available */
    ERISNAM             = -120, /* Is a named type file */
    ERREMOTEIO          = -121, /* Remote I/O error */
    ERDQUOT             = -122, /* Quota exceeded */

    ERNOMEDIUM          = -123, /* No medium found */
    ERMEDIUMTYPE        = -124, /* Wrong medium type */
    ERCANCELED          = -125, /* Operation Canceled */
    ERNOKEY             = -126, /* Required key not available */
    ERKEYEXPIRED        = -127, /* Key has expired */
    ERKEYREVOKED        = -128, /* Key has been revoked */
    ERKEYREJECTED       = -129, /* Key was rejected by service */

    /* for robust mutexes */
    EROWNERDEAD         = -130, /* Owner died */
    ERNOTRECOVERABLE    = -131 /* State not recoverable */

} HAL_E_Status;


#endif  /* of GENERIC_ERRNO_INC */

/*! @} */ /* EOF, no more */
