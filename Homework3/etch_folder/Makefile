TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes

TOOLS_TARGETS	:= my2cset

#
# Programs
#
all:	etch_matrix

etch_matrix: etch_matrix.o gpio-utils.o i2cbusses.o
	$(CC) $(LDFLAGS) -o $@ $^

#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm etch_matrix.o gpio-utils.o i2cbusses.o
