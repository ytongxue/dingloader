CC := gcc
CFLAGS := -g
obj = main.o http.o bitmap.o cookie.o
main : $(obj)
	$(CC) $(CFLAGS) -o $@ $^

$(obj) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

run :
	./main

clean:
	@-rm *.o
	@-rm main
	@echo Done.
