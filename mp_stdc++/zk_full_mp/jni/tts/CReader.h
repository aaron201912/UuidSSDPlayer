#ifndef __CREADER_H__
#define __CREADER_H__

#include "base_types.h"
#include "CReader_const.h"


/************************************************************************/
// TTS callback                                                                    
/************************************************************************/
typedef INT (*CReaderCallback) (VOID *pVoid, INT nFlag, CHAR *pchData, INT nSize);


#ifndef TTSDLL_API
#if defined WIN32 || defined _WIN32_WCE
#define TTSDLL_API __declspec(dllexport)
#else
#define TTSDLL_API
#endif
#endif 

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _CReaderSDKVersionInfo
{
	const char* pchSDKVersion;
	const char* pchSDKName;
	const char* pchReleaseTo;
	const char* pchReleaseDate;
	BOOL  bTrialVersion;
	BOOL  bOpenLicense;
} CReaderSDKVersionInfo;

/************************************************************************/
// GetLangID and Speaker
/************************************************************************/
//Purpose: Get languages that TTS support
//Param:
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  pLangID[OUT]: the language ID support
//  nSize[IN]: lenth of pLangID
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: number of language support 
//Remark: pass pLangID with NULL to get how many languages support and allocate buffer for store langID.
TTSDLL_API INT CReader_GetAvailableLangID(const char* lpchDataPath, INT *pLangID, INT nSize, CREADER_STATUS* pnErr);

//Purpose: Get Speaker number in this language.
//Param:
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  nLangID[IN]: the language ID
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: number of speaker the language support.
TTSDLL_API INT CReader_GetAvailableSpeakerNum(const char* lpchDataPath, INT nLangID, CREADER_STATUS* pnErr);

//Purpose: Get Speaker in this language.
//Param:
//  lpchLibPath[IN]: the library path which saves *.so, and the path is a UTF8 string
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  nLangID[IN]: the language ID
//  nSpeaker[IN]: get which speaker
//	lpchSpeaker[OUT]: speaker name
//	nBufferSize[IN]: length of lpchSpeaer
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: size of speaker name, return 0 if error occur
//Remark: pass lpchSpeaker with NULL to get speaker name size and allocate buffer for store speaker name. Use nSpeaker to choose which speaker to get.
TTSDLL_API INT CReader_GetAvailableSpeaker(const char* lpchLibPath, const char* lpchDataPath, INT nLangID, INT nSpeaker, char *lpchSpeaker, int nBufferSize, CREADER_STATUS* pnErr);

/************************************************************************/
// Initialize and Release                                                                      
/************************************************************************/
//Purpose: Initialize SDK
//Param:
//  nVsrLangID[IN]: the language ID
//  lpchLibPath[IN]: the library path which saves *.so, and the path is a UTF8 string
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  lpchVoiceName[IN]: the voice name of TTS bin files, and the name is a UTF8 string
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: handle of TTS engine
//Remark: For nVsrLangID, please refer to "base_types.h", this function will find license in data path
TTSDLL_API HANDLE CReader_Init(const INT nVsrLangID, const char* lpchLibPath, const char* lpchDataPath, const char* lpchVoiceName, CREADER_STATUS* pnErr);


//Purpose: Initialize SDK
//Param:
//  nVsrLangID[IN]: the language ID
//  lpchLibPath[IN]: the library path which saves *.so, and the path is a UTF8 string
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  lpchVoiceName[IN]: the voice name of TTS bin files, and the name is a UTF8 string
//  lpchLicensePath[IN]: the path of license bin files, and the path is a UTF8 string
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: handle of TTS engine
//Remark: For nVsrLangID, please refer to "base_types.h"
TTSDLL_API HANDLE CReader_InitWithLicense(const INT nVsrLangID, const char* lpchLibPath, const char* lpchDataPath, const char* lpchVoiceName, const char* lpchLicensePath, CREADER_STATUS* pnErr);


//Purpose: Release SDK
//Param:
//  hTTS[IN]: handle of TTS engine
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_Release(HANDLE hTTS);

/************************************************************************/
// Play(Generate TTS data)                                                                      
/************************************************************************/
//Purpose: Start to TTS synthesis flow
//Param: 
//  hTTS[IN]: handle of TTS engine
//  lpwcUttr[IN]: the UNICODE string(2 bytes/little endian) for TTS data
//  nType[IN]: reserved. Please assign it to CREADER_TYPE_NORMAL
//  lpfnCallback[IN]: The callback function. It is called by SDK when SDK generates TTS data
//  lpCBVoid[IN]: The first parameter when the callback function is called. Caller can use it to transfer parameters.
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_Start(HANDLE hTTS, const UNICODE *lpwcUttr, CREADER_TYPE nType, CReaderCallback lpfnCallback, LPVOID lpCBVoid);

//Purpose: UTF8 Ver CReader_Start
//Param: 
//  hTTS[IN]: handle of TTS engine
//  lpwcUttr[IN]: the string(1 bytes) for TTS data
//  nType[IN]: reserved. Please assign it to CREADER_TYPE_NORMAL
//  lpfnCallback[IN]: The callback function. It is called by SDK when SDK generates TTS data
//  lpCBVoid[IN]: The first parameter when the callback function is called. Caller can use it to transfer parameters.
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_StartWithUTF8(HANDLE hTTS, const char *lpchUttr, CREADER_TYPE nType, CReaderCallback lpfnCallback, LPVOID lpCBVoid);

//Purpose: Stop CReader
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: CREADER_RET_OK indicates success, else error code

TTSDLL_API CREADER_STATUS CReader_Stop(HANDLE hTTS);

