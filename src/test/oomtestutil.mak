EXE=testoom
SLIB=oomtestutil.so
DUMMY=oomtestdummy.o
ALL_PRODUCTS=$(EXE) $(SLIB) $(DUMMY)

all: $(ALL_PRODUCTS)

clean:
	rm $(ALL_PRODUCTS)

# The dummy functions must be in a -shared object or they will be statically
# linked to their call sites and the loader will not be able to replace them.
$(DUMMY): oomtestdummy.cpp
	gcc -shared -o $@ $<

# -ldl must be at the end of the options list.
# Utility library must be in a -shared object for LD_PRELOAD to be able to
# load it at all.
# -fpic doesn't seem to be required.
$(SLIB): oomtestutil.cpp
	gcc -shared -o $@ $< -ldl

# If the dummy library is not placed on the usual library path, the executable
# must be executed with LD_LIBRARY_PATH=. as well as LD_PRELOAD=oomtestutil.so
# So the shell command to execute the test executable is:
# LD_PRELOAD=./testoomutil.so LD_LIBRARY_PATH=. ./testoom
$(EXE): testoom.cpp $(DUMMY)
	gcc -o $@ $^
