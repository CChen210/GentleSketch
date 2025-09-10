CPPFLAGS = -Wall -O3 -std=c++14 -lm -w -mcmodel=medium -g
PROGRAMS = main 

all: $(PROGRAMS)

main:main.cpp \
	BOBHASH32.h BOBHASH64.h BaseSketch.h CMSketch.h WavingSketch.h DASketch.h CuckooCounter.h heavykeeper.h params.h spacesaving.h ssummary.h GentleSketch.h LossyStrategy.h Uss.h BS.h
	g++ -o Sketch main.cpp $(CPPFLAGS)

clean:
	rm -f *.o $(PROGRAMS)
