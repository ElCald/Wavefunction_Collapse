#
# MAIN CONFIGURATION
# Author makefile : Cyril Rabat
#

EXEC = wfc_image_seq wfc_image_omp wfc_image_omp_tasks
OBJECTS = wave_utils.o
PROJECT_NAME = WaveCollapse

#
# SUFFIXES
#

.SUFFIXES: .cpp .o

#
# OBJECTS
#

EXEC_O = $(EXEC:=.o)
OBJECTS_O = $(OBJECTS) $(EXEC_O)

#
# ARGUMENTS AND COMPILER
#

CC = g++
CCFLAGS_STD =-Ofast -Wall -march=native -mtune=native
CCFLAGS_DEBUG = -D _DEBUG_ -g -O0
CCFLAGS = $(CCFLAGS_STD)
CCFLAGOMP = -fopenmp
CCLIBS = -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

#
# RULES
#

all: msg $(OBJECTS) $(EXEC_O)
	@echo "Create executables..."
	@for i in $(EXEC); do \
	$(CC) $(CCFLAGOMP) -o $$i.exe $$i.o $(OBJECTS) $(CCLIBS); \
	done
	@echo "Done."

msg:
	@echo "Create objects..."

debug: CCFLAGS = $(CCFLAGS_STD) $(CCFLAGS_DEBUG)
debug: all

#
# DEFAULT RULES
#

%.o : %.cpp
	@cd $(dir $<) && ${CC} ${CCFLAGOMP} ${CCFLAGS} -c $(notdir $<) -o $(notdir $@)

#
# GENERAL RULES
#

clean:
	@echo "Delete objects, temporary files..."
	@rm -f $(OBJECTS) $(EXEC_O)
	@rm -f *~ *#
	@rm -f $(EXEC).exe
	@rm -f dependancies
	@echo "Done."

depend:
	@echo "Create dependancies..."
	@sed -e "/^# DEPENDANCIES/,$$ d" makefile > dependancies
	@echo "# DEPENDANCIES" >> dependancies
	@for i in $(OBJECTS_O); do \
	$(CC) -MM -MT $$i $(CCFLAGOMP) $(CCFLAGS) `echo $$i | sed "s/\(.*\)\\.o$$/\1.cpp/"` >> dependancies; \
	done
	@cat dependancies > makefile
	@rm dependancies
	@echo "Done."

#
# CREATE ARCHIVE
#

ARCHIVE_FILES = *

archive: clean
	@echo "Create archive $(PROJECT_NAME)_$(shell date '+%y%m%d.tar.gz')..."
	@REP=`basename "$$PWD"`; cd .. && tar zcf $(PROJECT_NAME)_$(shell date '+%y%m%d.tar.gz') $(addprefix "$$REP"/,$(ARCHIVE_FILES))
	@echo "Done."

# DEPENDANCIES
wave_utils.o : wave_utils.h wave_utils.cpp
test.o: test.cpp wave_utils.h
