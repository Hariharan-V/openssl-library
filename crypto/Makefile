CXX	:= g++ 
DEBUG	:=	-g -std=c++11
OPT	:=	#-O2
CFLAGS	:= $(DEBUG) $(OPT)
LD	:= g++
LDFLAGS	:= $(DEBUG) -L ../../lib/openssl -lcrypto

all: crypto

crypto:	crypto.cpp 
	$(CXX) $(CFLAGS) $< -o crypto $(LDFLAGS)