//Purpose: Do TTS synthesis
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: 1. CREADER_RET_DONE indicates TTS synthesis finished
//    	  2. CREADER_RET_OK indicates continue doing TTS synthesis
//		  3. CREADER_RET_WAV_OUT_STOP indicates TTS synthesis stopped by CReader_Stop(...)
//        4. error code.
//Remark: Caller must call it until return value is CREADER_RET_WAV_OUT_STOP or error code. 
TTSDLL_API CREADER_STATUS CReader_Next(HANDLE hTTS);

/************************************************************************/
// Settings                                                                    
/************************************************************************/
//Purpose: Set volume value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nOutputFormat[IN]: The output format of TTS data. refer to CREADER_OUTUT_FORMAT in CReader_const.h
//Return: CREADER_RET_OK indicates success, else error code
//Remark: If not call this API, the defaulted format is 16k/16 bit/Mono PCM.
TTSDLL_API CREADER_STATUS CReader_SetOutputFormat(HANDLE hTTS, const CREADER_OUTPUT_FORMAT nOutputFormat);



//Purpose: Set volume value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nVolume[IN]: The volume value. The range is 0 ~ 500 (%). The default value is 200.
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_SetVolume(HANDLE hTTS, const INT nVolume);


//Purpose: Get volume value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The volume value of TTS or error code
TTSDLL_API INT CReader_GetVolume(HANDLE hTTS);


//Purpose: Set speed value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nSpeed[IN]: The speed value. The range from fast to slow is 200 ~ 50. The default value is 100.
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_SetSpeed(HANDLE hTTS, const INT nSpeed);


//Purpose: Get speed value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The speed value of TTS or error code
TTSDLL_API INT CReader_GetSpeed(HANDLE hTTS);


//Purpose: Set pitch value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nPitch[IN]: The pitch value. The range from low to high is 50 ~ 200. The default value is 100.
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API CREADER_STATUS CReader_SetPitch(HANDLE hTTS, const INT nPitch);

//Purpose: Get pitch value of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The pitch value of TTS or error code
TTSDLL_API INT CReader_GetPitch(HANDLE hTTS);

//Purpose: Set short delay of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nShortDelay[IN]: The short delay value. in millisecond.
//Return: CREADER_RET_OK indicates success, else error code
//Remark: Other not ，。!?； punctuations.
TTSDLL_API CREADER_STATUS CReader_SetShortDelay(HANDLE hTTS, const INT nShortDelay);

//Purpose: Get short delay of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The short delay value of TTS or error code
TTSDLL_API INT CReader_GetShortDelay(HANDLE hTTS);

//Purpose: Set long delay of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  nLongDelay[IN]: The long delay value. in millisecond.
//Return: CREADER_RET_OK indicates success, else error code
//Remark: Long delay string:，。!?；
TTSDLL_API CREADER_STATUS CReader_SetLongDelay(HANDLE hTTS, const INT nLongDelay);

//Purpose: Get Long delay of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The long delay value of TTS or error code
TTSDLL_API INT CReader_GetLongDelay(HANDLE hTTS);

//Purpose: Set mapping table for TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//  lpchPath[IN]: path of mapping table
//Return: 0 if Success, or err code
TTSDLL_API INT CReader_SetMappingTable(HANDLE hTTS, const char *lpchPath);

//Purpose: Get Callback Data size of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The size of callback TTS data in bytes or error code
TTSDLL_API INT CReader_GetTTSCallBackSize(HANDLE hTTS);

//Purpose: Get Sample rate of TTS
//Param: 
//  hTTS[IN]: handle of TTS engine
//Return: The Sample Rate of TTS or error code
TTSDLL_API INT CReader_GetTTSSampleRate(HANDLE hTTS);

// Purpose	:	Get SDK Version Info
//Param: 
//  lpCReaderSDKVersionInfo[IN]: struct of SDK version & info
//Return: CREADER_RET_OK indicates success, else error code
TTSDLL_API INT CReader_GetVersionInfo(char* lpchLicenseFile, CReaderSDKVersionInfo *lpCReaderSDKVersionInfo);


/************************************************************************/
// Online license
/************************************************************************/
#ifdef PEROBOT_ONLINE_CHECK_LICENSE
//Purpose: Register and get license from server
//Param: 
//  lpchLicensePath[IN]: License file name. License will be saved here.
//  lpchUUID[IN]:        UUID. Used for register.
//  lpchSerialKey[IN]:   SerialKey(provided by Cyberon). Used for register.
//Return: CREADER_RET_OK indicates success, else error code
//Remark: 
TTSDLL_API CREADER_STATUS CReader_Server_RegisterKey(const char*lpchLicensePath, const char*lpchUUID, const char*lpchSerialKey);

//Purpose: Initialize SDK
//Param:
//  nVsrLangID[IN]: the language ID
//  lpchLibPath[IN]: the library path which saves *.so, and the path is a UTF8 string
//  lpchDataPath[IN]: the data path which saves *.bin, and the path is a UTF8 string
//  lpchVoiceName[IN]: the voice name of TTS bin files, and the name is a UTF8 string
//  lpchLicensePath[IN]: the path of license bin files, and the path is a UTF8 string
//  pnErr[OUT]: error code. CREADER_RET_OK indicates success, else error code
//Return: handle of TTS engine
//Remark: For nVsrLangID, please refer to "base_types.h"
TTSDLL_API HANDLE CReader_Server_InitWithLicense(const INT nVsrLangID, const char* lpchLibPath, const char* lpchDataPath, const char* lpchVoiceName, const char* lpchLicensePath, CREADER_STATUS* pnErr);
#endif //PEROBOT_ONLINE_CHECK_LICENSE

#ifdef __cplusplus
}
#endif

#endif // __CREADER_H__
