STRIP=$(CROSS)-strip --remove-section=.note --remove-section=.comment
#CFLAGS += -W -Wall -Os  -I../common/share -I../common/md5Common -I../common/libOutput -I../common/libSysMonitor
CFLAGS = -W -Wall -g  -I../common/share -I../common/libOutput -I../common/libSysMonitor
LIBS   += -L. -L../common/libSysMonitor -L../common/libOutput  -loutputLib -ltaskLib

OUTPUT=output
OUTPUT_OBJS=outputTask.o


ifeq ($(DEBUG),1)
	CFLAGS+=-DDEBUG
endif

all: $(OUTPUT)

$(OUTPUT):$(OUTPUT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(OUTPUT_OBJS) $(LIBS)
#	$(STRIP) $(OUTPUT)

clean:
	rm -rf $(OUTPUT)
	rm -rf *.o	

