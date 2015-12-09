CC      = g++
CFLAGS  = -Wall -g -lz -lrt -lmysqlcppconn -std=c++11
INCLUDEFLAGS =
LDFLAGS =
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))
SUB_DIR = db
OBJS += $(patsubst %.cpp,%.o,$(wildcard $(SUB_DIR)/*.cpp))
TARGETS = HttpServer

.PHONY:all
all : $(TARGETS)

HttpServer:HttpServer.o $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f *.d *.d.*

%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)

%.o:%.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)

%.d:%.cpp
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJS:.o=.d)

.PHONY:clean
clean:
	rm -f $(TARGETS) *.o
# all:
# 	g++ -o HttpServer.o -c HttpServer.cpp -Wall -g
# 	g++ -o http_parser.o -c http_parser.c -Wall -g
# 	g++ -o RequestHandler.o -c RequestHandler.cpp -Wall -g
# 	g++ -o HttpServer HttpServer.o http_parser.o RequestHandler.o -Wall

# .PHONY:clean
# clean:
# 	rm -f $(TARGETS) *.o *.d *.d.*
