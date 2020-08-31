/*
 * SocketClient.h
 *
 *  Created on: Aug 9, 2017
 *      Author: guoxs
 */

#ifndef _SOCKET_SOCKETCLIENT_H_
#define _SOCKET_SOCKETCLIENT_H_


#define SERVER_IP_ADDR			"192.168.43.1"
#define BUFFER_FILE_NAME		"/mnt/extsd/recv.jpg"
#define UPGRADE_FILE_PATH		"/mnt/extsd/update.img"

#define RECV_TYPE_JPG			"jpg"
#define RECV_TYPE_IMG			"img"


class Uploader {
public:
	Uploader();
	virtual ~Uploader();

	void start();
	void stop();

	void threadLoop();

public:
	typedef enum {
		E_SOCKET_STATUS_START_RECV = 0,
		E_SOCKET_STATUS_RECVING,
		E_SOCKET_STATUS_RECV_OK,
		E_SOCKET_STATUS_RECV_ERROR,
		E_SOCKET_STATUS_UPDATE_DATE
	} ESocketStatus;

	class ISocketListener {
	public:
		virtual ~ISocketListener() { };
		virtual void notify(int what, int status, const char *msg) = 0;
	};

	void setSocketListener(ISocketListener *pListener) {
		mSocketListener = pListener;
	}

private:
	bool Connect();
	bool Disconnect();

private:
	int mClientSocket;
	ISocketListener *mSocketListener;
};

#endif /* _SOCKET_SOCKETCLIENT_H_ */
