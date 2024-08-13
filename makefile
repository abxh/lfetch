EXEC_NAME := lfetch

CFLAGS     += -Wall -Wextra -pedantic
CFLAGS     += -O3 -march=native -DNDEBUG

C_FILES    := main.c
OBJ_FILES  := $(C_FILES:.c=.o)

.PHONY: all clean

all: $(EXEC_NAME)

clean:
	rm -rf $(OBJ_FILES)

$(EXEC_NAME): $(OBJ_FILES)
	$(CC) $(LD_FLAGS) $^ -o $(EXEC_NAME)

$(OBJ_FILES): $(SRC_FILES)

$(SRC_FILES):
	$(CC) -c $(CFLAGS) $@
