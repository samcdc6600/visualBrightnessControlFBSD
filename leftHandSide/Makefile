# Note that there is something a little annying about our targets. Specifically
# the $(BIN_NAME) and debug targets will not update if the other was just called
# as the source files will not have a different modifiction date. We do not know
# enough about make files to resolve this issue. So one may use the -B option
# (aka --always-make) to force the target.

CXX	= clang++
CXXFLAGS	= -std=c++2a -Wall -Wpedantic -Wextra -O2
INCLUDES	= -I/usr/local/include/
LIBS	= -lX11 -L/usr/local/lib/
BIN_NAME	= visualBar
SRC	= ./src
IDIR	= $(SRC)/include
ODIR	= ./obj
DEPS	= $(wildcard $(IDIR)/*.hpp)
CPP_DIRS	=  $(wildcard $(SRC)/*.cpp) # Get list of .cpp file directories.
CPP_FILES	= $(subst $(SRC)/,,$(CPP_DIRS)) # Strip srings of paths.
OBJ_FILES	= $(patsubst %.cpp,%.o,$(CPP_FILES)) # Replace .cpp with .o.
# Prepend object directory to .o file names.
OBJ	= $(patsubst %,$(ODIR)/%,$(OBJ_FILES))


# Main make file pattern...
# Target: Prerequisite
#	Rule
# 
# $(BIN_NAME) depends on $(OBJ).
$(BIN_NAME): $(OBJ)
	$(CXX) -o $@ $^ $(LIBS) -DNDEBUG


# $(ODIR)/%.o depends on $(SRC)/%.cpp $(DEPS).
$(ODIR)/%.o: $(SRC)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(DEBUG) $(INCLUDES)


# Clean and debug targets won't update on files.
.PHONY: clean debug


# Call make recursively on $(BIN_NAME) target (with DEBUG var set to -DDEBUG.)
debug:
	$(MAKE) $(BIN_NAME) DEBUG=-DDEBUG


# Files with '#' and '~' seem to be left around be emacs >:'^(
clean:
	$(RM) $(BIN_NAME) $(ODIR)/*.o *.core *~ $(SRC)/*~ $(IDIR)/*~ \#*\# $(SRC)/\#*\# $(IDIR)/\#*\#
