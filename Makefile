CFLAGS += -Wall -Wextra -Wsign-conversion -std=c18 -MD

SOURCES = $(wildcard *.c)
OBJECTS = ${SOURCES:.c=.o}
	DEPS = $(OBJECTS:.o=.d)
	HEADERS = $(wildcard *.h)
BINARY = $(shell basename "${PWD}")

.PHONY: all release clean fmt

all: CFLAGS += -DDEBUG -O0 -g
all: ${BINARY}

release: CFLAGS += -O3
release: ${BINARY}
	strip ${BINARY}

${BINARY}: ${OBJECTS}
	${LINK.c} ${OUTPUT_OPTION} $^

clean:
	${RM} ${BINARY} ${OBJECTS} ${DEPS}

fmt: ${SOURCES} ${HEADERS}
	clang-format -i $^

-include ${DEPS}
