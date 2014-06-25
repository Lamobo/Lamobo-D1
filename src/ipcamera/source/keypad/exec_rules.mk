CC		:= $(PREFIX)gcc
AS		:= $(CC)
LD		:= $(CC)

ifdef O
BUILD_DIR=$(O)
else
BUILD_DIR=BUILD_$(LOCAL_MODULE)_EXEC
endif

CFLAGS		:= $(LOCAL_CFLAGS) $(addprefix -I,$(LOCAL_C_INCLUDES)) -Wall
LDFLAGS		:= $(LOCAL_LDFLAGS) $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIBRARIES)))

OBJS		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(LOCAL_SRC_FILES))
OBJ_DIRS	:= $(sort $(dir $(OBJS)))


.PHONY: all checkdirs clean rebuild

TARGET		:= $(BUILD_DIR)/$(LOCAL_MODULE)

all: checkdirs $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJS): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	$(CP) $(TARGET) $(ROOTFS_BIN_DIR)

checkdirs: $(OBJ_DIRS)

$(OBJ_DIRS):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIRS)

rebuild: clean all install
