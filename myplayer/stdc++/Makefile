CURRENT_PATH = $(shell pwd)
CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

PROJECT_PATH = $(CURRENT_PATH)/../../stdc++/zk_full/jni/

# config hdmi or panel display
CONFIG_SUPPORT_HDMI = 

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm -g -funwind-tables -ffunction-sections -rdynamic
C_FLAGS   = $(COM_FLAGS) -std=c11
CPP_FLAGS = $(COM_FLAGS) -std=c++11 

INCLUDES  = -I. -I../include -I$(PROJECT_PATH)/sdkdir/include -I$(PROJECT_PATH)/player/ffmpeg/include

LIB_PATH  = -L. -L../lib -L$(PROJECT_PATH)/sdkdir/lib -L$(PROJECT_PATH)/player/ffmpeg/lib

TARGET_NAME  = MyPlayer

CPP_SRCS  =  $(wildcard $(CURRENT_PATH)/../app/*.cpp)
C_SRCS    =  $(wildcard $(CURRENT_PATH)/../src/*.c)

CPP_OBJS  = $(patsubst %.cpp, %.cpp.o, $(CPP_SRCS))
C_OBJS    = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_NAME  = -lavformat -lavcodec -lavutil -lswscale -lswresample
LIB_NAME += -pthread -lm -lmi_vdec -lmi_sys -lmi_divp -lmi_disp -lmi_ao -lmi_gfx -ldl -lmi_common
LIB_NAME += -lcrypto -lnl -lssl


ifeq ($(CONFIG_SUPPORT_HDMI),enable)
C_FLAGS   += -DSUPPORT_HDMI
CPP_FLAGS += -DSUPPORT_HDMI
LIB_NAME  += -lmi_hdmi
else
LIB_NAME  += -lmi_panel
endif

.PHONY: all prepare clean

all: prepare $(TARGET_NAME) finish

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo

clean:
	@rm -rf $(CPP_OBJS)
	@rm -rf $(C_OBJS)
	@rm -rf $(TARGET_NAME)

finish:
	@echo "make done"
	@echo "<<<<========================================================"
	@echo

$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CPP) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) $(CPP_FLAGS) $(INCLUDES) -c $< -o $@

