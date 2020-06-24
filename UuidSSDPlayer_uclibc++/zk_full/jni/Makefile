#一般情况无需修改该文件

LOCAL_PATH=$(dir  $(abspath $(lastword $(MAKEFILE_LIST))))
ZKSW_PREBUILT_LIB_PATH := $(ZKSW_REAL_PREBUILT)/lib/
ZKSW_PREBUILT_INCLUDE_PATH := $(ZKSW_REAL_PREBUILT)/include

EASYUI_COMMON_INCLUDE := $(LOCAL_PATH)/include

#配置源文件目录
PROJ_ROOT_PATH  :=./
#源文件后缀名
PROJ_SRC_SUFFIX := %.cpp %.c
PROJ_HEAD_SUFFIX := %.h
# 递归遍历目录下的所有的文件 
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))  
# 获取相应的源文件
PROJ_ALL_FILES := $(foreach src_path,$(PROJ_ROOT_PATH), $(call rwildcard,$(src_path),*.*) )   
PROJ_SRC_FILES := $(filter $(PROJ_SRC_SUFFIX),$(PROJ_ALL_FILES))   
PROJ_HEAD_FILES := $(filter $(PROJ_HEAD_SUFFIX),$(PROJ_ALL_FILES))   

# 获取相应的源文件
LOCAL_SRC_FILES  := $(PROJ_SRC_FILES) 
LOCAL_HEAD_FILES := $(PROJ_HEAD_FILES) 

#源文件目录结构
PROJ_DIR_TREE := $(dir $(foreach src_path, $(LOCAL_SRC_FILES), $(call rwildcard,$(src_path),*/) ) )  
PROJ_DIR_TREE := $(sort $(PROJ_DIR_TREE))  

#wlan功能启用开关
CONFIG_WLAN_SWITCH := "enable"
#CONFIG_WLAN_SWITCH :=

#player功能启用开关
CONFIG_PLAYER_SWITCH := "enable"
#CONFIG_PLAYER_SWITCH :=

#cloudplay功能启用开关
#CONFIG_CLOUD_PLAY_SWITCH := "enable"
CONFIG_CLOUD_PLAY_SWITCH :=

OBJS_ROOT_DIR=../obj/
OBJS_1 = $(patsubst %.cpp,%.o,$(LOCAL_SRC_FILES))
OBJS_2 = $(patsubst %.c,%.o,$(OBJS_1))
OBJS   = $(addprefix $(OBJS_ROOT_DIR),$(OBJS_2))
OBJS_DIR_TREE=$(addprefix $(OBJS_ROOT_DIR),$(PROJ_DIR_TREE))

CFLAGS=-Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=return-type -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc -Wl,-z,defs 
CXXFLAGS=-Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=return-type -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -Wl,-z,defs 
CXXFLAGS+= -fexceptions -fpermissive -Wall -O2

INCLUDE_DIR := -I$(EASYUI_COMMON_INCLUDE) \
					-I$(ZKSW_PREBUILT_INCLUDE_PATH) \
					-I. \
					-I$(ZKSW_PREBUILT_INCLUDE_PATH)/mgeff \
					-I ../jni/sdkdir/include \
					-I ../jni/voicedetect \
					-I ../jni/tts

LDFLAGS =  -leasyui -pthread -lmi_sys -lmi_disp -lmi_panel -lmi_ao -lmi_gfx -lmi_divp -lmi_vdec -lmi_common -lstdc++ -lmi_ai -lDSpotter -lCReader -ldl -lrt -fPIC -shared
LDFLAGS += -L$(ZKSW_PREBUILT_LIB_PATH)/system -L. -L ../jni/sdkdir/lib

INCLUDE_DIR += -I ../jni/player -I ../jni/player/ffmpeg/include -I ../jni/player/mediastream -I ../jni/player/imagedata
INCLUDE_DIR += -I ../jni/v4l2/inc -I ../jni/v4l2/inc/uvc -I ../jni/v4l2/inc/video -I ../jni/v4l2/src -I ../jni/v4l2/src/control -I ../jni/v4l2/src/gcc \
               -I ../jni/v4l2/src/libv4l -I ../jni/v4l2/src/processing
INCLUDE_DIR += -I ../jni/hotplugdetect -I ../jni/hotplugdetect/common -I ../jni/hotplugdetect/usbdetect -I ../jni/hotplugdetect/wifidetect \
			   -I ../jni/hotplugdetect/wirednetworkdetect
INCLUDE_DIR += -I ../jni/airplay/include

ifneq ($(CONFIG_WLAN_SWITCH),)
CFLAGS+=-DSUPPORT_WLAN_MODULE
CXXFLAGS+=-DSUPPORT_WLAN_MODULE

#INCLUDE_DIR += -I ../jni/hotplugdetect/wifidetect
LDFLAGS += -lmi_wlan -lcjson
endif

ifneq ($(CONFIG_PLAYER_SWITCH),)
CFLAGS+=-DSUPPORT_PLAYER_MODULE
CXXFLAGS+=-DSUPPORT_PLAYER_MODULE

LDFLAGS += -lavformat -lavcodec -lavutil -lswscale -lswresample -L../jni/player/ffmpeg/lib
endif

ifneq ($(CONFIG_CLOUD_PLAY_SWITCH),)
CFLAGS+=-DSUPPORT_CLOUD_PLAY_MODULE
CXXFLAGS+=-DSUPPORT_CLOUD_PLAY_MODULE
endif

TARGET=../libs/armeabi/libzkgui.so

ifeq ($(TARGET), $(wildcard $(TARGET)))
	DELETE_TARGET=$(TARGET)
endif

# alias commads if compile on windows
RM =rm
MKDIR=mkdir
ECHO=echo
ifeq ($(OS),Windows_NT)
	RM=win32-rm
	MKDIR=win32-mkdir
	ECHO=win32-echo
else
	#keep default
endif
#alias end

all: prepare $(TARGET)
	@$(ECHO)
	@$(ECHO) "[armeabi] Install        : libzkgui.so => libs/armeabi/libzkgui.so"
	

$(OBJS_ROOT_DIR)%.o: %.cpp $(LOCAL_HEAD_FILES)
	@$(ECHO) "[armeabi] Compile++      : "$< 
	@$(CC) -c $< -o $@ $(CXXFLAGS) $(INCLUDE_DIR) $(LDFLAGS) 
	
$(OBJS_ROOT_DIR)%.o: %.c $(LOCAL_HEAD_FILES)
	@$(ECHO) "[armeabi] Compile++      : "$< 
	@$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_DIR) $(LDFLAGS) 

$(TARGET):$(OBJS) 
	@$(ECHO) "[armeabi] SharedLibrary  : "$@ 
#	@$(CC) -fPIC -shared $^ -o $@ $(LDFLAGS) $(CXXFLAGS)
	@$(CC)  $^ -o $@ $(LDFLAGS) $(CXXFLAGS)
	
prepare: 
	@$(ECHO) "[armeabi] GCC            : "$(CC) 
	@-$(MKDIR) ../libs/armeabi -p 
	@-$(MKDIR) $(OBJS_DIR_TREE) -p 
	@-$(RM) $(TARGET) -rf 
	
clean:
	@$(ECHO) "[armeabi] Clean          : "$(OBJS_ROOT_DIR) 
	@-$(RM)  $(OBJS_ROOT_DIR) -rf 
	@$(ECHO) "[armeabi] Clean          : "$(TARGET)
	@-$(RM)  $(TARGET) -rf 

.PHONY:all clean prepare $(TARGET)
