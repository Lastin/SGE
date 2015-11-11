INCS =
LIBP = 

ARCH_CFLAGS =
WX_LIBS = `wx-config --libs --gl-libs`
WX_FLAGS = `wx-config --cxxflags`

LIBS = $(WX_LIBS)
ARCH_CFLAGS =
EXES = window

CFLAGS	= $(ARCH_CFLAGS) $(WX_FLAGS) -Wall -Wno-unused -Wno-reorder \
	-O3  -fomit-frame-pointer -fforce-addr 


# ------------------

all : clean $(EXES)

clean :
	find -name "*.o" -exec rm {} \;
	rm -f ${EXES} -R

# ------------------

window : window.o
	g++ window.o -o window $(ARCH_CFLAGS) $(LIBS) $(CFLAGS)

window.o : window.cpp window.h
	g++ -c window.cpp $(ARCH_CFLAGS) $(INCS) $(WX_FLAGS)

# ------------------
