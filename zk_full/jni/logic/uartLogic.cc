#pragma once
#include "uart/ProtocolSender.h"
#include "uart/UartContext.h"
#include "manager/ConfigManager.h"
#include "manager/LanguageManager.h"
#include <termio.h>
#include "statusbarconfig.h"

#define TYPE_HEX  "16进制"
#define TYPE_TEXT "文本"

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    //{0,  6000}, //定时器id=0, 时间间隔6秒
    //{1,  1000},
};

static void readHook(const BYTE* data, int len) {
    if (strcmp(EASYUICONTEXT->currentAppName(), "uartActivity") != 0) {
        return;
    }

    static int lines = 0;
    string s = "";
    char buf[8] = {0};
    for (int i = 0; i < len; ++i) {
      snprintf(buf, sizeof(buf), "%02x ", data[i]);
      s = s + buf;
    }

    if (lines < 20) {
        s = mTextviewLogPtr->getText() + "\n" + s;
        ++lines;
    } else {
        lines = 0;
    }
    mTextviewLogPtr->setText(s);
}

static const char* getBaudRate(UINT baudRate) {
    struct {
        UINT baud;
        const char *pBaudStr;
    } baudInfoTab[] = {
        { B4800, "B4800" },
        { B9600, "B9600" },
        { B19200, "B19200" },
        { B38400, "B38400" },
        { B57600, "B57600" },
        { B115200, "B115200" },
        { B230400, "B230400" },
        { B460800, "B460800" },
        { B921600, "B921600" }
    };

    int len = sizeof(baudInfoTab) / sizeof(baudInfoTab[0]);
    for (int i = 0; i < len; ++i) {
        if (baudInfoTab[i].baud == baudRate) {
            return baudInfoTab[i].pBaudStr;
        }
    }

    return NULL;
}

/**
 * 当界面构造时触发
 */
static void onUI_init(){


   char buf[128] = {0};
   snprintf(buf, sizeof(buf), "%s： %s  %s：%s",
       LANGUAGEMANAGER->getValue("serial_port").c_str(),
       CONFIGMANAGER->getUartName().c_str(),
       LANGUAGEMANAGER->getValue("baud_rate").c_str(),
       getBaudRate(CONFIGMANAGER->getUartBaudRate()));
   mTextviewBaundPtr->setText(buf);
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
    UARTCONTEXT->addReadHook(readHook);
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
    UARTCONTEXT->removeReadHook();
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	ShowStatusBar(1, 0, 0);
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id){
    switch (id) {

        default:
            break;
    }
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onuartActivityTouchEvent(const MotionEvent &ev) {
    switch (ev.mActionStatus) {
        case MotionEvent::E_ACTION_DOWN://触摸按下
            //LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
            break;
        case MotionEvent::E_ACTION_MOVE://触摸滑动
            break;
        case MotionEvent::E_ACTION_UP:  //触摸抬起
            break;
        default:
            break;
    }
    return false;
}
static void onEditTextChanged_Edittext(const std::string &text) {

}

static bool onButtonClick_ButtonSendType(ZKButton *pButton) {
    if (pButton->getText().compare(LANGUAGEMANAGER->getValue("hex")) == 0) {
        pButton->setText(LANGUAGEMANAGER->getValue("text"));
    } else {
        pButton->setText(LANGUAGEMANAGER->getValue("hex"));
    }
    return false;
}

/**
* 将16进制字符串转为字节数组， 遇到异常字符停止转换，返回转换成功的字节数
*/
static int HexStr2Int(const char* data, unsigned char* out, int out_len) {
  int len = strlen(data);
  int offset = 0;
  int index = 0;

  if (out == NULL) {
    return 0;
  }

  while (offset < len) {

    if (index >= out_len) {
      break;
    }
    char* end = NULL;
    long h = strtol(data + offset, &end, 16);

    if (*end == '\0') {
      //结束
      out[index] = h;
      break;
    }

    //出错
    if ((h == 0) && (end == (data + offset))) {
      mTextviewErrPtr->setVisible(true);
      return index;
    }

    out[index] = h;
    index = index + 1;
    offset = end - data;
  }
  return index + 1;
}

static bool onButtonClick_ButtonSend(ZKButton *pButton) {
    //mTextviewErrPtr->setVisible(false);
    //if (mEdittextPtr->getText().length() < 1) {
    //    return false;
    //}

    //if (mButtonSendTypePtr->getText().compare(LANGUAGEMANAGER->getValue("text")) == 0) {
    //    sendRaw((BYTE*)mEdittextPtr->getText().c_str(), mEdittextPtr->getText().size());
    //} else {
    //    unsigned char buffer[1024] = {0};
    //    int size = HexStr2Int(mEdittextPtr->getText().c_str(), buffer, sizeof(buffer));
    //    for (int i = 0; i < size; ++i) {
    //      LOGD(" atoi() = %02x", buffer[i]);
    //    }
    //    sendRaw(buffer, size);
    //}
    return false;
}
static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
static bool onButtonClick_ButtonClean(ZKButton *pButton) {
    mTextviewLogPtr->setText("");
    return false;
}
