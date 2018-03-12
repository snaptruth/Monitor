STRIP=$(CROSS)-strip --remove-section=.note --remove-section=.comment
#CFLAGS += -W -Wall -Os  -I../common/share -I../common/md5 -I../common/libOutput -I../common/libSysMonitor
CFLAGS = -W -Wall -g  -I../common/share -I../common/libOutput -I../common/libSysMonitor
LIBS   += -L. -L../common/libSysMonitor -L../common/libOutput -loutputLib -ltaskLib

MONITOR=monitor
MONITOR_OBJS=monitorTask.o


ifeq ($(DEBUG),1)
	CFLAGS+=-DDEBUG
endif

all: $(MONITOR)

$(MONITOR):$(MONITOR_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MONITOR_OBJS) $(LIBS)
#	$(STRIP) $(MONITOR)


clean:
	rm -rf $(MONITOR)
	rm -rf *.o	

