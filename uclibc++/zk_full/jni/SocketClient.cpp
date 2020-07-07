/*
 * SocketClient.cpp
 *
 *  Created on: Aug 9, 2017
 *      Author: guoxs
 */

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <time.h>
#include <sys/time.h>

#include "SocketClient.h"
#include "utils/Log.h"

#define HELLO_WORLD_SERVER_PORT    	30000
#define BUFFER_SIZE 				4096
#define FILENAME_MAX_SIZE 			512

static int rtcSetTime(const struct tm *tm_time) {
    int rtc_handle = -1;
	int ret = 0;
	struct rtc_time rtc_tm;
	if (tm_time == NULL) {
	    return  -1;
	}

    rtc_handle = open("/dev/rtc0", O_RDWR);
	if (rtc_handle < 0) {
		printf("open /dev/rtc0 fail\n");
		return  -1;
	}

	memset(&rtc_tm, 0, sizeof(rtc_tm));
	rtc_tm.tm_sec   = tm_time->tm_sec;
	rtc_tm.tm_min   = tm_time->tm_min;
	rtc_tm.tm_hour  = tm_time->tm_hour;
	rtc_tm.tm_mday  = tm_time->tm_mday;
	rtc_tm.tm_mon   = tm_time->tm_mon;
	rtc_tm.tm_year  = tm_time->tm_year;
	rtc_tm.tm_wday  = tm_time->tm_wday;
	rtc_tm.tm_yday  = tm_time->tm_yday;
	rtc_tm.tm_isdst = tm_time->tm_isdst;
	ret = ioctl(rtc_handle, RTC_SET_TIME, &rtc_tm);
    if (ret < 0) {
        printf("rtcSetTime fail\n");
        close(rtc_handle);
        return -1;
    }

	printf("rtc_set_time ok\n");
	close(rtc_handle);

	return 0;
}

static int setDateTime(struct tm* ptm) {
	time_t timep;
	struct timeval tv;
	ptm->tm_wday = 0;
	ptm->tm_yday = 0;
	ptm->tm_isdst = 0;
	timep = mktime(ptm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;

	if (settimeofday(&tv, NULL) < 0) {
		printf("Set system date and time error, errno(%d)", errno);
		return -1;
	}

	time_t t = time(NULL);
	struct tm *local = localtime(&t);

	rtcSetTime(local);

	return 0;
}

static int setDateTime(const char *pDate) {
	LOGD("setDateTime pDate: %s\n", pDate);

	struct tm _tm;
	struct timeval tv;
	time_t timep;

	sscanf(pDate, "%d-%d-%d %d:%d:%d",
			&_tm.tm_year, &_tm.tm_mon,
			&_tm.tm_mday, &_tm.tm_hour, &_tm.tm_min, &_tm.tm_sec);

	_tm.tm_mon -= 1;
	_tm.tm_year -= 1900;

	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;

	return setDateTime(&_tm);
}

static void* socketThreadRx(void *lParam) {
	((Uploader *) lParam)->threadLoop();
	return NULL;
}

Uploader::Uploader() :
	mClientSocket(-1),
	mSocketListener(NULL) {

}

Uploader::~Uploader() {
	stop();
}

void Uploader::start() {
	if (mClientSocket > 0) {
		LOGD("socket thread had run...\n");
		return;
	}

	pthread_t threadID = 0;
	pthread_attr_t attr; 		// 线程属性
	pthread_attr_init(&attr);  	// 初始化线程属性
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);      // 设置线程属性
	int ret = pthread_create(&threadID, &attr, socketThreadRx, this);
	pthread_attr_destroy(&attr);
	if (ret || !threadID) {
		LOGD("create socket thread error, erro=%s\n", strerror(errno));
		Disconnect();
		return;
	}

	LOGD("create socket thread success!\n");
}

void Uploader::stop() {
	Disconnect();
}

bool Uploader::Connect() {
	// 设置一个socket地址结构clientAddr,代表客户机internet地址, 端口
	struct sockaddr_in clientAddr;
	bzero(&clientAddr, sizeof(clientAddr)); // 把一段内存区的内容全部设置为0
	clientAddr.sin_family = AF_INET;    	// internet协议族
	clientAddr.sin_addr.s_addr = htons(INADDR_ANY);// INADDR_ANY表示自动获取本机地址
	clientAddr.sin_port = htons(0);    // 0表示让系统自动分配一个空闲端口
	// 创建用于internet的流协议(TCP)socket,用clientSocket代表客户机socket
	mClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	LOGD("Create Socket clientSocket: %d\n", mClientSocket);
	if (mClientSocket < 0) {
		LOGD("Create Socket Failed!\n");
		Disconnect();
		return false;
	}

	// 把客户机的socket和客户机的socket地址结构联系起来
	if (bind(mClientSocket, (struct sockaddr*) &clientAddr, sizeof(clientAddr))) {
		LOGD("Client Bind Port Failed!\n");
		Disconnect();
		return false;
	}

	LOGD("Client Bind OK!\n");
	// 设置一个socket地址结构serverAddr,代表服务器的internet地址, 端口
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;

	if (inet_aton(SERVER_IP_ADDR, &serverAddr.sin_addr) == 0) {     // 服务器的IP地址来自程序的参数
		LOGD("Server IP Address Error!\n");
		Disconnect();
		return false;
	}

	serverAddr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
	socklen_t serverAddrLength = sizeof(serverAddr);
	// 向服务器发起连接,连接成功后clientSocket代表了客户机和服务器的一个socket连接
	if (::connect(mClientSocket, (struct sockaddr *) &serverAddr, serverAddrLength) < 0) {
		LOGD("Can Not Connect To %s!\n", SERVER_IP_ADDR);
		Disconnect();
		return false;
	}

	LOGD("connect %s success!\n", SERVER_IP_ADDR);

//	struct timeval timeout = { 1, 0 };     // 1s
//	int ret = setsockopt(mClientSocket, SOL_SOCKET, SO_RCVTIMEO,
//			(const char*)&timeout, sizeof(timeout));

	return true;
}

