/*
 * debug - debug, warning and error reporting facility
 *
 * Copyright (c) 2019-2020 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * Share and enjoy! :-)
 */

/* NUMERIC EXIT CODES: 250-254	debug.c - reserved for internal errors */
/* NUMERIC EXIT CODES: 255	debug.c - FORCED_EXIT */
// NOTE: Other code calls err() and errp() with various exit codes that may result in zero or non-zero exits

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "debug.h"		// debug, warning, error and usag macros


#ifndef DEBUG_LINT

/*
 * msg - print a generic message
 *
 * given:
 *      fmt     printf format
 *      ...
 *
 * Example:
 *
 *      msg("foobar information");
 *      msg("foo = %d\n", foo);
 */
void
msg(const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (fmt == NULL) {
	warn(__func__, "NULL fmt given to debug");
	fmt = "((NULL fmt))";
    }

    /*
     * Print the message
     */
    ret = vfprintf(stderr, fmt, ap);
    if (ret <= 0) {
	fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
    }
    fputc('\n', stderr);

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);
    return;
}


/*
 * dbg - print debug message if we are verbose enough
 *
 * given:
 *      level   print message if >= verbosity level
 *      fmt     printf format
 *      ...
 *
 * Example:
 *
 *      dbg(DBG_MED, "foobar information");
 *      dbg(DBG_LOW, "curds: %s != whey: %d", curds, whey);
 */
void
dbg(int level, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (fmt == NULL) {
	warn(__func__, "NULL fmt given to debug");
	fmt = "((NULL fmt))";
    }

    /*
     * Print the debug message (if verbosity level is enough)
     */
    if (level <= debuglevel) {
	ret = vfprintf(stderr, fmt, ap);
	if (ret <= 0) {
	    fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
	}
	fputc('\n', stderr);
    }

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);
    return;
}


/*
 * warn - issue a warning message
 *
 * given:
 *      name    name of function issuing the warning
 *      fmt     format of the warning
 *      ...     optional format args
 *
 * Example:
 *
 *      warn(__func__, "unexpected foobar: %d", value);
 */
void
warn(const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (name == NULL) {
	fprintf(stderr, "Warning: %s called with NULL name\n", __func__);
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	fprintf(stderr, "Warning: %s called with NULL fmt\n", __func__);
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the warning
     */
    fprintf(stderr, "Warning: %s: ", name);
    ret = vfprintf(stderr, fmt, ap);
    if (ret <= 0) {
	fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
    }
    fputc('\n', stderr);

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);
    return;
}


/*
 * warn - issue a warning message
 *
 * given:
 *      name    name of function issuing the warning
 *      fmt     format of the warning
 *      ...     optional format args
 *
 * Unlike warn() this function also prints an errno message.
 *
 * Example:
 *
 *      warnp(__func__, "unexpected foobar: %d", value);
 */
void
warnp(const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */
    int saved_errno;		/* errno at function start */

    /*
     * Start the var arg setup and fetch our first arg
     */
    saved_errno = errno;
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (name == NULL) {
	fprintf(stderr, "Warning: %s called with NULL name\n", __func__);
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	fprintf(stderr, "Warning: %s called with NULL fmt\n", __func__);
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the warning
     */
    fprintf(stderr, "Warning: %s: ", name);
    ret = vfprintf(stderr, fmt, ap);
    if (ret <= 0) {
	fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
    }
    fputc('\n', stderr);
    fprintf(stderr, "errno[%d]: %s\n", saved_errno, strerror(saved_errno));

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);
    return;
}


/*
 * err - issue a fatal error message and exit
 *
 * given:
 *      exitcode        value to exit with
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.
 *
 * Example:
 *
 *      err(99, __func__, "bad foobar: %s", message);
 */
