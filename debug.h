#ifndef MX_DEBUG_H
#define MX_DEBUG_H

#include <stdio.h> // fprintf

#ifdef DEBUG
#define MX_DEBUG_PRINT 1
#else
#define MX_DEBUG_PRINT 0
#endif

///////////////////////////////////////////////////////////////////////////////
// The following macro is based on a similar example found on StackOverflow.
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
///////////////////////////////////////////////////////////////////////////////
#define mxDebug(fmt, ...) \
        do { if (MX_DEBUG_PRINT) fprintf(stderr, "%s:%d:%s(): " fmt "\n", __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

///////////////////////////////////////////////////////////////////////////////
// The next is an additional macro for printing just a string, for convenience.
///////////////////////////////////////////////////////////////////////////////
#define mxDebugStr(s) mxDebug("%s", s);

#endif /* MX_DEBUG_H */