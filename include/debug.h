/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _UAPI_INCLUDE_DEBUG_H
#define _UAPI_INCLUDE_DEBUG_H

#include "err.h"
#include "types.h"

#ifndef LIGHTS_MODULE
#error LIGHTS_MODULE must be defined
#endif

#define __LIGHTS_PREFIX "lights " LIGHTS_MODULE ": "

#ifdef DEBUG
#define _IS_NULL(_1) WARN(NULL == (_1), __LIGHTS_PREFIX "arg '%s' is NULL", #_1)
#define _EXEC_1(X,_1) X(_1)
#define _EXEC_2(X,_1,_2) (X(_1) || X(_2))
#define _EXEC_3(X,_1,_2,_3) (X(_1) || X(_2) || X(_3))
#define _EXEC_4(X,_1,_2,_3,_4) (X(_1) || X(_2) || X(_3) || X(_4))
#define _EXEC(_0, _1, _2, _3, _4, N, ...) N
#define IS_NULL(...) \
    _EXEC("dummy", ##__VA_ARGS__, _EXEC_4, _EXEC_3, _EXEC_2, _EXEC_1)(_IS_NULL, ##__VA_ARGS__)
#define IS_TRUE(_1) WARN(_1, __LIGHTS_PREFIX "expr '%s' is TRUE", #_1)
#define IS_FALSE(_1) WARN(!(_1), "lights hw: expr '%s' is FALSE", #_1)
#else
#define IS_NULL(...)
#define IS_TRUE(_1)
#define IS_FALSE(_1)
#endif

#define LIGHTS_ERR(_fmt, ...)({ \
    pr_err(__LIGHTS_PREFIX "[%s:%d] " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
})

#define LIGHTS_WARN(_fmt, ...) ({ \
    pr_warn(__LIGHTS_PREFIX "[%s:%d] " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
})

#define LIGHTS_DBG(_fmt, ...) ({ \
    pr_debug(__LIGHTS_PREFIX "[%s:%d] " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
})

#define LIGHTS_INFO(_fmt, ...) ({ \
    pr_info(__LIGHTS_PREFIX "[%s:%d] " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
})

#define LIGHTS_TRACE(_fmt, ...) ({ \
    pr_debug(__LIGHTS_PREFIX "[%s:%d] " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
})

#define LIGHTS_DUMP(p) ({ \
    print_hex_dump_bytes(__LIGHTS_PREFIX "\n", DUMP_PREFIX_NONE, (p), sizeof(*(p)) ); \
})

#define ERR_NAME strerr

#define CLEAR_ERR(_var) ({ \
    error_t ___err = PTR_ERR(_var); \
    if (___err == 0) ___err = -ENOMEM; \
    LIGHTS_DBG("Failed to assign '%s': %s", #_var, ERR_NAME(___err)); \
    _var = NULL; \
    ___err; \
})

#endif
