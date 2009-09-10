CXX=g++-4.4
CXXFLAGS=-std=c++0x -O2 -Wall -pedantic -Ilibfsqueeze

LIBFSQUEEZE_SOURCES=\
	libfsqueeze/src/DataSet/DataSet.cpp \
	libfsqueeze/src/FeatureSelection/FeatureSelection.cpp
LIBFSQUEEZE_OBJECTS=$(LIBFSQUEEZE_SOURCES:.cpp=.o)

FSQUEEZE_SOURCES=util/fsqueeze/fsqueeze.cpp
FSQUEEZE_OBJECTS=$(FSQUEEZE_SOURCES:.cpp=.o)

all: fsqueeze

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

libfsqueeze.a: $(LIBFSQUEEZE_OBJECTS)
	ar cr $@ $(LIBFSQUEEZE_OBJECTS)

fsqueeze: $(FSQUEEZE_OBJECTS) libfsqueeze.a
	$(CXX) -o $@ $(FSQUEEZE_OBJECTS) libfsqueeze.a

clean:
	rm -f libfsqueeze.a fsqueeze
	find libfsqueeze -name '*.o' -exec rm -f {} \;
	find util -name '*.o' -exec rm -f {} \;
