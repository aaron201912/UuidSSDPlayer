TARGET = zkgui

PROJECT_PATH = ../zk_full/jni/sdkdir

ZKGUI_PATH=./zkgui_dir
ENABLE_HDMI = 0
COMPILE_ROOT := $(shell pwd)
CC = arm-linux-gnueabihf-g++
#CC = arm-linux-gnueabihf-gcc
STRIP = arm-linux-gnueabihf-strip

CFLAGS = -I$(ZKGUI_PATH)/include -I$(ZKGUI_PATH)/include/freetype2 -I$(ZKGUI_PATH)/include/easyui -I$(ZKGUI_PATH)/include/uClibc++ -I$(PROJECT_PATH)/include
LDFLAGS = -L$(ZKGUI_PATH)/lib -L$(PROJECT_PATH)/lib
CONFIG_PREFIX =.

LOCAL_CFLAGS =

ifeq ($(ENABLE_HDMI), 1)
LOCAL_LDFLAGS = -leasyui -lnanovg -lpng12 -ljpeg -lpthread -lfreetype -ldl -lts -lz -lmi_common -lmi_gfx -lmi_sys -lmi_disp -lmi_panel -lmi_vdec -lmi_ao -lmi_ai -lDSpotter -lCReader -lmi_hdmi -luClibc++ -Wl,--hash-style=gnu
else
LOCAL_LDFLAGS = -leasyui -lnanovg -lpng12 -ljpeg -lpthread -lfreetype -ldl -lts -lz -lmi_common -lmi_gfx -lmi_sys -lmi_disp -lmi_panel -lmi_vdec -lmi_ao -lmi_ai -lDSpotter -lCReader -luClibc++ -Wl,--hash-style=gnu
#LOCAL_LDFLAGS = -leasyui -lnanovg -lpng12 -ljpeg -lpthread -lfreetype -ldl -lts -lz -lmi_common -lmi_gfx -lmi_sys -lmi_disp -lmi_panel -lmi_vdec -lmi_ao -lmi_ai -lDSpotter -lCReader -Wl,--hash-style=gnu
endif

CPPSRCS += ${wildcard *.cpp}
CPPOBJS = $(patsubst %.cpp, %.cpp.o, $(CPPSRCS))

CSRCS += ${wildcard *.c}
COBJS = $(patsubst %.c,%.c.o,$(CSRCS))

all:$(TARGET) install

$(TARGET):$(CPPOBJS) $(COBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^

$(CPPOBJS):%.cpp.o:%.cpp
	$(CC) -DENABLE_HDMI=$(ENABLE_HDMI) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

$(COBJS):%.c.o:%.c
	$(CC) -DENABLE_HDMI=$(ENABLE_HDMI) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

install: 
	-mkdir -p $(CONFIG_PREFIX)/bin
	-mv $(TARGET) $(CONFIG_PREFIX)/bin/
	
clean:
	-rm -rf $(CPPOBJS) $(CONFIG_PREFIX)/bin/
	-rm -rf *.o
.PHONY:all clean $(TARGET) install
