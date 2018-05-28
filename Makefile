PROJECT =	guff
OPTIMIZE =	-O3
WARN =		-Wall -pedantic
CSTD +=		-std=c99
LDFLAGS +=	-lm
#CDEFS=		-DDEBUG=0
CFLAGS +=	${CSTD} -g ${WARN} ${CDEFS} ${CINCS} ${OPTIMIZE}

TEST_CFLAGS = 	${CFLAGS}
TEST_LDFLAGS = 	${LDFLAGS}

all: test_${PROJECT}
all: ${PROJECT}

OBJS=	args.o \
	ascii.o \
	braille.o \
	counter.o \
	draw.o \
	fnv.o \
	input.o \
	regression.o \
	scale.o \
	svg.o \

TEST_OBJS=	${OBJS} \
	test_draw.o \
	test_input.o \
	test_regression.o \
	test_scale.o \
	test_types.o \

# Basic targets

${PROJECT}: main.o ${OBJS}
	${CC} -o $@ main.o ${OBJS} ${LDFLAGS}

test_${PROJECT}: test_${PROJECT}.o ${TEST_OBJS}
	${CC} -o $@ test_${PROJECT}.o ${TEST_LDFLAGS} \
		${TEST_OBJS} ${TEST_CFLAGS}

test: ./test_${PROJECT}
	./test_${PROJECT}

clean:
	rm -f ${PROJECT} test_${PROJECT} *.o *.a *.core

tags: TAGS
TAGS:
	etags *.[ch]

docs: man/${PROJECT}.1 man/${PROJECT}.1.html

man/${PROJECT}.1: man/${PROJECT}.1.ronn
	ronn --roff $<

man/${PROJECT}.1.html: man/${PROJECT}.1.ronn
	ronn --html $<


# Dependencies
*.o: *.h Makefile

# Installation
PREFIX ?=	/usr/local
INSTALL ?=	install
RM ?=		rm
MAN_DEST ?=	${PREFIX}/share/man

install:
	${INSTALL} -d ${PREFIX}/bin ${MAN_DEST}/man1/
	${INSTALL} -c ${PROJECT} ${PREFIX}/bin
	${INSTALL} -c man/${PROJECT}.1 ${MAN_DEST}/man1/

uninstall:
	${RM} -f ${PREFIX}/bin/${PROJECT}
	${RM} -f ${MAN_DEST}/man1/${PROJECT}.1
