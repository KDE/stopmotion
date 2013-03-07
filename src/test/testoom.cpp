#include <assert.h>
#include <malloc.h>
#include "oomtestutil.h"

#include <list>

// This is the function we are going to OOM-test.
// It is supposed to remove the front item of 'from' and put
// it onto the front of 'to'.
template <typename T>
void MoveBetweenLists(std::list<T>& to, std::list<T>& from) {
	if (!from.empty()) {
		// Is there a problem here?
		T temp = from.front();
		from.pop_front();
		to.push_front(temp);
	}
}

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
	printf("success!\n");

	// Now here's how you might use it
	int successfulMallocs = 0;
	bool threw;
	do {
		std::list<int> a1;
		a1.push_front(1);
		a1.push_front(2);
		a1.push_front(3);
		std::list<int> a2;
		a2.push_front(4);
		a2.push_front(5);
		a2.push_front(6);
		threw = false;
		try {
			printf("Trying %d mallocs until failure\n", successfulMallocs);
			SetMallocsUntilFailure(successfulMallocs);
			++successfulMallocs;
			MoveBetweenLists(a1, a2);
		} catch (...) {
			threw = true;
			// make sure the failing function didn't do anything
			// to our lists
			std::list<int>::iterator i1 = a1.begin();
			assert(*i1++ == 3);
			assert(*i1++ == 2);
			assert(*i1++ == 1);
			assert(i1 == a1.end());
			std::list<int>::iterator i2 = a2.begin();
			assert(*i2++ == 6);
			assert(*i2++ == 5);
			assert(*i2++ == 4);
			assert(i2 == a2.end());
		}
		if (!threw) {
			// check we got the right result here
			std::list<int>::iterator i1 = a1.begin();
			assert(*i1++ == 6);
			assert(*i1++ == 3);
			assert(*i1++ == 2);
			assert(*i1++ == 1);
			assert(i1 == a1.end());
			std::list<int>::iterator i2 = a2.begin();
			assert(*i2++ == 5);
			assert(*i2++ == 4);
			assert(i2 == a2.end());
		}
	} while (threw);
}
