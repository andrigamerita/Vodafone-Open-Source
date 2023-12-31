/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 * \brief Time-related functions and macros
 */

#ifndef _ASTERISK_TIME_H
#define _ASTERISK_TIME_H

#include <sys/time.h>
#include <sys/times.h>

#include "asterisk/inline_api.h"

/* We have to let the compiler learn what types to use for the elements of a
   struct timeval since on linux, it's time_t and suseconds_t, but on *BSD,
   they are just a long. */
extern struct timeval tv;
typedef typeof(tv.tv_sec) ast_time_t;
typedef typeof(tv.tv_usec) ast_suseconds_t;
extern long ticks_per_sec;

AST_INLINE_API(
void ast_tv_init (void),
{
    ticks_per_sec = sysconf(_SC_CLK_TCK);
}
)

/*!
 * \brief Computes the difference (in milliseconds) between two \c struct \c timeval instances.
 * \param end the beginning of the time period
 * \param start the end of the time period
 * \return the difference in milliseconds
 */
AST_INLINE_API(
int ast_tvdiff_ms(struct timeval end, struct timeval start),
{
	/* Prevent overflow */
	if ((end.tv_sec - start.tv_sec) > (INT_MAX / 1000))
		return INT_MAX;
		
	/* the offset by 1,000,000 below is intentional...
	   it avoids differences in the way that division
	   is handled for positive and negative numbers, by ensuring
	   that the divisor is always positive
	*/
	return  ((end.tv_sec - start.tv_sec) * 1000) +
		(((1000000 + end.tv_usec - start.tv_usec) / 1000) - 1000);
}
)

/*!
 * \brief Returns true if the argument is 0,0
 */
AST_INLINE_API(
int ast_tvzero(const struct timeval t),
{
	return (t.tv_sec == 0 && t.tv_usec == 0);
}
)

/*!
 * \brief Compres two \c struct \c timeval instances returning
 * -1, 0, 1 if the first arg is smaller, equal or greater to the second.
 */
AST_INLINE_API(
int ast_tvcmp(struct timeval _a, struct timeval _b),
{
	if (_a.tv_sec < _b.tv_sec)
		return -1;
	if (_a.tv_sec > _b.tv_sec)
		return 1;
	/* now seconds are equal */
	if (_a.tv_usec < _b.tv_usec)
		return -1;
	if (_a.tv_usec > _b.tv_usec)
		return 1;
	return 0;
}
)

/*!
 * \brief Returns true if the two \c struct \c timeval arguments are equal.
 */
AST_INLINE_API(
int ast_tveq(struct timeval _a, struct timeval _b),
{
	return (_a.tv_sec == _b.tv_sec && _a.tv_usec == _b.tv_usec);
}
)

/*!
 * \brief Returns current timeval. Meant to replace calls to gettimeofday().
 */
AST_INLINE_API(
struct timeval ast_tvnow(void),
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}
)

struct timeval ast_tvfromboot (void);

/*!
 * \brief Returns the sum of two timevals a + b
 */
struct timeval ast_tvadd(struct timeval a, struct timeval b);

/*!
 * \brief Returns the difference of two timevals a - b
 */
struct timeval ast_tvsub(struct timeval a, struct timeval b);

/*!
 * \brief Returns a timeval from sec, usec
 */
AST_INLINE_API(
struct timeval ast_tv(ast_time_t sec, ast_suseconds_t usec),
{
	struct timeval t;
	t.tv_sec = sec;
	t.tv_usec = usec;
	return t;
}
)

/*!
 * \brief Returns a timeval corresponding to the duration of n samples at rate r.
 * Useful to convert samples to timevals, or even milliseconds to timevals
 * in the form ast_samp2tv(milliseconds, 1000)
 */
AST_INLINE_API(
struct timeval ast_samp2tv(unsigned int _nsamp, unsigned int _rate),
{
	return ast_tv(_nsamp / _rate, (_nsamp % _rate) * (1000000 / _rate));
}
)

#endif /* _ASTERISK_TIME_H */
