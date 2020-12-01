#ifndef __COAPI_APP_H_
#define __COAPI_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "coapi_api/coapi_common.h"

int coapi_register_status(void);
void coapi_init(char *device_id, char *ssid, const char *pAppKey, const char *pSecretKey, dev_comm *pDev);
int coapi_deinit(void);
void dev_bindcode_cb(int status, char *url,char *code);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
