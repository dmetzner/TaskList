include config.mk

build: $(PROJECT)

$(PROJECT): $(OBJECTS)
	@echo [LD] $@
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	@echo [CC] $<
	@$(CC) $(CFLAGS) -o $@ -c -MMD $<

run:
	@./$(PROJECT) ${DATABASE} ${PORT}

clean:
	@rm -f $(PROJECT) $(OBJECTS) $(DEPENDS)

prepare:
	@:

-include $(DEPENDS)
