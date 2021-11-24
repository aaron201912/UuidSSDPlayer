## myplayer
    1. 功能说明
    本播放器demo需配合zkgui应用使用.播放器进程作为客户端,通过socket进程间通讯,接收zkgui主进程发送的命令,实现播放/结束/快进/快退等操作.

    2. 目录介绍
    app: 客户端应用源码
    ffmpeg: 包含ffmpeg头文件以及动态链接库文件,源码参考: https://github.com/aaron201912/ffmpeg/tree/master/ffmpeg-4.1.3
    lib: 包含openssl, dns相关的第三方库.其中libssplayer.so是播放器逻辑相关的动态库,源码实现参考: https://github.com/aaron201912/ffmpeg/tree/master/ffplayer
    include: 包含播放器相关的头文件

    3. 编译方法：
    cd app
    make clean; make

    4. 运行说明
    将编译生成的MyPlayer执行档拷贝到板端/customer目录下,将lib目录下的动态库拷贝到/customer/lib目录下并export.运行zkgui打开播放器即可

