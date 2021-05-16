CC = g++

TARGET = main
H1	   = HelperFunctions
H2	   = Constants

ROW_ACCESS_DELAY = 10
COLUMN_ACCESS_DELAY = 2
CPU = 5
INPUT = input
SRC_DIR = Input
OUT_DIR = Output

.PHONY: all

all: clean build run

.DEFAULT:
	@echo Use make all to run the project ...

short:
	./$(TARGET) > out

run:
	@echo Running the Executable ...
	mkdir $(OUT_DIR); 
	./$(TARGET) $(CPU) $(ROW_ACCESS_DELAY) $(COLUMN_ACCESS_DELAY) > $(OUT_DIR)/out
	@echo Output Stored in $(OUT_DIR)/out

build:
	@echo Building Project ...
	$(CC) -o $(TARGET) $(TARGET).cpp $(CPPFLAGS)
	@echo Executable file generated successfuly ...
	@echo

clean:
	@echo Removing old files ... 
	$(RM) -r $(OUT_DIR)
	$(RM) -r $(TARGET)
	$(RM) -r $(H1)
	$(RM) -r $(H2)
	@echo 