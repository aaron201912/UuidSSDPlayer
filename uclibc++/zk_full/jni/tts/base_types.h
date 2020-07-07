/*
	Copyright (c) 2002 Cyberon Corp.  All right reserved.
	File: base_types.h
	Desc: Define all base types
	Author: Alan
	Date: 2002/7/21
	FixBug: 2002/11/18 William
	Version : 1.30.00
*/

#ifndef	__BASE_TYPES_H
#define	__BASE_TYPES_H


#if defined(_WIN32) && !defined(__SYMBIAN32__)

	#include <windows.h>
	#include <tchar.h>
	#include <strsafe.h>
	
	typedef signed char INT8;
	typedef unsigned short USHORT;
	typedef unsigned int UINT;
	typedef unsigned long ULONG;
	typedef wchar_t WCHAR;
	typedef signed int INTEGER;

	#ifdef _WIN32
		#ifdef UNICODE
		#undef UNICODE
		#endif
		#define UNICODE WCHAR 
	#else
		#define UNICODE unsigned short 
	#endif

#if defined(_WIN32) && !defined(_WIN32_WCE)
#pragma warning(disable:4995)
#pragma warning(disable:4996)
#endif //#if defined(_WIN32) && !defined(_WIN32_WCE)

#elif defined(__SUNPLUS__)

	#include <general.h>

	typedef SINT8	CHAR; 		/*typedef signed char CHAR;*/
	typedef UINT8	BYTE;		/*typedef unsigned char BYTE;*/
	typedef SINT8	INT8;		/*typedef signed char INT8;*/
	typedef SINT16	SHORT;		/*typedef signed short SHORT;*/
	typedef UINT16	WORD;		/*typedef unsigned short WORD;*/
	typedef UINT16	USHORT;		/*typedef unsigned short USHORT;*/
	typedef SINT32	LONG;		/*typedef signed long LONG;*/
	typedef UINT32	DWORD;		/*typedef unsigned long DWORD;*/
	typedef UINT32	ULONG;		/*typedef unsigned long ULONG;*/
	typedef SINT32	INT;		/*typedef signed int INT;*/
	typedef UINT32	UINT;		/*typedef unsigned int UINT;*/
	typedef SINT32  BOOL;		/*typedef int BOOL;*/
	typedef SINT32	INTEGER;	/*typedef signed int INTEGER;*/
	typedef void	VOID;
	typedef	UINT8*	PBYTE;		/*typedef unsigned char*  PBYTE; unsigned 8 bit data */ 

	#ifndef UNICODE
	#define UNICODE wchar_t
	#endif

#else

	#if defined(__LINUX__)
	#include <ctype.h>
	#include <wchar.h>
	#endif

	typedef signed char CHAR;
	typedef unsigned char BYTE;
	typedef signed short SHORT;
	typedef unsigned short WORD;
	typedef signed long LONG;
	typedef unsigned long DWORD;
	//typedef unsigned int DWORD;
	typedef signed int INT;
	typedef unsigned int UINT;
	typedef int BOOL;
	typedef void VOID;
	typedef signed char INT8;
	typedef unsigned short USHORT;
	typedef unsigned long ULONG;
	typedef unsigned short WCHAR;
	typedef signed int INTEGER;
	
	#define UNICODE WORD
	/*
	#if defined(LINUX)
		#ifndef UNICODE
		#define UNICODE WORD
		#endif
	#else
		#ifndef UNICODE
		#define UNICODE wchar_t
		#endif
	#endif
	*/

#endif


#if defined(__SYMBIAN32__)
	#include <e32def.h>
	
	#ifndef HANDLE
	typedef void *HANDLE;
	#endif
	
	typedef struct tagFILETIME
	{
	    DWORD dwLowDateTime;
	    DWORD dwHighDateTime;
	}	FILETIME;
#else
	#ifndef HANDLE
	#define HANDLE	VOID*
	#endif
	
	#ifndef _WIN32
	typedef struct tagFILETIME
	{
	    DWORD dwLowDateTime;
	    DWORD dwHighDateTime;
	}	FILETIME;
	#endif
#endif

#include <stdlib.h>
#include <stdio.h>
//#include <string.h>

#ifdef LPVOID
#undef LPVOID
#endif
#define LPVOID	void*

#ifndef TRUE
#define TRUE  ( 1 == 1 )
#endif

#ifndef FALSE
#define FALSE ( 1 == 0 )
#endif

#ifndef NULL
#define NULL	((VOID*)0)
#endif

#define PNULL	((void*) 0)

#ifndef STATIC
#define STATIC 	static
#endif

#if defined(_WIN32) && !defined(__SYMBIAN32__)
	#define EXPAPI WINAPI
#else
	#define EXPAPI
#endif

#if defined (__SYMBIAN32__)
	#ifdef DLL_EXPORT
		#define DLLAPI EXPORT_C
	#else
		#define DLLAPI IMPORT_C
	#endif
#elif defined (_WIN32)
	#ifdef DLL_EXPORT
		#define DLLAPI __declspec(dllexport)
	#elif defined(_LIB)
		#define DLLAPI
	#else
		#define DLLAPI __declspec(dllimport)
	#endif
#endif

//#include "portable.h"

