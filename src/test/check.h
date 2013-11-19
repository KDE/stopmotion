#ifndef CHECK_H_
#define CHECK_H_

// struct of arbitrary types, like:
// HStruct<int, HStruct<std::string, HStruct<bool, HNil> > >
// for a struct of int, string and bool.
// Template V<T> must have a default constructor and an operator*() that
// returns a T or reference to T
class HNil {};
template<template<typename> class V, typename T, typename NEXT> class HStruct {
public:
	V<T> value;
	NEXT next;
};

// get an HStruct of arguments from a function type, each wrapped
// in a V
template<template<typename> class V, typename FN>
class fn_traits {};

template<template<typename> class V, typename R>
class fn_traits<V, R ()> {
public:
	typedef HNil args_t;
	R call(R (*f)(), args_t) { return f(); }
};

template<template<typename> class V, typename R, typename A0>
class fn_traits<V, R (A0)> {
public:
	typedef HStruct<V, A0, HNil> args_t;
	static R call(R (*f)(A0), args_t a) { return f(*a.value); }
};

template<template<typename> class V, typename R, typename A0, typename A1>
class fn_traits<V, R (A0, A1)> {
public:
	typedef HStruct<V, A0, HStruct<V, A1, HNil> > args_t;
	static R call(R (*f)(A0, A1), args_t a) { return
			f(*a.value, *a.next.value); }
};

template<template<typename> class V, typename R, typename A0, typename A1, typename A2>
class fn_traits<V, R (A0, A1, A2)> {
public:
	typedef HStruct<V, A0, HStruct<V, A1, HStruct<V, A2, HNil> > > args_t;
	static R call(R (*f)(A0, A1, A2), args_t a) { return
			f(*a.value, *a.next.value, *a.next.next.value); }
};

template<template<typename> class V, typename R, typename A0, typename A1, typename A2, typename A3>
class fn_traits<V, R (A0, A1, A2, A3)> {
public:
	typedef HStruct<V, A0, HStruct<V, A1, HStruct<V, A2, HStruct<V, A3, HNil> > > > args_t;
	static R call(R (*f)(A0, A1, A2, A3), args_t a) { return
			f(*a.value, *a.next.value, *a.next.next.value, *a.next.next.next.value); }
};

// and so on
//...

#include <stdlib.h>
#include <list>
template<typename T> class Random {};

template<> class Random<int> {
public:
	int operator*() {
		return rand();
	}
};

template<typename T> class Random<std::list<T> > {
public:
	std::list<T> operator*() {
		if (rand() < RAND_MAX/5) {
			return std::list<T>();
		}
		std::list<T> rest = *this;
		Random<T> v;
		rest.push_front(*v);
		return rest;
	}
};

#include <assert.h>
#include <stdio.h>
template<typename FN> void RunTest(int n, FN* fn, const char* message,
		const char* file, int line) {
	typedef fn_traits<Random, FN> FN_traits;
	for (int i = 0; i != n; ++i) {
		typename FN_traits::args_t args;
		if (!FN_traits::call(fn, args)) {
			fprintf(stderr, "test '%s' failed at line %d of %s\n", message,
					line, file);
			assert(false);
		}
	}
}

#define RUNTEST(fn) RunTest(100, fn, #fn, __FILE__, __LINE__)

// used like this:
bool test_add_commutes(int a, int b) { return a + b == b + a; }

void run_tests() {
	RUNTEST(test_add_commutes);
}

#endif /* CHECK_H_ */
