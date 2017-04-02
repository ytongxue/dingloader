CC := gcc
CFLAGS := -g -std=c99
src := http.c bitmap.c cookie.c
src_dir := src/
obj_dir := obj/
ifneq ($(MAKECMDGOALS), tester)
target := dingloader
src += main.c
else
target := tester
src += tester.c
endif

obj := $(addprefix $(obj_dir),$(patsubst %.c, %.o, $(src)))
VPATH := src

$(target) : $(obj)
	$(CC) $(CFLAGS) -o $@ $^

$(obj) : obj/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

run :
	./$(target) "http://hnd-jp-ping.vultr.com/vultr.com.1000MB.bin"

clean:
	@-rm $(obj_dir)/*.o
	@-rm dingloader
	@-rm tester
	@echo Done.

