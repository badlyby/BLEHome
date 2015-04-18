VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION = 1
NAME = Wang Zhong Lei
DATE = 2015/04/16

export KBUILD_BUILDHOST := $(SUBARCH)
ARCH		= ARM
CROSS_COMPILE	= arm-openwrt-linux-

# Make variables (CC, etc...)

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
GENKSYMS	= scripts/genksyms/genksyms
DEPMOD		= /sbin/depmod
KALLSYMS	= scripts/kallsyms
PERL		= perl
CHECK		= sparse

SRC_DIR = src
OBJ_DIR = objs
BIN_DIR = bin
TARGET = weather_home
CPPFLAGS += -I./inc
TARGET_FLAGS = -llua -lm -ldl
COMPILE_FLAGS =

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET_FILE = $(BIN_DIR)/$(TARGET)

all : $(TARGET) jf

$(TARGET): $(OBJS)
	@$(CC) -o $(TARGET_FILE) $(OBJS) $(COMPILE_FLAGS) $(TARGET_FLAGS)&&\
	echo "链接生成:"$(TARGET_FILE)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@$(CC) -c $(CPPFLAGS) $(COMPILE_FLAGS) $(CFLAGS) -o $@ $<&&\
	echo "编译:"$@;

-include $(OBJS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@set -e; rm -f $@;\
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$;\
	rm -f $@

.PHONY : clean

jf : $(TARGET_FILE)
	$(STRIP) $(TARGET_FILE)

clean :
	@rm $(TARGET_FILE) $(OBJS)&&\
	echo clean!

install:
	cp bin/weather_home /usr/bin/
	mkdir /usr/share/weather_home
	cp tools/update.lua /usr/share/weather_home/
	cp tools/weather_home.sh /etc/init.d/
	cp tools/weather_home.conf /etc/init/

uninstall:
	rm /usr/bin/weather_home
	rm -rf /usr/share/weather_home
	rm /etc/init.d/weather_home.sh
	rm /etc/init/weather_home.conf

