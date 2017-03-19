CC := gcc
CFLAGS := -g -std=c99
obj = http.o bitmap.o cookie.o
ifneq ($(MAKECMDGOALS), tester)
target := dingloader
obj += main.o
else
target := tester
obj += tester.o
endif

$(target) : $(obj)
	#echo $(obj)
	$(CC) $(CFLAGS) -o $@ $^

$(obj) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

run :
	./main

clean:
	@-rm *.o
	@-rm $(target)
	@echo Done.
