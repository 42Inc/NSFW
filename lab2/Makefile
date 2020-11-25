.PHONY: all build 
all: build
build:
	make -C server
	make -C client

.PHONY: clean
clean:
	make -C server clean
	make -C client clean

.PHONY: restruct
restruct: clean all
