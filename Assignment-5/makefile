CC = g++
TARGET = main
ROW_ACCESS_DELAY = 10
COLUMN_ACCESS_DELAY = 2
CPU = 1
INPUT = input
SRC_DIR = ./InputFile
OUT_DIR = ./OutputFile

.PHONY: all

all: build run

.DEFAULT:
	@echo Use make all to run the project ...

short:
	./$(TARGET) > out

run:
	@echo Running the Executable ...
	./$(TARGET) $(CPU) $(ROW_ACCESS_DELAY) $(COLUMN_ACCESS_DELAY) > out

build:
	@echo Building Project ...
	$(CC) -o $(TARGET) $(TARGET).cpp $(CPPFLAGS)
	@echo Executable file generated successfuly ...
	@echo

clean:
	@echo Removing old files ... 
	$(RM) -r out
	$(RM) -r $(TARGET)
	@echo 