bool Uploader::Disconnect() {
	LOGD("SocketClient disconnect\n");
	if (mClientSocket > 0) {
		LOGD("SocketClient close socket...\n");
		// 关闭socket
		close(mClientSocket);
		mClientSocket = -1;
	}

	return true;
}

void Uploader::threadLoop() {
	if (!Connect()) {
		LOGD("socket thread connect error return!\n");
		return;
	}

	// 同步时间
/*	char timeStr[20] = { 0 };
	int len = read(mClientSocket, timeStr, 20);
	LOGD("threadLoop len: %d, timeStr: %s\n", len, timeStr);
	char dateStr[40] = { 0 };
	sprintf(dateStr, "date -s\"%s\"", timeStr);

	system(dateStr);*/

	// 同步时间
	char timeStr[20] = { 0 };
	int len = read(mClientSocket, timeStr, 20);
	if (len > 0) {
		setDateTime(timeStr);

		if (mSocketListener != NULL) {
			mSocketListener->notify(0, E_SOCKET_STATUS_UPDATE_DATE, "");
		}
	}

	struct timeval timeout = { 1, 0 };     // 1s
	int ret = setsockopt(mClientSocket, SOL_SOCKET, SO_RCVTIMEO,
			(const char*)&timeout, sizeof(timeout));

	while (mClientSocket > 0) {
		char fileType[10] = { 0 };

		int length = read(mClientSocket, fileType, 3);
		LOGD("length %d, fileType %s\n", length, fileType);
		if (length < 0) {
			if (errno != EAGAIN) {
				LOGD("read fileType error %s\n", strerror(errno));
				Disconnect();
				break;
			}
			continue;
		} else if (length == 0) {
			LOGD("read length is 0......%s\n", strerror(errno));
			Disconnect();
			break;
		}


		int fileSize = 0;
		length = read(mClientSocket, &fileSize, sizeof(int));

		if (length > 0) {
			LOGD("read fileSize: %d\n", fileSize);
			if (fileSize <= 0) {
				continue;
			}

			if (mSocketListener != NULL) {
				mSocketListener->notify(fileSize / BUFFER_SIZE, E_SOCKET_STATUS_START_RECV, fileType);
			}

			bool ret = true;
			const char *pFile = (strcmp(fileType, RECV_TYPE_IMG) == 0) ? UPGRADE_FILE_PATH : BUFFER_FILE_NAME;
			FILE *fp = fopen(pFile, "w");
			if (fp != NULL) {
				int readLen = 0;
				char buffer[BUFFER_SIZE] = { 0 };

				int recvLen = 0;

				while (fileSize > 0) {
					readLen = (fileSize > BUFFER_SIZE) ? BUFFER_SIZE : fileSize;
					length = read(mClientSocket, buffer, readLen);
					if (length < 0) {
						LOGE("Recieve Data From Server %s Failed! %s\n", SERVER_IP_ADDR, strerror(errno));
						ret = false;
						break;
					} else if (length == 0) {
						continue;
					}

					int writeLength = fwrite(buffer, sizeof(char), length, fp);
					if (writeLength < length) {
						LOGD("File:\t%s Write Failed\n", pFile);
						ret = false;
//						break;
					} else {
						recvLen += length;
						if (mSocketListener != NULL) {
							mSocketListener->notify(recvLen / BUFFER_SIZE, E_SOCKET_STATUS_RECVING, fileType);
						}
					}

					fileSize -= length;

					bzero(buffer, length);
				}

				if (fclose(fp) != 0) {
					ret = false;
				}

				if (mSocketListener != NULL) {
					mSocketListener->notify(0, ret ? E_SOCKET_STATUS_RECV_OK : E_SOCKET_STATUS_RECV_ERROR, pFile);
				}
			} else {
				LOGE("%s open fail, erro: %s\n", pFile, strerror(errno));
			}
		} else if (length < 0) {
			if (errno != EAGAIN) {
				LOGD("read fileSize error %s\n", strerror(errno));
				Disconnect();
				break;
			}
		} else if (length == 0) {
			LOGD("read length is 0......%s\n", strerror(errno));
			Disconnect();
			break;
		}
	}

	LOGD("socket thread end.\n");
}
