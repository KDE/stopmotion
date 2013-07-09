EXE=testoom
SLIB=oomtestutil.so
STUB=oomteststub.o
MAIN=testoom.o
ALL_PRODUCTS=$(EXE) $(SLIB) $(STUB) $(MAIN)

all: $(ALL_PRODUCTS)

clean:
	rm $(ALL_PRODUCTS)

# -ldl sets the dl library as a dependency of oomtestutil.so. This library
# contains dlsym().
# Utility library must be in a -shared object for LD_PRELOAD to be able to
# load it at all.
# -D_GNU_SOURCE allows us to use the GNU extensions. For some reason
# _GNU_SOURCE is defined automatically for C++ source but not for C.
# -fpic doesn't seem to be required.
$(SLIB): oomtestutil.c
	gcc -D_GNU_SOURCE -g -shared -o $@ $< -ldl

# We shall produce an object file that must be linked into any executable that
# wants to use our OOM testing utility. The executable must also be linked with
# -ldl. Object files do not allow us to specify dependencies, so using -ldl
# here does not help.
$(STUB): oomteststub.c oomtestutil.h
	g++ -D_GNU_SOURCE -g -c -o $@ $<

# The shell command to execute the test executable with our OOM utility and
# its version of malloc is:
# LD_PRELOAD=./oomtestutil.so ./testoom
$(MAIN): testoom.cpp oomtestutil.h check.h
	gcc -c -o $@ $<

# Library switches like -ldl should be at the end of the options list or their
# symbols won't be seen by object files specified after them.
$(EXE): $(MAIN) $(STUB)
	g++ -o $@ $^ -ldl
