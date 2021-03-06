#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER tp

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./tp.h"

#if !defined(_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TP_H

/*
 * Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

#include <lttng/tracepoint.h>

#ifdef IVC_ENUM
TRACEPOINT_ENUM(
    tp,
    tp_enum,
    TP_ENUM_VALUES(
        ctf_enum_value("ZERO", 0)
        ctf_enum_value("ONE", 1)
    )
)

TRACEPOINT_EVENT(tp, tpenum,
	TP_ARGS(int, intarg),
	TP_FIELDS(
		ctf_enum(tp, tp_enum, int, enum_field, intarg)
	)
)
#else
TRACEPOINT_EVENT(tp, tpenum,
		TP_ARGS(int, intarg),
		TP_FIELDS(
			ctf_integer(int, intfield, intarg)
		)
)
#endif

TRACEPOINT_EVENT(tp, tptest,
	TP_ARGS(int, anint, int, netint, long *, values,
		char *, text, size_t, textlen,
		double, doublearg, float, floatarg),
	TP_FIELDS(
		ctf_integer(int, intfield, anint)
		ctf_integer_hex(int, intfield2, anint)
		ctf_integer(long, longfield, anint)
		ctf_integer_network(int, netintfield, netint)
		ctf_integer_network_hex(int, netintfieldhex, netint)
		ctf_array(long, arrfield1, values, 3)
		ctf_array_text(char, arrfield2, text, 10)
		ctf_sequence(char, seqfield1, text, size_t, textlen)
		ctf_sequence_text(char, seqfield2, text, size_t, textlen)
		ctf_string(stringfield, text)
		ctf_float(float, floatfield, floatarg)
		ctf_float(double, doublefield, doublearg)
	)
)


#endif /* _TRACEPOINT_TP_H */

/* This part must be outside ifdef protection */
#include <lttng/tracepoint-event.h>
