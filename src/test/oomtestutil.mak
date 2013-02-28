EXE=testoom
SLIB=oomtestutil.so
ALL_PRODUCTS=$(EXE) $(SLIB)

all: $(ALL_PRODUCTS)

clean:
	rm $(ALL_PRODUCTS)

# The -ldl switch here isn't strictly necessary, as the host executable will
# always have the dl library loaded anyway. Still, dl really is a dependency,
# so we shall add it here. You can do as you please. 
# Utility library must be in a -shared object for LD_PRELOAD to be able to
# load it at all.
# -fpic doesn't seem to be required.
$(SLIB): oomtestutil.cpp
	gcc -shared -o $@ $< -ldl

# -ldl must be at the end of the options list.
# If the dummy library is not placed on the usual library path, the executable
# must be executed with LD_LIBRARY_PATH=. as well as LD_PRELOAD=oomtestutil.so
# So the shell command to execute the test executable is:
# LD_PRELOAD=./testoomutil.so LD_LIBRARY_PATH=. ./testoom
$(EXE): testoom.cpp oomteststub.cpp
	gcc -o $@ $^ -ldl
