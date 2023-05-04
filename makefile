SRC=ngcubesmon_cm.c 
OBJ=$(subst .c,.o,$(SRC))
all: ngcubesmon_cm

%.o: %.c
	gcc -Wall -g -c $< -o $@

ngcubeslib: $(OBJ)
	gcc -Wall -g -o $@ $+

clean:
	-rm *.o ngcubesmon_cm