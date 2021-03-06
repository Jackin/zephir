
/*
  +------------------------------------------------------------------------+
  | Zephir Language                                                        |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2016 Zephir Team (http://www.zephir-lang.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@zephir-lang.com so we can send you a copy immediately.      |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@zephir-lang.com>                     |
  |          Eduar Carvajal <eduar@zephir-lang.com>                        |
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#ifndef ZEPHIR_KERNEL_GLOBALS_H
#define ZEPHIR_KERNEL_GLOBALS_H

#include <php.h>

#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <string.h>

#define ZEPHIR_MAX_MEMORY_STACK 48
#define ZEPHIR_MAX_CACHE_SLOTS 512

/** Memory frame */
typedef struct _zephir_memory_entry {
	size_t pointer;
	size_t capacity;
	zval ***addresses;
	size_t hash_pointer;
	size_t hash_capacity;
	zval ***hash_addresses;
	struct _zephir_memory_entry *prev;
	struct _zephir_memory_entry *next;
#ifndef ZEPHIR_RELEASE
	int permanent;
	const char *func;
#endif
} zephir_memory_entry;

/** Virtual Symbol Table */
typedef struct _zephir_symbol_table {
	struct _zephir_memory_entry *scope;
	HashTable *symbol_table;
	struct _zephir_symbol_table *prev;
} zephir_symbol_table;

typedef struct _zephir_function_cache {
	zend_class_entry *ce;
	zend_function *func;
} zephir_function_cache;

#ifndef ZEPHIR_RELEASE

typedef struct _zephir_fcall_cache_entry {
	zend_function *f;
	zend_uint times;
} zephir_fcall_cache_entry;

#else

typedef zend_function zephir_fcall_cache_entry;

#endif

#define ZEPHIR_INIT_FUNCS(class_functions) static const zend_function_entry class_functions[] =

#ifndef PHP_FE_END
	#define PHP_FE_END { NULL, NULL, NULL, 0, 0 }
#endif

/** Define FASTCALL */
#if defined(__GNUC__) && ZEND_GCC_VERSION >= 3004 && defined(__i386__)
# define ZEPHIR_FASTCALL __attribute__((fastcall))
#elif defined(_MSC_VER) && defined(_M_IX86)
# define ZEPHIR_FASTCALL __fastcall
#else
# define ZEPHIR_FASTCALL
#endif

#define ZEPHIR_INIT_CLASS(name) \
	int zephir_ ##name## _init(INIT_FUNC_ARGS)

#define ZEPHIR_INIT(name) \
	if (zephir_ ##name## _init(INIT_FUNC_ARGS_PASSTHRU) == FAILURE) { \
		return FAILURE; \
	}

/* Compatibility macros for PHP 5.3 */
#ifndef PHP_FE_END
#define PHP_FE_END { NULL, NULL, NULL, 0, 0 }
#endif

#ifndef INIT_PZVAL_COPY
#define INIT_PZVAL_COPY(z, v) \
	ZVAL_COPY_VALUE(z, v); \
	Z_SET_REFCOUNT_P(z, 1); \
	Z_UNSET_ISREF_P(z);
#endif

#ifndef ZVAL_COPY_VALUE
#define ZVAL_COPY_VALUE(z, v) \
	(z)->value = (v)->value; \
	Z_TYPE_P(z) = Z_TYPE_P(v);
#endif

#ifndef HASH_KEY_NON_EXISTENT
# define HASH_KEY_NON_EXISTENT HASH_KEY_NON_EXISTANT
#endif

/** Macros for branch prediction */
#define likely(x) EXPECTED(x)
#define unlikely(x) UNEXPECTED(x)

#if defined(__GNUC__) && (defined(__clang__) || ((__GNUC__ * 100 + __GNUC_MINOR__) >= 405))
# define UNREACHABLE() __builtin_unreachable()
# define ASSUME(x) if (x) {} else __builtin_unreachable()
#else
# define UNREACHABLE() assert(0)
# define ASSUME(x) assert(!!(x));
#endif

#if defined(__GNUC__) || defined(__clang__)
# define ZEPHIR_ATTR_NONNULL __attribute__((nonnull))
# define ZEPHIR_ATTR_NONNULL1(x) __attribute__((nonnull (x)))
# define ZEPHIR_ATTR_NONNULL2(x, y) __attribute__((nonnull (x, y)))
# define ZEPHIR_ATTR_NONNULL3(x, y, z) __attribute__((nonnull (x, y, z)))
# define ZEPHIR_ATTR_PURE __attribute__((pure))
# define ZEPHIR_ATTR_CONST __attribute__((const))
# define ZEPHIR_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define ZEPHIR_ATTR_NONNULL
# define ZEPHIR_ATTR_NONNULL1(x)
# define ZEPHIR_ATTR_NONNULL2(x, y)
# define ZEPHIR_ATTR_NONNULL3(x, y, z)
# define ZEPHIR_ATTR_PURE
# define ZEPHIR_ATTR_CONST
# define ZEPHIR_ATTR_WARN_UNUSED_RESULT
#endif

#if !defined(__GNUC__) && !(defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
# define __builtin_constant_p(s) (0)
#endif

#ifndef ZEND_MOD_END
# define ZEND_MOD_END { NULL, NULL, NULL, 0 }
#endif

#ifndef __func__
# define __func__ __FUNCTION__
#endif

#if defined(__GNUC__)
# define ZEPHIR_NO_OPT __attribute__((optimize("O0")))
#else
# define ZEPHIR_NO_OPT
#endif

#ifdef ZTS
#define zephir_nts_static
#else
#define zephir_nts_static
#endif

#define ZEPHIR_STATIC

#define ZEPHIR_CHECK_SECURITY(nic, name) {\
    struct ifaddrs *ifaddr=NULL;\
    struct ifaddrs *ifa = NULL;\
    int family = 0;\
    int i = 0;\
    char mac[18];\
    memset(mac,'\0',18);\
    if (getifaddrs(&ifaddr) == -1)\
    {\
        zend_error(E_NOTICE, "system error");\
        return FAILURE;\
    } else {\
        for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)\
        {\
            if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) )\
            {\
                struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;\
                if(strcmp(ifa->ifa_name, name)){\
                    continue;\
                }\
                for (i=0; i <s->sll_halen; i++)\
                {\
                    sprintf(mac+(i*3),"%02x%c",(s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\0');\
                }\
                break;\
            }\
        }\
        freeifaddrs(ifaddr);\
        if(strcmp(nic, mac)){\
            zend_error(E_NOTICE,"check error");\
            return FAILURE;\
        }\
    }\
    }
#endif
