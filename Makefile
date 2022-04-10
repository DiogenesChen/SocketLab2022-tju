SRC_DIR := src
OBJ_DIR := obj
LOG_DIR := log
# all src files
SRC := $(wildcard $(SRC_DIR)/*.c)
# all objects
OBJ := $(OBJ_DIR)/y.tab.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/example.o
# all binaries
ALLOBJ := $(OBJ_DIR)/y.tab.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/log.o $(OBJ_DIR)/login.o
# all lex, yacc and parse, log objects
LOGTXT :=$(LOG_DIR)/log.txt $(LOG_DIR_DIR)/error.txt
# all log files
BIN := example liso_server liso_client
# C compiler
CC  := gcc
# C PreProcessor Flag
CPPFLAGS := -Iinclude
# compiler flags
CFLAGS   := -g -Wall
# DEPS = parse.h y.tab.h

default: all
all : example liso_server liso_client

$(shell touch $(LOG_DIR)/log.txt)
$(shell touch $(LOG_DIR)/error.txt)

example: $(OBJ)
	$(CC) $^ -o $@

log: $(OBJ)
	$(CC) $^ -o $@

$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l
	flex -o $@ $^

$(SRC_DIR)/y.tab.c: $(SRC_DIR)/parser.y
	yacc -d $^
	mv y.tab.c $@
	mv y.tab.h $(SRC_DIR)/y.tab.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

liso_server: $(ALLOBJ) $(OBJ_DIR)/liso_server.o
	$(CC) -Werror $^ -o $@

liso_client: $(ALLOBJ) $(OBJ_DIR)/liso_client.o
	$(CC) -Werror $^ -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ) $(BIN) $(SRC_DIR)/lex.yy.c $(SRC_DIR)/y.tab.* $(LOG_DIR)/*.txt
	$(RM) -r $(OBJ_DIR)
