/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Jul 16 13:00:42 2001
 */
/* Compiler settings for C:\DEPOT\QT\MAIN\TESTS\DISTSYSTEM\licproc_com\licproc_com.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_ILicProc = {0x4881DDEE,0xECE8,0x4496,{0x9F,0x4A,0xFD,0x33,0xFC,0x50,0x0F,0xED}};


const IID LIBID_LICPROC_COMLib = {0x5F5A3404,0xF6FD,0x4DE0,{0x94,0xAA,0x98,0xF7,0x1E,0xC8,0xCE,0x91}};


const CLSID CLSID_LicProc = {0x5A084CF4,0xC6E2,0x42B5,{0xA9,0x18,0x1F,0xAD,0x5E,0xA0,0x8F,0x38}};


#ifdef __cplusplus
}
#endif

