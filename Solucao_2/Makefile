OBJ_DIR=obj
INC_DIR=include
SRC_DIR=src

EXE    =barbershop
OBJ    =$(addprefix $(OBJ_DIR)/, queue.o barber.o customer.o barbershop.o)
INC    =$(wildcard $(INC_DIR)/*)

CC     =gcc
CFLAGS =-g -Iinclude
LDLIBS =-pthread

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -pv $@

.PHONY: clean
clean:
	@rm $(EXE) $(OBJ_DIR) -rfv