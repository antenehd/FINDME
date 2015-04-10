LIB=inc/
SRC=src
OBJ_DIR=OBJECTS
TARGET=mserver
CFLAGS:= -pedantic -Wno-variadic-macros -Wall --coverage
LDFLAG:= -L . -lpthread -lrt --coverage

SOURCE=$(shell find $(SRC) -name '*.c')
OBJS:=$(patsubst $(SRC)/%.c,$(OBJ_DIR)/%.o,$(SOURCE))

.PHONY: all clean

all: OBJDIR $(TARGET)

$(TARGET): $(OBJS)
	gcc $(OBJS) $(LDFLAG) -g -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC)/%.c
	gcc -c $(CFLAGS) -g -I $(LIB) $< -o $@

OBJDIR:
	mkdir -p $(OBJ_DIR)/

clean:
	rm -rf $(OBJ_DIR) $(TARGET) 
