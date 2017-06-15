OBJS=$(addprefix ../obj/,$(subst .c,.o,$(wildcard *.c)))

all: $(OBJS)

../obj/%.o: %.c call.h
	@$(CC) -c $< -o $@

clean:
	@$(RM) -f $(OBJS) *~ call.h
