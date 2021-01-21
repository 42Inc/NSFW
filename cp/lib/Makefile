CC := gcc
BIN_PATH := ./bin
SRC_PATH := ./src
INC_PATH := ./include
RPC_PATH := /usr/include/tirpc
OBJ_PATH := ./obj
STD := -std=c99
DEBUG := -g3
CFLAGS := ${DEBUG} -pthread -ltirpc

SERVICE := librmpi.a


SOURCES=$(wildcard ${SRC_PATH}/*.c)
OBJECTS=$(patsubst ${SRC_PATH}%, ${OBJ_PATH}%, $(SOURCES:.c=.o))

GIT_BRANCH := "unknown"
GIT_HASH := $(shell git log --pretty=format:%H -n 1)
GIT_HASH_SHORT := $(shell echo "${GIT_HASH}" | cut -c1-7)
GIT_TAG := $(shell git describe --always --tags --abbrev=0 | tail -c+2)
GIT_COMMIT := $(shell git rev-list v${GIT_TAG}..HEAD --count)
GIT_COMMIT_DATE := $(shell git show -s --format=%ci | cut -d\  -f1)

VERSION_RELEASE := ${GIT_TAG}.${GIT_COMMIT}


.PHONY: all
.PHONY: DIRECTORY
all: DIRECTORY VARS ${SERVICE}

VARS:
	sed 's/%%VERSION_RELEASE%%/${VERSION_RELEASE}/g' ${SRC_PATH}/constants.c.tpl > ${SRC_PATH}/constants.c

${SERVICE}: ${OBJECTS}
	${AR} rvs $@ $^

DIRECTORY: ${OBJ_PATH}
${OBJ_PATH}:
	$(if ifeq test -d "${OBJ_PATH}" 0, @mkdir -p ${OBJ_PATH})

${OBJ_PATH}/%.o: ${SRC_PATH}/%.c
	${CC} -c $< -o $@ ${STD} ${CFLAGS} -I ${INC_PATH} -I ${RPC_PATH}

.PHONY: clean
clean:
	rm -rf ${BIN_PATH}/*
	rm -rf ${OBJ_PATH}/*

.PHONY: restruct
restruct: clean all
