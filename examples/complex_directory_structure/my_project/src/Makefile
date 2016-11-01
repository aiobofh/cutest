OBJS=$(addprefix ../obj/,$(subst .c,.o,$(wildcard *.c)))

all: $(OBJS)

../obj/%.o: %.c call.h
	@$(CC) -c $< -o $@

call.h: # To avoid warnings in your text editor for call() macros
	@echo "#ifndef _CALL_H_" > $@
	@echo "#define call" >> $@
	@echo "#endif" >> $@

clean:
	@$(RM) -f $(OBJS) *~ call.h
