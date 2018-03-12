SUB_DIRS=libOutput libSysMonitor
sall: $(SUB_DIRS)
$(SUB_DIRS):ECHO
	make -C $@
ECHO:
	@echo $(SUB_DIRS)


.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OUTPUT_OBJ) $(OUTPUT_PRG)
	rm -f $(MONITOR_OBJ) $(MONITOR_PRG)
	find . -name "*.o" | xargs rm -f
	find . -name "*.so" | xargs rm -f
