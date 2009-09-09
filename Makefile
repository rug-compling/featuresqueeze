CXX=g++-4.4
CXXFLAGS=-std=c++0x -O2 -Wall -pedantic -I.

SOURCES=src/DataSet/DataSet.cpp src/FeatureSelection/FeatureSelection.cpp \
	fsqueeze.cpp
OBJECTS=$(SOURCES:.cpp=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

fsqueeze: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

clean:
	rm -f *.o fsqueeze
	find src -name '*.o' -exec rm -f {} \;
