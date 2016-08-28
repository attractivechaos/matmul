CC=			gcc
CFLAGS=		-g -Wall -Wc++-compat -O2
CXXFLAGS=	-g -Wall -O2 -DNDEBUG -DBOOST_UBLAS_NDEBUG
PROG=		matmul
#CBLAS=		$(HOME)/OpenBLAS

ifdef CBLAS
	CPPFLAGS=-DHAVE_CBLAS
	INCLUDES=-I$(CBLAS)/include
	LIBS=-L$(CBLAS)/lib -lopenblas
endif

ifdef BOOST
	INCLUDES+=-I$(BOOST)/include
endif

ifdef EIGEN3
	INCLUDES+=-I$(EIGEN3)
endif

.SUFFIXES:.c .cpp .o
.PHONY:all

.c.o:
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $< -o $@

.cpp.o:
		$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

matmul:matmul.o
		$(CC) $< -o $@ $(LIBS)

matmul-boost:matmul-boost.o
		$(CXX) $< -o $@ $(LIBS)

matmul-eigen:matmul-eigen.o
		$(CXX) $< -o $@ $(LIBS)

clean:
		rm -fr gmon.out *.o a.out $(PROG) *~ *.a *.dSYM session*
