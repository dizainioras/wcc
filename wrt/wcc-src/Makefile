#Req. external curl and openblas lbs.
#All other libraries are included during compile time.
#

USE32 = "-m32"

SRCDIR = .
INCDIR = .

CXXFLAGS2 = -std=c++17 -I$(CURDIR)/libs/mlpack-4.5.1/src  -I $(CURDIR)/libs/ensmallen/include -I $(CURDIR)/libs/cereal-1.3.0/include -I $(CURDIR)/libs/armadillo-14.4.0/include -DARMA_DONT_USE_WRAPPER -lcurl

LDFLAGS2 = -std=c++17 -lopenblas -lcurl -lstdc++ 

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(wildcard $(INCDIR)/*.h)

OUTPUTFILE=wcc

main: $(OBJS)
	$(CXX)  -o $(OUTPUTFILE) $^ $(LDFLAGS2) $(LDFLAGS)

%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(CXXFLAGS2)

clean:
	rm -f $(OUTPUTFILE)
	rm -f $(SRCDIR)/*.o    


debug:
	gdb -ex run ./$(OUTPUTFILE)
