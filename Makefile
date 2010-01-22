include Makefile.defs

CXXFLAGS=-O3 -Wall -Ilibfsqueeze \
	-I$(EIGEN2PREFIX)/include/eigen2
CXX=g++

LIBFSQUEEZE_SOURCES=\
	libfsqueeze/src/DataSet/DataSet.cpp \
	libfsqueeze/src/feature_selection/feature_selection.cpp \
	libfsqueeze/src/maxent/maxent.cpp
LIBFSQUEEZE_OBJECTS=$(LIBFSQUEEZE_SOURCES:.cpp=.o)

FSQUEEZE_SOURCES=\
	util/fsqueeze/fsqueeze.cpp \
	util/fsqueeze/ProgramOptions.cpp
FSQUEEZE_OBJECTS=$(FSQUEEZE_SOURCES:.cpp=.o)

all: fsqueeze

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

libfsqueeze.a: $(LIBFSQUEEZE_OBJECTS)
	ar cr $@ $(LIBFSQUEEZE_OBJECTS)

fsqueeze: $(FSQUEEZE_OBJECTS) libfsqueeze.a
	$(CXX) -O3 -o $@ $(FSQUEEZE_OBJECTS) libfsqueeze.a

clean:
	rm -f libfsqueeze.a fsqueeze
	find libfsqueeze -name '*.o' -exec rm -f {} \;
	find util -name '*.o' -exec rm -f {} \;
