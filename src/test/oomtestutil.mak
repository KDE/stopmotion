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
$(SLIB): oomtestutil.cpp
	g++ -D_GNU_SOURCE -g -fPIC -shared -o $@ $< -ldl

# We shall produce an object file that must be linked into any executable that
# wants to use our OOM testing utility. The executable must also be linked with
# -ldl. Object files do not allow us to specify dependencies, so using -ldl
# here does not help.
$(STUB): oomteststub.cpp oomtestutil.h
	g++ -D_GNU_SOURCE -g -c -o $@ $<

# Library switches like -ldl should be at the end of the options list or their
# symbols won't be seen by object files specified after them.
$(EXE): $(MAIN) $(STUB)
	g++ -o $@ $^ -ldl

