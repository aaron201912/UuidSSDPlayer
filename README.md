# UuidSSDPlayer

IDE:

	基于ZK UI播放器的IDE。
	编译方法：
	1. 使用FlyThings IDE导入IDE工程；
	2. 选中项目，点击右键选择清空项目，构建项目；
	3. 导出生成文件到指定根目录，生成文件包含libzkgui.so和ui等。
		
app:

	用来点屏的app，它会call到IDE中编译出来的so
	编译方法：
	1. 根据需要点的panel修改sstardisp.c打开对应的配置：
	   #define UI_1024_600 1
	2. make clean;make即可
  
tool:

	用来改变触屏分辨率配置的文件：
	echo 1024x600.bin > /sys/bus/i2c/devices/1-005d/gtcfg
	echo 800x480.bin > /sys/bus/i2c/devices/1-005d/gtcfg
