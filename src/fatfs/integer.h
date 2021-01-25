/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER

#ifdef _WIN32	/* FatFs development platform */

    #include <windows.h>
    #include <tchar.h>
    typedef unsigned __int64 QWORD;


#else			/* Embedded platform */

    #include    <wchar.h>

    /* These types MUST be 16-bit or 32-bit */
    typedef int				INT;
    typedef unsigned int	UINT;

    /* This type MUST be 8-bit */
    typedef unsigned char	BYTE;

    /* These types MUST be 16-bit */
    typedef short			SHORT;
    typedef unsigned short	WORD;
    typedef wchar_t     	WCHAR;

    /* These types MUST be 32-bit */
    typedef long			LONG;
    typedef unsigned long	DWORD;

    /* This type MUST be 64-bit (Remove this for C89 compatibility) */
    typedef unsigned long long QWORD;

    #include <assert.h>
    static_assert(sizeof(WCHAR) == 2, "WCHAR size doesn't match. Use -fshort-wchar  option");

#endif

#endif