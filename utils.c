/*--------------------------------------------------------------------------------------------------
 *
 * utils.c
 *			Utility functions for tibero_fdw
 *
 * Portions Copyright (c) 2022-2023, Tmax OpenSQL Research & Development Team
 *
 * IDENTIFICATION
 *			contrib/tibero_fdw/utils.c
 *
 *--------------------------------------------------------------------------------------------------
 */
#include <unistd.h>																/* getpid																				*/
#include "postgres.h"
#include "port.h"																	/* pqsigfunc																		*/
#include "libpq/pqsignal.h"												/* pqsignal																			*/

/* {{{ global variables ***************************************************************************/

/* TODO PG_SIGNAL_COUNT */
pqsigfunc prev_sigsegv_handler;
pqsigfunc prev_sigbus_handler;
pqsigfunc prev_sigabrt_handler;
pqsigfunc prev_sigfpe_handler;

bool is_signal_handlers_registered = false;
bool use_sleep_on_sig = false;
/*************************************************************************** global variables }}} */

void register_signal_handlers(void);
static void sleep_on_sig(int signo);
void set_sleep_on_sig_on(void);
void set_sleep_on_sig_off(void);
static void sigsegv_handler(int signo);
static void sigbus_handler(int signo);
static void sigabrt_handler(int signo);
static void sigfpe_handler(int signo);

inline void
set_sleep_on_sig_on(void)
{
	use_sleep_on_sig = true;
}

inline void
set_sleep_on_sig_off(void)
{
	use_sleep_on_sig = false;
}

static void
sigsegv_handler(int signo)
{
	if (use_sleep_on_sig)
		sleep_on_sig(signo);

	pqsignal(signo, prev_sigsegv_handler);
	raise(signo);
}

static void
sigbus_handler(int signo)
{
	if (use_sleep_on_sig)
		sleep_on_sig(signo);

	pqsignal(signo, prev_sigbus_handler);
	raise(signo);
}

static void
sigabrt_handler(int signo)
{
	if (use_sleep_on_sig)
		sleep_on_sig(signo);

	pqsignal(signo, prev_sigabrt_handler);
	raise(signo);
}

static void
sigfpe_handler(int signo)
{
	if (use_sleep_on_sig)
		sleep_on_sig(signo);

	pqsignal(signo, prev_sigfpe_handler);
	raise(signo);
}

void
register_signal_handlers(void)
{
	prev_sigsegv_handler = pqsignal(SIGSEGV, sigsegv_handler);
	prev_sigbus_handler	= pqsignal(SIGBUS, sigbus_handler);
	prev_sigabrt_handler = pqsignal(SIGABRT, sigabrt_handler);
	prev_sigfpe_handler	= pqsignal(SIGFPE, sigfpe_handler);

	is_signal_handlers_registered = true;
}

static void
sleep_on_sig(int signo)
{
	if (!use_sleep_on_sig) return;

	ereport(WARNING, errcode(ERRCODE_WARNING),
					errmsg("*** RECEIVED signal %d (PID=%d)\n\nWAITING SLEEP ON SIGNAL\n", signo, getpid()));

	while (use_sleep_on_sig) {
		pg_usleep(100000);
	}
}
