CC		:= $(PREFIX)gcc
#CXX	:= $(PREFIX)g++
AS		:= $(CC)
#LD		:= $(CXX)
LD		:= $(CC)

ifdef O
BUILD_DIR=$(O)
else
BUILD_DIR=BUILD_$(LOCAL_MODULE)_EXEC
endif

#-g -rdynamic
CFLAGS		:= $(LOCAL_CFLAGS) $(addprefix -I,$(LOCAL_C_INCLUDES)) -Wall -D_GNU_SOURCE -std=c99 -fms-extensions
LDFLAGS		:= $(LOCAL_LDFLAGS) $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIBRARIES)))

#OBJSC		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CFILES))
#OBJSCXX := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CXXFILES))
#OBJS := $(OBJSCXX) $(OBJSC)

OBJS		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CFILES))
OBJ_DIRS	:= $(sort $(dir $(OBJS)))


.PHONY: all checkdirs clean rebuild

TARGET		:= $(BUILD_DIR)/$(LOCAL_MODULE)

all: checkdirs $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJS): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#$(OBJSC): $(BUILD_DIR)/%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@
	
#$(OBJSCXX): $(BUILD_DIR)/%.o: %.cpp
#	$(CXX) $(CFLAGS) -c $< -o $@

checkdirs: $(OBJ_DIRS)

$(OBJ_DIRS):
	mkdir -p $@

install:
	$(CP) $(TARGET) $(ROOTFS_BIN_DIR)

clean:
	rm -rf $(OBJ_DIRS)

rebuild: clean all install

.PHONY:	all install clean

