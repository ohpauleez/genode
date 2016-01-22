/*
 * \brief   Startup code
 * \author  Christian Helmuth
 * \author  Christian Prochaska
 * \author  Norman Feske
 * \date    2006-04-12
 *
 * The startup code calls constructors for static objects before calling
 * main(). Furthermore, this file contains the support of exit handlers
 * and destructors.
 *
 * Some code within this file is based on 'atexit.c' of FreeBSD's libc.
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/env.h>
#include <base/sleep.h>
#include <base/printf.h>
#include <base/component.h>
#include <cap_session/connection.h>

/* platform-specific local helper functions */
#include <startup/internal/_main_parent_cap.h>
#include <base/internal/crt0.h>


enum { ATEXIT_SIZE = 256 };


/***************
 ** C++ stuff **
 ***************/

void * __dso_handle = 0;

enum Atexit_fn_type { ATEXIT_FN_EMPTY, ATEXIT_FN_STD, ATEXIT_FN_CXA };

struct atexit_fn
{
	Atexit_fn_type fn_type;
	union
	{
		void (*std_func)(void);
		void (*cxa_func)(void *);
	} fn_ptr;		/* function pointer */
	void *fn_arg;	/* argument for CXA callback */
	void *fn_dso;	/* shared module handle */
};

/* all members are initialized with 0 */
static struct atexit
{
	bool enabled;
	int index;
	struct atexit_fn fns[ATEXIT_SIZE];
} _atexit;


static Genode::Lock &atexit_lock()
{
	static Genode::Lock _atexit_lock;
	return _atexit_lock;
}


static void atexit_enable()
{
	_atexit.enabled = true;
}


static int atexit_register(struct atexit_fn *fn)
{
	Genode::Lock::Guard atexit_lock_guard(atexit_lock());

	if (!_atexit.enabled)
		return 0;

	if (_atexit.index >= ATEXIT_SIZE) {
		PERR("Cannot register exit handler - ATEXIT_SIZE reached");
		return -1;
	}

	_atexit.fns[_atexit.index++] = *fn;

	return 0;
}


/**
 * Register a function to be performed at exit
 */
int genode_atexit(void (*func)(void))
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_STD;
	fn.fn_ptr.std_func = func;
	fn.fn_arg = 0;
	fn.fn_dso = 0;

	error = atexit_register(&fn);
	return (error);
}


/**
 * Register a function to be performed at exit or when an shared object
 * with given dso handle is unloaded dynamically.
 *
 * This function is called directly by compiler generated code, so
 * it needs to be declared as extern "C" and cannot be local to
 * the cxx lib.
 */
int genode___cxa_atexit(void (*func)(void*), void *arg, void *dso)
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_CXA;
	fn.fn_ptr.cxa_func = func;
	fn.fn_arg = arg;
	fn.fn_dso = dso;

 	error = atexit_register(&fn);
	return (error);
}


/*
 * Call all handlers registered with __cxa_atexit for the shared
 * object owning 'dso'.  Note: if 'dso' is NULL, then all remaining
 * handlers are called.
 */
void genode___cxa_finalize(void *dso)
{
	struct atexit_fn fn;
	int n = 0;

	atexit_lock().lock();
	for (n = _atexit.index; --n >= 0;) {
		if (_atexit.fns[n].fn_type == ATEXIT_FN_EMPTY)
			continue; /* already been called */
		if (dso != 0 && dso != _atexit.fns[n].fn_dso)
			continue; /* wrong DSO */
		fn = _atexit.fns[n];

		/*
		 * Mark entry to indicate that this particular handler
		 * has already been called.
		 */
		_atexit.fns[n].fn_type = ATEXIT_FN_EMPTY;
		atexit_lock().unlock();

		/* call the function of correct type */
		if (fn.fn_type == ATEXIT_FN_CXA)
			fn.fn_ptr.cxa_func(fn.fn_arg);
		else if (fn.fn_type == ATEXIT_FN_STD)
			fn.fn_ptr.std_func();

		atexit_lock().lock();
	}
	atexit_lock().unlock();
}


