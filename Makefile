SUB_DIRS=common sample monitorTask outputTask
sall: $(SUB_DIRS)
$(SUB_DIRS):ECHO
	make -C $@
ECHO:
	@echo $(SUB_DIRS)

.PRONY:install
install:

	mkdir -p                          bin
	cp -f sample/sample               bin/
	cp -f sample/appDbg               bin/
	cp -f monitorTask/monitor         bin/
	cp -f outputTask/output           bin/
	cp -f common/libOutput/*.so       bin/
	cp -f common/libSysMonitor/*.so   bin/

.PRONY:devinstall
devinstall:	
	mkdir -p /usr/local/lib/pkgconfig
	cp -f common/libOutput/*.so       /usr/local/lib
	cp -f common/libSysMonitor/*.so   /usr/local/lib
	cp -f pkgconfig/libmonitor.pc     /usr/local/lib/pkgconfig 
	ldconfig

	mkdir -p /usr/local/include/monitor
	cp -f common/libSysMonitor/*.h   /usr/local/include/monitor
	cp -f common/share/*.h           /usr/local/include/monitor
	cp -f common/libOutput/*.h       /usr/local/include/monitor	          

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	find . -name "*.o" | xargs rm -f
	find . -name "*.so" | xargs rm -f

	rm -f sample/sample
	rm -f sample/appDbg
	rm -f outputTask/output
	rm -f monitorTask/monitor
	rm -f bin/*
	
