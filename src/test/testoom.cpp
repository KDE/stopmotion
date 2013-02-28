#include <assert.h>
#include <malloc.h>
#include "oomtestutil.h"

int main(int argc, char** argv) {
	assert(LoadOomTestUtil());
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
	printf("Success!\n");
}