extern "C" void __cxa_finalize(void *dso);

/**
 * Terminate the process.
 */
void genode_exit(int status)
{
	/* call handlers registered with 'atexit()' or '__cxa_atexit()' */
	__cxa_finalize(0);

	/* call destructors for global static objects. */
	void (**func)();
	for (func = &_dtors_start; func != &_dtors_end; (*func++)());

	/* inform parent about the exit status */
	Genode::env()->parent()->exit(status);

	/* wait for destruction by the parent */
	Genode::sleep_forever();
}


/**
 * Dummy default arguments for main function
 */
static char  argv0[] = { '_', 'm', 'a', 'i', 'n', 0};
static char *argv[1] = { argv0 };


/**
 * Arguments for main function
 *
 * These global variables may be initialized by a constructor provided by an
 * external library.
 */
char **genode_argv = argv;
int    genode_argc = 1;
char **genode_envp = 0;


namespace {

	struct Environment : Genode::Environment
	{
		Genode::Entrypoint &_ep;

		Environment(Genode::Entrypoint &ep) : _ep(ep) { }

		Genode::Parent      &parent() override { return *Genode::env()->parent(); }
		Genode::Ram_session &ram()    override { return *Genode::env()->ram_session(); }
		Genode::Cpu_session &cpu()    override { return *Genode::env()->cpu_session(); }
		Genode::Rm_session  &rm()     override { return *Genode::env()->rm_session(); }
		Genode::Pd_session  &pd()     override { return *Genode::env()->pd_session(); }
		Genode::Entrypoint  &ep()     override { return _ep; }

		Genode::Ram_session_capability ram_session_cap() override
		{
			return Genode::env()->ram_session_cap();
		}

		Genode::Cpu_session_capability cpu_session_cap() override
		{
			return Genode::env()->cpu_session_cap();
		}
	};
}


namespace Genode { struct Startup; }


/*
 * We need to execute the constructor of the main entrypoint from a
 * class called 'Startup' as 'Startup' is a friend of 'Entrypoint'.
 */
struct Genode::Startup
{
	::Environment env { ep };

	/*
	 * The construction of the main entrypoint does never return.
	 */
	Entrypoint ep { *Genode::env()->pd_session(), env };
};


/****************************************************
 ** Fallback implementation of Component interface **
 ****************************************************/

/*
 * Regular components provide the 'Component' interface as defined in
 * base/component.h. This fallback accommodates legacy components that lack the
 * implementation of this interface but come with a main function.
 */

extern int main(int argc, char **argv, char **envp);

void Component::construct(Genode::Environment &env) __attribute__((weak));
void Component::construct(Genode::Environment &env)
{
	/* call real main function */
	int ret = main(genode_argc, genode_argv, genode_envp);

	genode_exit(ret);
}


Genode::size_t Component::stack_size() __attribute__((weak));
Genode::size_t Component::stack_size()
{
	return 16UL * 1024 * sizeof(Genode::addr_t);
}


char const *Component::name() __attribute__((weak));
char const *Component::name()
{
	return "ep";
}


/******************************************************
 ** C entry function called by the crt0 startup code **
 ******************************************************/


namespace Genode {

	/*
	 * To be called from the context of the initial entrypoiny before
	 * passing control to the 'Component::construct' function.
	 */
	void call_global_static_constructors()
	{
		void (**func)();
		for (func = &_ctors_end; func != &_ctors_start; (*--func)());
	}
}

 
extern "C" int _main()
{
	/*
	 * Allow exit handlers to be registered.
	 *
	 * This is done after the creation of the environment to prevent its
	 * destruction. The environment is still needed to notify the parent
	 * after all exit handlers (including static object destructors) have
	 * been called.
	 */
	atexit_enable();

	static Genode::Startup startup;

	/* never reached */
	return 0;
}
