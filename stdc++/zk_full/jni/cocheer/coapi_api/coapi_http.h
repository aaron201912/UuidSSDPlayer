#ifndef __COAPI_HTTP_H__
#define __COAPI_HTTP_H__

#ifdef __cpluscplus
extern "C" {
#endif

struct webclient_session
{
    int socket;
    int response;
    char *transfer_encoding;
    int  chunk_sz;
    int  chunk_offset;
    char *content_type;
    int  content_length;
    char *last_modified;
    char *location;
    char *host;
    char *request;
    unsigned int position;
    unsigned int content_length_remainder;  
#ifdef WEBCLIENT_USING_TLS
    MbedTLSSession *tls_session;
#endif
};

typedef void (*webclient_cbk)(char *data, unsigned int datalen);

int coapi_download_start(char *purl,webclient_cbk func_cbk);

int coapi_download_stop(void);

struct webclient_session *coapi_url_open(const char *url);

int coapi_url_close(struct webclient_session *session);

int coapi_url_read(struct webclient_session *session, unsigned char *buffer,unsigned int length);

int coapi_url_write(struct webclient_session *session,const unsigned char *buffer, unsigned int length);

#ifdef __cpluscplus
}
#endif

#endif

