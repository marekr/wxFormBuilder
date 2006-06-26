CXX = g++

LIBCOMMON = libcommon.so
LIBLAYOUT = liblayout.so
LIBADDITIONAL = libadditional.so

WXFBDIR = ../../..

CXXFLAGS = `wx-config --cxxflags` -I"." -I$(WXFBDIR)/src -I$(WXFBDIR)/libs/tinyxml -DTIXML_USE_STL -g
CFLAGS = `wx-config --cflags` -I"." -I$(WXFBDIR)/src -I$(WXFBDIR)/libs/tinyxml -DTIXML_USE_STL -g
LDFLAGS = `wx-config --libs` $(WXFBDIR)/libs/tinyxml/tinyxml.a

all: $(LIBCOMMON) $(LIBLAYOUT) $(LIBADDITIONAL)

$(LIBCOMMON): common.o xrcconv.o
	$(CXX) -L. -shared -o $(LIBCOMMON) common.o xrcconv.o $(LDFLAGS)
	strip $(LIBCOMMON)
	mv $(LIBCOMMON) $(WXFBDIR)/bin/xml/

common.o: common.cpp
	$(CXX) -c $(CXXFLAGS) -o common.o common.cpp

$(LIBLAYOUT): layout.o xrcconv.o
	$(CXX) -L. -shared -o $(LIBLAYOUT) layout.o xrcconv.o $(LDFLAGS)
	strip $(LIBLAYOUT)
	mv $(LIBLAYOUT) $(WXFBDIR)/bin/xml/

layout.o: layout.cpp xrcconv.o
	$(CXX) -c $(CXXFLAGS) -o layout.o layout.cpp

$(LIBADDITIONAL): additional.o xrcconv.o
	$(CXX) -L. -shared -o $(LIBADDITIONAL) additional.o xrcconv.o $(LDFLAGS)
	strip $(LIBADDITIONAL)
	mv $(LIBADDITIONAL) $(WXFBDIR)/bin/xml/

additional.o: additional.cpp
	$(CXX) -c $(CXXFLAGS) -o additional.o additional.cpp
	
		
xrcconv.o: $(WXFBDIR)/src/utils/xrcconv.cpp
	$(CXX) -c $(CXXFLAGS) -o xrcconv.o $(WXFBDIR)/src/utils/xrcconv.cpp


clean:
	rm -rf *.o *.so
