# UuidSSDPlayer

stdc++/uclibc++:

    UuidSSDPlayer为加密版本的全功能播放器，包含链接标准c++库的版本和链接uclibc++的版本，分别对应于stdc++目录和uclibc++目录。
    目录下有libsercurity，app，zk_full三个目录。
    注：新增mp_stdc++目录，记录sdk v008版本对应的demo。

    libsercurity：存放编译libzkgui.so所依赖的zk_sdk头文件和libeasyui.so，以及运行时依赖的libs。随zk_sdk发布来同步更新。
                  注：运行时依赖的libeasyui.so为加密版本的lib，与参与编译的libeasyui.so不同，不可混淆。
                      运行时libeasyui.so有授权版本和水印版本，分别位于libs目录和libs/watermark目录下。
                      授权版本在正式生产版本中使用，依赖芯片的UUID，需要申请授权账号，购买授权次数来进行授权；
                      水印版本用于开发阶段测试使用，不依赖于UUID，无需授权。使用水印版本lib的app在运行时会出现"Powered by Flythings"的水印标记。
                      授权版本和水印版本的切换只需替换对应的libeasyui.so即可。


    app：编译zkgui bin的source code目录，根据使用panel类型和规格来替换屏参，编译适用于平台的zkgui bin。

    zk_full：编译libzkgui.so的source code目录。
	编译方法：
	1. 使用FlyThings IDE导入zk_full工程；
	2. 选中项目，点击右键选择清空项目，构建项目；
	3. 导出生成文件到指定根目录，生成文件包含libzkgui.so和ui等。

tool:

	用来改变触屏分辨率配置的文件：
	echo 1024x600.bin > /sys/bus/i2c/devices/1-005d/gtcfg
	echo 800x480.bin > /sys/bus/i2c/devices/1-005d/gtcfg

myplayer:
    在makefile中通过宏SUPPORT_PLAYER_PROCESS使能跨进程模式, 需要单独编译生成bin文件.
    编译方法:
    1. cd myplayer/stdc++
    2. make clean;make
    3. 将生成的MyPlayer文件拷贝到sdk/verify/application/zk_full_sercurity/bin下

    播放器进程与主进程通讯说明.
    1. 播放器进程间通讯源码在UuidSSDPlayer/myplayer/app/main.cpp中, 主要接收主进程的消息, 并反馈播放状态信息.
    2. 主进程(即zkgui)通讯相关的源码在UuidSSDPlayer/stdc++/zk_full/jni/logic/playerLogic.cc中, 主要函数StartPlayStreamFile, StopPlayStreamFile, PlayFileProc发送消息到播放进程并接收反馈.
    3. 主进程中会通过UI的一些操作调到进程间通讯的接口. 主进程中会通过popen创建一个播放器进程, 退出时销毁播放器进程.

    使用注意事项：
    播放器切换为跨进程模式, SDK版本需要在V008版本上更新“20201224_DISP支持多进程”的RedFlag.
    跨进程模式下使用了心跳包机制, 如果进程间超过5秒没有消息握手, 播放进程会自动退出.
    播放器进程与主进程的版本需要保持一致, 否则可能导致进程间通讯失败.
