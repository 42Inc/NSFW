CC := gcc
BIN_PATH := ./bin
SRC_PATH := ./src
INC_PATH := ./include
OBJ_PATH := ./obj
STD := -std=c99
DEBUG := -g3
CFLAGS := ${DEBUG}

SERVICE := client


SOURCES=$(wildcard ${SRC_PATH}/*.c)
OBJECTS=$(patsubst ${SRC_PATH}%, ${OBJ_PATH}%, $(SOURCES:.c=.o))

.PHONY: all
.PHONY: DIRECTORY
all: DIRECTORY ${SERVICE}

${SERVICE}: ${OBJECTS}
	${CC} $^ -o ${BIN_PATH}/$@ ${STD} ${CFLAGS} -I ${INC_PATH}

DIRECTORY: ${BIN_PATH} ${OBJ_PATH}
${BIN_PATH}:
	$(if ifeq test -d "${BIN_PATH}" 0, @mkdir -p ${BIN_PATH})

${OBJ_PATH}:
	$(if ifeq test -d "${OBJ_PATH}" 0, @mkdir -p ${OBJ_PATH})

${OBJ_PATH}/%.o: ${SRC_PATH}/%.c
	${CC} -c $< -o $@ ${STD} ${CFLAGS} -I ${INC_PATH}

.PHONY: clean
clean:
	rm -rf ${BIN_PATH}/*
	rm -rf ${OBJ_PATH}/*

.PHONY: restruct
restruct: clean all
