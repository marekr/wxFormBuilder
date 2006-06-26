CXX = g++

LIBRARY = libmmwx.dll

LDFLAGS = -L. -LC:/wx-2.5.4_compiled_as_dll/lib -mthreads -mwindows
EXTRALIBS =  -mthreads -lrpcrt4 -loleaut32 -lole32 -luuid -lodbc32 -lole32 -loleaut32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lgdi32
# -mthreads -Wl,--subsystem,windows -mwindows -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32 -lodbc32_@@_

DEBUG = -g
CXXFLAGS = -DWX_PRECOMP -O2 -mthreads -Wall -DNO_GCC_PRAGMA -D__WXMSW__ -DWXUSINGDLL -IC:/wx-2.5.4_compiled_as_dll/include $(DEBUG) -I./mmwx
LIBS = -lwxmsw25_core -lwxmsw25_adv -lwxmsw25_media -lwxmsw25_html -lwxmsw25_xrc -lwxbase25 -lwxbase25_net -lwxbase25_xml -lwxtiff -lwxjpeg -lwxpng -lwxzlib  -lwxregex -lwxexpat

all: $(LIBRARY)

OBJS = mmwx.o mmmultibutton.o mmmonthctrl.o mmnavigator.o

$(LIBRARY): $(OBJS)
	$(CXX) -L. -shared -o $(LIBRARY) $(OBJS) -lstdc++ $(LDFLAGS)  $(LIBS)
	strip $(LIBRARY)

mmwx.o: mmwx.cpp
	$(CXX) -c $(CXXFLAGS) -o mmwx.o mmwx.cpp

mmmultibutton.o: mmwx/mmmultibutton.cpp
	$(CXX) -c $(CXXFLAGS) -o mmmultibutton.o mmwx/mmmultibutton.cpp

mmmonthctrl.o: mmwx/mmmonthctrl.cpp
	$(CXX) -c $(CXXFLAGS) -o mmmonthctrl.o mmwx/mmmonthctrl.cpp

mmnavigator.o: mmwx/mmnavigator.cpp
	$(CXX) -c $(CXXFLAGS) -o mmnavigator.o mmwx/mmnavigator.cpp