#ifdef PEROBOT_ONLINE_CHECK_LICENSE
#define NO_ZLIB
//#define NO_STRTOLL
#endif

/*enum
{
	VSR_LANG_UNDEFINED = 0,
	VSR_LANG_MANDARIN_TWN,		//	BSR_LANG_1
	VSR_LANG_MANDARIN_CHN,		//	BSR_LANG_2
	VSR_LANG_CANTONESE_HK,		//	BSR_LANG_3
	VSR_LANG_ENGLISH_USA,		//	BSR_LANG_4
	VSR_LANG_ENGLISH_UK,		//	BSR_LANG_5
	VSR_LANG_GERMAN_GER,		//	BSR_LANG_6
	VSR_LANG_SPANISH_SAM,		//	BSR_LANG_7
	VSR_LANG_FRENCH_FRA,		//	BSR_LANG_8
	VSR_LANG_ITALIAN_ITA,		//	BSR_LANG_9
	VSR_LANG_KOREAN_KOR,		//	BSR_LANG_10
	VSR_LANG_RUSSIAN_RUS,		//	BSR_LANG_11
	VSR_LANG_PORTUGUESE_BRA,	//	BSR_LANG_12
	VSR_LANG_ENGLISH_CHN,		//	BSR_LANG_13, Chinese-accent English. 2005/08/25
	VSR_LANG_THAI_THA,			//	BSR_LANG_14
	VSR_LANG_ARABIC_ARB,		//	BSR_LANG_15; not ready yet
	VSR_LANG_DUTCH_NL,			//	BSR_LANG_16
	VSR_LANG_JAPANESE_JP,		//	BSR_LANG_17
	VSR_LANG_HINDI_IND,			//	BSR_LANG_18; 2010/11/22

	VSR_LANG_POLISH_PL,			//	BSR_LANG_19
	VSR_LANG_CZECH_CZ,			//	BSR_LANG_20
	VSR_LANG_TURKISH_TUR,		//	BSR_LANG_21
	VSR_LANG_DANISH_DEN,		//	BSR_LANG_22
	VSR_LANG_SWEDISH_SWE,		//	BSR_LANG_23
	VSR_LANG_NORWEGIAN_NOR,		//	BSR_LANG_24; Norwegian. 2007/11/15
	VSR_LANG_FINNISH_FIN,		//	BSR_LANG_25; Finnish. 2007/10/12
	VSR_LANG_GREEK_GRE,			//	BSR_LANG_26; Greek. 2007/11/15
	VSR_LANG_PORTUGUESE_POR,	//	BSR_LANG_27
	VSR_LANG_SLOVAK_SLO,		//	BSR_LANG_28; not ready yet
	VSR_LANG_HUNGARIAN_HUN,		//	BSR_LANG_29; Hungarian. 2008/02/19
	VSR_LANG_UKRAINIAN_UKR,		//	BSR_LANG_30; not ready yet
	VSR_LANG_ENGLISH_AU,		//	BSR_LANG_31, Australian English. 2007/10/09
	VSR_LANG_ENGLISH_WW,		//	BSR_LANG_32, Worldwide English. 2007/10/09
	VSR_LANG_SPANISH_SPA,		//	BSR_LANG_33, Spanish (Euro)	2009/03/17
	VSR_LANG_MINNAN_TW,			//	BSR_LANG_34, Taiwan Minnan.�ԫn�y	2010/05/12  not ready yet
	VSR_LANG_VIETNAMESE_VIE,	//	BSR_LANG_35, VIETNAM VIETNAMESE.    2010/11/11  not ready yet
	VSR_LANG_ENGLISH_IND,		//	BSR_LANG_36, India-accent English.	2010/11/22
	VSR_LANG_ENGLISH_SEA,		//	BSR_LANG_37, Southeast Asia accent English.	not ready yet
	VSR_LANG_BAHASA_IDN,		//	BSR_LANG_38; Bahasa - Indonesia 2012/02/01
	VSR_LANG_BAHASA_MAL,		//	BSR_LANG_39; Bahasa - Malaysia not ready yet
	VSR_LANG_TAGALOG_PHI,		//	BSR_LANG_40; Tagalog - Philippines not ready yet
	VSR_LANG_HAKKA_TW,			//	BSR_LANG_41; Taiwan - Hakka	2018/03/26
	VSR_LANG_MAX				//	Max 2009/04/21
	
}; //Last update: 2007/10/09*/

#ifndef SAFE_DELETE
#define SAFE_DELETE(a)	if ( (a) != NULL ) {delete (a); (a) = NULL;}
#endif

#ifndef SAFE_DELETEARRAY
#define SAFE_DELETEARRAY(a)	if ( (a) != NULL ) {delete [](a); (a) = NULL;}
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(a)	if ( (a) != NULL ) {free(a); (a) = NULL;}
#endif

//#if !defined(_WIN32) && !defined(__SYMBIAN32__)
//#include "SubName.h"
//#endif

//For DMemory (only for WinPC & debug)
#if defined(_WIN32) && !defined(_WIN32_WCE) && defined(_DEBUG)
//#include "DMemory.h"
#endif

#endif	/* #ifndef	__BASE_TYPES_H */
