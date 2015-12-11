CC      = g++
CFLAGS  = -Wall -g -lz -lrt -lmysqlcppconn -std=c++11
INCLUDEFLAGS =
LDFLAGS =
SRC_DIR = ./src
BIN_DIR = ./bin
SUB_DIRS = db

OBJS := $(patsubst %.c, %.o, $(wildcard ${SRC_DIR}/*.c))
OBJS += $(patsubst %.cpp, %.o, $(wildcard ${SRC_DIR}/*.cpp))
OBJS += $(foreach sub_dir, ${SUB_DIRS}, $(patsubst %.c,%.o,$(wildcard ${SRC_DIR}/${sub_dir}/*.c)))
OBJS += $(foreach sub_dir, ${SUB_DIRS}, $(patsubst %.cpp,%.o,$(wildcard ${SRC_DIR}/${sub_dir}/*.cpp)))

TARGET = HttpServer
BIN_TARGET = ${BIN_DIR}/${TARGET}

.PHONY:all
all : $(BIN_TARGET)

$(BIN_TARGET):$(OBJS)
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
	rm -f $(BIN_TARGET) $(SRC_DIR)/*.o $(SRC_DIR)/${SUB_DIR}/*.o  $(SRC_DIR)/*.d $(SRC_DIR)/${SUB_DIR}/*.d