void
err(int exitcode, const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (exitcode >= 256) {
	warn(__func__, "called with exitcode >= 256: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (exitcode < 0) {
	warn(__func__, "called with exitcode < 0: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (name == NULL) {
	warn(__func__, "called with NULL name");
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	warn(__func__, "called with NULL fmt");
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the fatal error
     */
    fprintf(stderr, "FATAL: %s: ", name);
    ret = vfprintf(stderr, fmt, ap);
    if (ret <= 0) {
	fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
    }
    fputc('\n', stderr);

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);

    /*
     * Terminate
     */
    exit(exitcode);
}


/*
 * errp - issue a fatal error message, errno string and exit
 *
 * given:
 *      exitcode        value to exit with
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.  Unlike err() this function
 * also prints an errno message.
 *
 * Example:
 *
 *      errp(99, __func__, "I/O failure: %s", message);
 */
void
errp(int exitcode, const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */
    int saved_errno;		/* errno at function start */

    /*
     * Start the var arg setup and fetch our first arg
     */
    saved_errno = errno;
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (exitcode >= 256) {
	warn(__func__, "called with exitcode >= 256: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (exitcode < 0) {
	warn(__func__, "called with exitcode < 0: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (name == NULL) {
	warn(__func__, "called with NULL name");
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	warn(__func__, "called with NULL fmt");
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the fatal error
     */
    fprintf(stderr, "FATAL: %s: ", name);
    ret = vfprintf(stderr, fmt, ap);
    if (ret <= 0) {
	fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
    }
    fputc('\n', stderr);
    if (errno != 0) {
	fprintf(stderr, "errno[%d]: %s\n", saved_errno, strerror(saved_errno));
    }

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);

    /*
     * Terminate
     */
    exit(exitcode);
}


/*
 * usage_err - issue a fatal error message and exit
 *
 * given:
 *      exitcode        value to exit with (must be 0 <= exitcode < 256)
 *                      exitcode == 0 ==> just how to use -h for usage help and exit(0)
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.
 *
 * Example:
 *
 *      usage_err(99, __func__, "bad foobar: %s", message);
 */
void
usage_err(int exitcode, const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (exitcode < 0 || exitcode >= 256) {
	warn(__func__, "exitcode must be >= 0 && < 256: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (name == NULL) {
	warn(__func__, "called with NULL name");
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	warn(__func__, "called with NULL fmt");
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the fatal error
     */
    if (exitcode > 0) {
	fprintf(stderr, "FATAL: %s: ", name);
	ret = vfprintf(stderr, fmt, ap);
	if (ret <= 0) {
	    fprintf(stderr, "%s: vfprintf returned error: %d", __func__, ret);
	}
	fputc('\n', stderr);
    }

    /*
     * Issue the usage message help
     */
    if (program == NULL) {
	fprintf(stderr, "For command line usage help, try using -h\n");
    } else {
	fprintf(stderr, "For command line usage help, try: %s -h\n", program);
    }
    fprintf(stderr, "version: %s\n", version_string);

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);

    /*
     * Terminate
     */
    exit(exitcode);
}


/*
 * usage_errp - issue a fatal error message, errno string and exit
 *
 * given:
 *      exitcode        value to exit with (must be 0 <= exitcode < 256)
 *                      exitcode == 0 ==> just how to use -h for usage help and exit(0)
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.  Unlike err() this function
 * also prints an errno message.
 *
 * Example:
 *
 *      usage_errp(99, __func__, "bad foobar: %s", message);
 */
void
usage_errp(int exitcode, const char *name, const char *fmt, ...)
{
    va_list ap;			/* argument pointer */
    int saved_errno;		/* errno at function start */
    int ret;			/* return code holder */

    /*
     * Start the var arg setup and fetch our first arg
     */
    saved_errno = errno;
    va_start(ap, fmt);

    /*
     * Check preconditions (firewall)
     */
    if (exitcode < 0 || exitcode >= 256) {
	warn(__func__, "exitcode must be >= 0 && < 256: %d", exitcode);
	exitcode = FORCED_EXIT;	// exit(255);
	warn(__func__, "forcing exit code: %d", exitcode);
    }
    if (name == NULL) {
	warn(__func__, "called with NULL name");
	name = "((NULL name))";
    }
    if (fmt == NULL) {
	warn(__func__, "called with NULL fmt");
	fmt = "((NULL fmt))";
    }

    /*
     * Issue the fatal error
     */
    if (exitcode > 0) {
	fprintf(stderr, "FATAL: %s: ", name);
	ret = vfprintf(stderr, fmt, ap);
	if (ret <= 0) {
	    fprintf(stderr, "%s: vfprintf returned error: %d", __func__, ret);
	}
	fputc('\n', stderr);
	fprintf(stderr, "errno[%d]: %s\n", saved_errno, strerror(saved_errno));
    }

    /*
     * Issue the usage message
     */
    if (program == NULL) {
	fprintf(stderr, "For command line usage help, try using -h\n");
    } else {
	fprintf(stderr, "For command line usage help, try: %s -h\n", program);
    }
    fprintf(stderr, "%s", version_string);

    /*
     * Clean up stdarg stuff
     */
    va_end(ap);

    /*
     * Terminate
     */
    exit(exitcode);
}

#endif				// DEBUG_LINT
