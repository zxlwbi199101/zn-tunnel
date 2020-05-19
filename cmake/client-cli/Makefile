CXX = g++
CXXFLAGS += \
	-DBOOST_LOG_DYN_LINK \
	-std=c++11 \
	-I../libtun/include

LDFLAGS += \
	-L../libtun/lib \
	-ltun \
	-lboost_log_setup-mt \
	-lboost_log-mt \
	-lboost_filesystem-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_date_time-mt \
	-lboost_regex-mt \
	-lfmt

SOURCES = $(wildcard src/*.cc)
OBJS = $(addsuffix .o, $(basename $(SOURCES)))

compile: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o client.o

run: compile
	sudo ./client.o

clean:
	rm -f $(OBJS) client.o