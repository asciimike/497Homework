TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes
#
# Programs
#
all:	gpio-int-test togglegpio gpioThru fourThru etch_gpio TMPTest

gpio-int-test:  gpio-int-test.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

togglegpio:	togglegpio.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

gpioThru:	gpioThru.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

fourThru: fourThru.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

etch_gpio: etch_gpio.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

TMPTest: TMPTest.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^


#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm gpio-int-test.o gpio-utils.o gpioThru.o fourThru.o etch_gpio.o TMPTest.o
