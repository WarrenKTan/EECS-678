# Executable name
TARGET = thread-1

# Default rule
all: run

# Rule to build the executable
$(TARGET):
	@echo
	@echo Compiling...
	@gcc -std=c11 $(TARGET).c -o $(TARGET).exe

# Rule to run the program
run: $(TARGET)
	@echo
	@echo Running Program...
	@echo ./$(TARGET).exe
	@./$(TARGET).exe
	@echo

# Clean rule
clean:
	@echo
	@echo Cleaning...
	@rm -f $(TARGET).exe
	@echo
