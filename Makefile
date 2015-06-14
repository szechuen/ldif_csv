EXENAME = ldif_csv
OBJS = main.o base64.o

CXX = gcc
CXXFLAGS = -ansi -c -O3 -Wall -Wextra -pedantic
LD = gcc
# LDFLAGS = 

all : $(EXENAME)

$(EXENAME) : $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(EXENAME)

main.o : main.c base64.h
	$(CXX) $(CXXFLAGS) main.c

base64.o : base64.c base64.h
	$(CXX) $(CXXFLAGS) base64.c

clean :
	-rm -f *.o $(EXENAME)