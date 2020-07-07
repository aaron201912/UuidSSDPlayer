#ifndef __DSPOTTER_API_H
#define __DSPOTTER_API_H

#if defined(_WIN32)
	#ifdef DSPDLL_EXPORTS
		#define DSPDLL_API __declspec(dllexport)
	#endif
#endif

#ifndef DSPDLL_API
#define DSPDLL_API
#endif

#include "base_types.h"
#include "DSpotterApi_Const.h"

#ifdef __cplusplus
extern "C"{
#endif

// Purpose: Get number of group from pack bin.
// lpchPackBin(IN): The packed model, contents of CYBase.mod + Group_X.mod.
// Return: The number of group or error code.
DSPDLL_API INT DSpotterGetNumGroup(char *lpchPackBin);

// Purpose: Create a recognizer.
// lpchPackBin(IN): The packed model, contents of CYBase.mod + Group_X.mod.
// lpbEnableGroup(IN): Enable which group. The size must be same as number of group packed in pack bin. Assign NULL will enable all the group.
// nMaxTime(IN): The maximum buffer length in number of frames for keeping the status of commands.
// lpbyState(IN/OUT): State buffer for recognizer.
// nStateSize(IN): Size in bytes of the state buffer lpbyState.
// lpnErr(OUT): DSPOTTER_SUCCESS indicates success, else error code. It can be NULL.
// lpchLicenseFile(IN): The license file.
// Return: A recognizer handle or NULL.
DSPDLL_API HANDLE DSpotterInitMultiWithPackBin(char *lpchPackBin, BOOL *lpbEnableGroup, INT nMaxTime, BYTE *lpbyState, INT nStateSize, INT *lpnErr, char *lpchLicenseFile);

// Purpose: Create a recognizer.
// lpchCYBaseFile(IN): The background model, contents of CYBase.mod.
// lppchGroupFile(IN): The group model, contents of Group_X.mod.
// nNumGroupFile(IN): The number of group model.
// nMaxTime(IN): The maximum buffer length in number of frames for keeping the status of commands.
// lpbyState(IN/OUT): State buffer for recognizer.
// nStateSize(IN): Size in bytes of the state buffer lpbyState.
// lpnErr(OUT): DSPOTTER_SUCCESS indicates success, else error code. It can be NULL.
// lpchLicenseFile(IN): The license file.
// Return: A recognizer handle or NULL.
DSPDLL_API HANDLE DSpotterInitMultiWithMod(char *lpchCYBaseFile, char *lppchGroupFile[], INT nNumGroupFile, INT nMaxTime, BYTE *lpbyState, INT nStateSize, INT *lpnErr, char *lpchLicenseFile);

// Purpose: Destroy a recognizer (free resources).
// hDSpotter(IN): A handle of the recognizer.
// Return: Success or error code.
DSPDLL_API INT DSpotterRelease(HANDLE hDSpotter);

// Purpose: Reset recognizer.
// hDSpotter(IN): A handle of the recognizer.
// Return: Success or error code.
DSPDLL_API INT DSpotterReset(HANDLE hDSpotter);

// Purpose: Transfer voice samples to the recognizer for recognizing.
// hDSpotter(IN): A handle of the recognizer.
// lpsSample(IN):  The pointer of voice data buffer.
// nNumSample(IN): The number of voice data (a unit is a short, we prefer to add 480 samples per call).
// Return: "DSPOTTER_ERR_NeedMoreSample" indicates call this function again, else call DSpotterGetResult(...).
DSPDLL_API INT DSpotterAddSample(HANDLE hDSpotter, SHORT *lpsSample, INT nNumSample);

// Purpose: Get recognition results.
// hDSpotter(IN): A handle of the recognizer.
// Return: The command ID. It is 0 based.
DSPDLL_API INT DSpotterGetResult(HANDLE hDSpotter);
DSPDLL_API INT DSpotterGetResultEPD(HANDLE hDSpotter, INT *lpnWordDura, INT *lpnEndDelay);
DSPDLL_API INT DSpotterGetResultScore(HANDLE hDSpotter, INT *lpnGMM, INT *lpnSG, INT *lpnFIL);
DSPDLL_API INT DSpotterGetResultCmdModel(HANDLE hDSpotter, INT *lpnModelID, INT *lpnTagID);
DSPDLL_API INT DSpotterGetResultMapID(HANDLE hDSpotter);

DSPDLL_API INT DSpotterSetResultMapID_Sep(HANDLE hDSpotter, USHORT *lpnMapID);
DSPDLL_API INT DSpotterSetResultMapID_Multi(HANDLE hDSpotter, USHORT **lppnMapID, INT nNumMapID);

DSPDLL_API INT DSpotterSetEnableNBest(HANDLE hDSpotter, INT bEnable);
DSPDLL_API INT DSpotterGetNBestScore(HANDLE hDSpotter, INT lpnCmdIdx[], INT lpnScore[], INT nMaxNBest);

DSPDLL_API INT DSpotterGetNumWordWithPackBin(char *lpchPackBin, INT lpnNumWord[]);
DSPDLL_API INT DSpotterGetNumWordWithMod(char *lpchGroupFile);

DSPDLL_API const char *DSpotterVerInfo(void);

/************************************************************************/
//  Threshold Adjust API                                                                   
/************************************************************************/
// Purpose: Set model rejection level.
// hDSpotter(IN): A handle of the recognizer.
// nRejectionLevel(IN): The rejection level.
// Return: Success or error code.
DSPDLL_API INT DSpotterSetRejectionLevel(HANDLE hDSpotter, INT nRejectionLevel);

// Purpose: Set model SG rejection level.
// hDSpotter(IN): A handle of the recognizer.
// nRejectionLevel(IN): The rejection level.
// Return: Success or error code.
DSPDLL_API INT DSpotterSetSgLevel(HANDLE hDSpotter, INT nSgLevel);

// Purpose: Set model FIL rejection level
// hDSpotter(IN): a handle of the recognizer
// nRejectionLevel(IN): rejection level
// Return: Success or error code
DSPDLL_API INT DSpotterSetFilLevel(HANDLE hDSpotter, INT nFilLevel);

// Purpose: Set engine response time.
// hDSpotter(IN): A handle of the recognizer.
// nResponseTime(IN): The response time.
// Return: Success or error code.
DSPDLL_API INT DSpotterSetResponseTime(HANDLE hDSpotter, INT nResponseTime);

// Purpose: Set Cmd response time.
// hDSpotter(IN): A handle of the recognizer.
// nCmdIdx(IN): The command ID. It is 0 based.
// nOffset(IN): The response time.
// Return: Success or error code.
DSPDLL_API INT DSpotterSetCmdResponseTime(HANDLE hDSpotter, INT nCmdIdx, INT nResponseTime);

#ifdef __cplusplus
}
#endif

#endif // __DSPOTTER_API_H
