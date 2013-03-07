#include <assert.h>
#include <malloc.h>
#include "oomtestutil.h"

int main(int argc, char** argv) {
	// Test that the user remembered LD_PRELOAD=./oomtestutil.so
	assert(LoadOomTestUtil());

	// Test that oomtestutil itself works
	SetMallocsUntilFailure(0);
	assert(malloc(1) == 0);
	void* shouldBeAllocated1 = malloc(1);
	assert(shouldBeAllocated1);
	free(shouldBeAllocated1);
	SetMallocsUntilFailure(1);
	void* shouldBeAllocated2 = malloc(1);
	assert(shouldBeAllocated2);
	free(shouldBeAllocated2);
	assert(malloc(2) == 0);

	// your tests here
	//...

	printf("success!\n");
}
