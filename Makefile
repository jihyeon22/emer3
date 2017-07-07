
###############################################################################
# Path

DESTDIR		:= $(CURDIR)/out

PREFIX		:= /system
BINDIR		:= $(PREFIX)/sbin
INITRCDIR	:= /etc/init.d

###############################################################################
# Compile

CC	:= $(CROSS_COMPILE)gcc

CFLAGS	= $(EXTRA_CFLAGS)
LDFLAGS	= $(EXTRA_LDFLAGS)

###############################################################################
# Options

#AUTOSTART	:= y

###############################################################################
# Board

BOARD	:= 
CFLAGS	+= -DBOARD_$(BOARD)

ifeq ($(NO_USE_NETWORK),1)
CFLAGS += -DNO_USE_NETWORK
endif

###############################################################################
# Target rules

CFLAGS	+= 
LDFLAGS	+= -lpthread -lat3 -llogd -lmdsapi

OBJS	:= src/emer3.o src/sms_cmd.o src/netcheck.o
APP		:= emer3

all: all-before	$(APP)

.c.o:
	$(CC) $(CFLAGS)  -c $< -o $@
all-before:
	$(eval $(call checkboard))
		
$(APP):	$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

install:	install-binary

install-binary:	$(APP)
	$(Q)$(call check_install_dir, $(DESTDIR)$(BINDIR))
	$(Q)fakeroot cp -v $(APP) $(DESTDIR)$(BINDIR)/$(APP)

clean:
	$(Q)rm -vrf $(APP) $(OBJS) 

uninstall:
	$(Q)rm -vrf $(DESTDIR)$(BINDIR)/$(APP)


###############################################################################
# Functions

define check_install_dir
	if [ ! -d "$1" ]; then mkdir -p $1; fi
endef

define checkboard
ifeq ($(BOARD),)
$$(error BOARD is not found, BOARD=NEO_W100/NEO_W200)
endif
endef
