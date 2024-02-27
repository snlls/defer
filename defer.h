#pragma once

#include <stdio.h>
#include <threads.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>

/**
 * Problem with defer is return statement.
 * To make sure defer is run with return, the return is overriden by macro.
 * THERE NEEDS TO BE RETURN IF YOU USE defer
 * 
 */

#ifndef DEFER_MAX
#define DEFER_MAX 8
#endif
#ifndef DEFER_NEST_MAX
#define DEFER_NEST_MAX 8
#endif

#if defined(__STDC_NO_THREADS__) || defined(__TINYC__)
#warning("Defer will break if you use it on more than one thread")
#else
#define _defer_local thread_local
#endif

#if defined(__TINYC__)
#define DEFER_IMPL_JMP
typedef struct {
	jmp_buf return_loc;
	jmp_buf deferrals[DEFER_MAX];
	int count;
} defer_t;

#else
#define DEFER_IMPL_GOTO
typedef struct {
	void *return_loc;
	void *deferrals[DEFER_MAX];
	int count;
} defer_t;
#endif

static _defer_local defer_t _defer_state[DEFER_NEST_MAX] = {0};
static _defer_local int _defer_nest = 0;
static _defer_local char *_defer_func[DEFER_NEST_MAX] = {0};


// variables to manage defer state in nested function calls

#define _defer_symb_comb(a, b) a ## b
#define _defer_symb(a, b) _defer_symb_comb(a, b)

/** @brief clenaup current nesting level, called before return */
static int _defer_clean()
{
	memset((void*)(_defer_state + _defer_nest), 0, sizeof(defer_t));
	_defer_func[_defer_nest] = 0;
	if(_defer_nest) {
		_defer_nest--;
	}
	return 0;

}

/** @brief check current nesting level against current function name and increment if needed */
static void _defer_handle_nest(char *func)
{
	if(!_defer_func[_defer_nest]) {
		_defer_func[_defer_nest] = func;
	}
	else if(!strcmp((char*)_defer_func[_defer_nest], func)) {
	}
	else {
		_defer_nest++;
		if(_defer_nest == DEFER_NEST_MAX)
			assert(0);
		_defer_func[_defer_nest] = func;
	}
}

#ifdef DEFER_IMPL_JMP
/** @brief create jump point and handle jump point once the user code is called */
#define defer								\
	for(int _defer_symb(_defer_n_, __LINE__) = 0;			\
	    _defer_symb(_defer_n_, __LINE__) < 3;			\
	    _defer_symb(_defer_n_, __LINE__)++)				\
		if(!_defer_symb(_defer_n_, __LINE__)) {			\
			_defer_handle_nest((char*)__func__);		\
			if(!setjmp((void*)_defer_state[_defer_nest].deferrals[_defer_state[_defer_nest].count++])) { \
				break;					\
			}						\
		}							\
		else if(_defer_symb(_defer_n_, __LINE__) == 2) {	\
			_defer_symb(_defer_n_, __LINE__) = 3;		\
			if(_defer_state[_defer_nest].count) {		\
				longjmp((void*)_defer_state[_defer_nest].deferrals[--_defer_state[_defer_nest].count], 1); \
			} else {					\
				longjmp((void*)_defer_state[_defer_nest].return_loc, 1); \
			}						\
		}							\
		else


/** @brief create jump point and handle the return point once all user defers are handled */
#define return								\
	if(!setjmp((void*)_defer_state[_defer_nest].return_loc)) {	\
	if(_defer_state[_defer_nest].count) {				\
		longjmp((void*)_defer_state[_defer_nest]		\
			.deferrals[--_defer_state[_defer_nest].count],	\
			1);						\
	}								\
	goto _defer_symb(_return_goto, __LINE__);			\
	}								\
	else								\
	_defer_symb(_return_goto, __LINE__):				\
		for(_defer_clean(); 1;)					\
			return
		
#else
#define defer								\
	for(int _defer_symb(_defer_n_, __LINE__) = 0;			\
	    _defer_symb(_defer_n_, __LINE__) < 3;			\
	    _defer_symb(_defer_n_, __LINE__)++)				\
		if(_defer_symb(_defer_n_, __LINE__) == 0) {		\
			_defer_handle_nest((char*)__func__);		\
			_defer_state[_defer_nest].deferrals[_defer_state[_defer_nest].count++] = && _defer_symb(_defer_ini, __LINE__); \
			break;						\
		}							\
		else if(0) {						\
		_defer_symb(_defer_ini, __LINE__):			\
			_defer_symb(_defer_n_, __LINE__) = 0;		\
		}							\
		else if(_defer_symb(_defer_n_, __LINE__) == 2) {	\
			if(_defer_state[_defer_nest].count)		\
				goto *_defer_state[_defer_nest].deferrals[--_defer_state[_defer_nest].count]; \
			else						\
				goto *_defer_state[_defer_nest].return_loc; \
			break;						\
		}							\
		else

#define return								\
	if(_defer_state[_defer_nest].count) {				\
		_defer_state[_defer_nest].return_loc = && _defer_symb(_defer_fini_, __LINE__); \
		goto *_defer_state[_defer_nest].deferrals[--_defer_state[_defer_nest].count]; \
	}								\
	else								\
	_defer_symb(_defer_fini_, __LINE__):				\
		for(_defer_clean(); 1;)					\
			return
#endif
