# UuidSSDPlayer

stdc++/uclibc++:

    UuidSSDPlayer为加密版本的全功能播放器，包含链接标准c++库的版本和链接uclibc++的版本，分别对应于std++目录和uclibc++目录。
    目录下有libsercurity，app，zk_full三个目录。

    libsercurity：存放编译libzkgui.so所依赖的zk_sdk头文件和libeasyui.so，以及运行时依赖的libs。随zk_sdk发布来同步更新。
                  注：运行时依赖的libeasyui.so为加密版本的lib，与参与编译的libeasyui.so不同，不可混淆。
                      运行时libeasyui.so分设备授权和手动授权版本，默认适用手动授权版本。

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
    播放器源码, 使能多进程模式时, 需要单独编译生成bin文件. 先运行MyPlayer再运行zkgui.
    编译方法:
    1. cd myplayer/stdc++
    2. make clean;make
    3. 将生成的MyPlayer文件拷贝到sdk/verify/application/zk_full_sercurity/bin下
