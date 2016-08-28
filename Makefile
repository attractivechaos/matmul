CC=			gcc
CFLAGS=		-g -Wall -Wc++-compat -O2
PROG=		matmul
#CBLAS=		$(HOME)/OpenBLAS

ifdef CBLAS
	CPPFLAGS=-DHAVE_CBLAS
	INCLUDES=-I$(CBLAS)/include
	LIBS=-L$(CBLAS)/lib -lopenblas
endif

.SUFFIXES:.c .o
.PHONY:all

.c.o:
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

matmul:matmul.o
		$(CC) $< -o $@ $(LIBS)

clean:
		rm -fr gmon.out *.o a.out $(PROG) *~ *.a *.dSYM session*
