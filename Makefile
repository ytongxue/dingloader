CC := gcc
CFLAGS := -g -std=c99
obj = main.o http.o bitmap.o cookie.o
ifneq ($(MAKECMDGOALS), tester)
target := dingloader
else
target := tester
obj := $(filter-out main.o,$(obj))
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
