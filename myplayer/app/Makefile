CURRENT_PATH = $(shell pwd)
CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

OUT = $(CURRENT_PATH)/out
STR ?= 0

COM_FLAGS = -Wall -O2 -fPIC -mtune=cortex-a7 -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -O2 -mthumb-interwork -marm -g -funwind-tables -ffunction-sections -rdynamic
C_FLAGS   = $(COM_FLAGS) -std=gnu99
CPP_FLAGS = $(COM_FLAGS) -std=gnu99 

INCLUDES = -I. -I../include -I../ffmpeg/include 
LIB_PATH = -L. -L../lib -L../ffmpeg/dynamic

ifeq ($(STR),1)
INCLUDES += -I../../stdc++/zk_full_str/jni/player/ipcserver -I../../stdc++/zk_full_str/jni/sdkdir/include
LIB_PATH += -L../../stdc++/zk_full_str/jni/sdkdir/lib
else
INCLUDES += -I../../stdc++/zk_full_cocheer/jni/player/ipcserver -I../../stdc++/zk_full_cocheer/jni/sdkdir/include
LIB_PATH += -L../../stdc++/zk_full_cocheer/jni/sdkdir/lib
endif

SOURCE_PATH = $(CURRENT_PATH)

TARGET_NAME  = MyPlayer

C_SRCS    = $(wildcard $(SOURCE_PATH)/*.c)
C_OBJS    = $(patsubst %.c, %.o, $(C_SRCS))

#CPP_SRCS  =  $(wildcard $(CURRENT_PATH)/*.cpp)
#CPP_OBJS  = $(patsubst %.cpp, %.o, $(CPP_SRCS))

LIB_NAME  = -lssplayer
LIB_NAME += -lssl -lcrypto
LIB_NAME += -lavformat -lavcodec -lavutil -lswscale -lswresample
LIB_NAME += -lmi_vdec -lmi_divp -lmi_panel -lmi_disp -lmi_ao -lmi_gfx -lmi_sys -lmi_common -pthread -lm -ldl

.PHONY: all prepare clean

all: prepare $(TARGET_NAME) finish

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@mkdir -p $(CURRENT_PATH)/out

clean:
	@rm -rf $(C_OBJS)
	@rm -rf $(TARGET_NAME)
	@rm -rf $(OUT)

finish:
	@mv $(SOURCE_PATH)/*.o $(CURRENT_PATH)/out
	@echo "make done"
	@echo "<<<<========================================================"
	@echo

$(TARGET_NAME): $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(LIB_PATH) $(LIB_NAME)

%.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

