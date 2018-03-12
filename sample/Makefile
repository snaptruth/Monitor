STRIP=$(CROSS)-strip --remove-section=.note --remove-section=.comment
#CFLAGS += -W -Wall -Os -g  -I../common/share -I../common/md5 -I../common/libOutput -I../common/libSysMonitor
CFLAGS = -W -Wall  -g  -I../common/share -I../common/libOutput -I../common/libSysMonitor
LIBS   += -L. -L../common/libSysMonitor -L../common/libOutput  -loutputLib -ltaskLib -ldl 

APP=sample
APP_OBJS=main.o public.o dbg_message_process.o 

APP_DBG=appDbg
APP_DBG_OBJS=appDbg.o public.o

ifeq ($(DEBUG),1)
	CFLAGS+=-DDEBUG
endif

all: $(APP) $(APP_DBG)

$(APP):$(APP_OBJS)
	$(CC) $(CFLAGS) -o $@ $(APP_OBJS) $(LIBS)
#	$(STRIP) $(APP)

$(APP_DBG):$(APP_DBG_OBJS)
	$(CC) $(CFLAGS) -o $@ $(APP_DBG_OBJS) $(LIBS)
#	$(STRIP) $(APP_DBG)

clean:
	rm -rf $(APP)
	rm -rf $(APP_DBG)
	rm -rf *.o	

