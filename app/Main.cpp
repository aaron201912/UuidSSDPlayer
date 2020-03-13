#include "entry/EasyUIContext.h"

#include <signal.h>
#include <stdlib.h>

#include "sstardisp.h"

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)

static MI_DISP_PubAttr_t stDispPubAttr;

static void server_on_exit() {
    sstar_disp_Deinit(&stDispPubAttr);
}

static void signal_crash_handler(int sig) {
    exit(-1);
}
 
static void signal_exit_handler(int sig) {
    exit(0);
}

static void installHandler() {
    atexit(server_on_exit);

    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
 
    // ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
 
    signal(SIGBUS, signal_crash_handler);	// 总线错误
    signal(SIGSEGV, signal_crash_handler);	// SIGSEGV，非法内存访�?    signal(SIGFPE, signal_crash_handler);	// SIGFPE，数学相关的异常，如�?除，浮点溢出，等�?    signal(SIGABRT, signal_crash_handler);	// SIGABRT，由调用abort函数产生，进程非正常退�?}
}

int main(int argc, const char *argv[]) 
{
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;

    sstar_disp_init(&stDispPubAttr);

    installHandler();

    if (EASYUICONTEXT->initEasyUI()) {
        EASYUICONTEXT->runEasyUI();
        EASYUICONTEXT->deinitEasyUI();
    }

    return 0;
}
