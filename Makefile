CC := g++
CFLAGS := -g
TARGET := parse
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))

all:$(TARGET)

%.o:%.cpp
	$(CC) $(CFLAGS) -c $<

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lcurl -lmysqlclient -ljsoncpp -llog4cplus -lrt -lpthread -lrt

clean:
	rm -rf $(TARGET) *.o

