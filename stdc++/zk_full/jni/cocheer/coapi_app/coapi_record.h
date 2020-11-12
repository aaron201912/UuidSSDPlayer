#ifndef __COAPI_RECORD_H_
#define __COAPI_RECORD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cap_feed_cb)(char *buf,int size,int status);

int coapi_record_create(cap_feed_cb cap_cb);
int coapi_record_destroy(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif

