NAME = proj2
EXEC = $(NAME)
sources = $(NAME).c barrier.c

CXX = gcc
RM = rm -f

CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS = -lrt -pthread

OBJFILES = $(sources:.c=.o)
DEPS = barrier.h

.PHONY : all

all : $(EXEC)

$(NAME).o : $(sources) $(DEPS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(EXEC) : $(OBJFILES)
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)

clean:
	$(RM) *.o core *.out

cleanall: clean
	$(RM) $(EXEC)
