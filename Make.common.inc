.PHONY: help
help:
	@make --print-data-base --question no-such-target | \
	grep -v -e '^no-such-target' -e '^makefile' |       \
	awk '/^[^.%][-A-Za-z0-9_]*:/                        \
	    { print substr($$1, 1, length($$1)-1) }' |      \
	sort |                                              \
	pr --omit-pagination --width=80 --columns=4         \



$(DEST)/%.d: %.c
	@echo generating dependences of $< ...
	@test -d ${DEST} || mkdir ${DEST}; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$;                      \
	sed 's,\($*\)\.o[ :]*,$(DEST)/\1 $(DEST)/\1.o $@ : ,g' < $@.$$$$ > $@;     \
	rm -f $@.$$$$

$(DEST)/%.o: %.c
	$(CC) ${CPPFLAGS} $(CFLAGS) -c $(filter %.c, $^) -o $@


${DEST}/%:	%.o
	$(CC) $(LDFLAGS) $(filter %.o, $^) -o $@ $(LDLIBS)

${DEST}/%:	%.c
	$(CC) ${CPPFLAGS} $(CFLAGS) $(LDFLAGS) $(filter %.c, $^) -o $@ $(LDLIBS)


