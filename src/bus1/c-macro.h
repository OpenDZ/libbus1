#pragma once

/***
  This file is part of bus1. See COPYING for details.

  bus1 is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  bus1 is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with bus1; If not, see <http://www.gnu.org/licenses/>.
***/

/*
 * Macros
 * This header contains macros useful across our codebase. This includes
 * pre-processor macros and a *very* limited set of inlined functions that are
 * used throughout the code-base.
 *
 * As this header is included all over the place, make sure to only add stuff
 * that really belongs all-over-the-place.
 *
 * This header also provides a basic ISO-C11 environment to the callers. This
 * means, we include the very basic set of headers, to avoid copying them to
 * all common callers. This set is quite limited, though, and is considered API
 * of this header.
 *
 * Conventions:
 *  - Any macro written in UPPER-CASE letters might have side-effects and
 *    special behavior. See its comments for details. Usually, such macros
 *    cannot be implemented as normal C-functions, so they behave differently.
 *  - Macros that behave like C-functions (no multiple evaluation, type-safe,
 *    etc.) use lower-case names, like c_min(). If those functions can be
 *    evaluated at compile-time, they *must* support constant folding (i.e.,
 *    you can use them in constant expressions), and they also provide an
 *    equivalent call without any guards, which is written as upper-case name,
 *    like C_MIN(). Those calls do *not* provide any guards, but can rather be
 *    used in file-context, compared to function-context (file-contexts don't
 *    allow statement-expressions).
 *  - If macros support different numbers of arguments, we use the number as
 *    suffix, like C_CC_MACRO2() and C_CC_MACRO3(). Usually, their concept can
 *    be extended to infinity, but the C-preprocessor does not allow it. Hence,
 *    we hard-code the number of arguments.
 *  - Any internal function is prefixed with C_INTERNAL_*() or c_internal_*().
 *    Never call those directly.
 */

/*
 * Basic ISO-C11 headers are considered API of this header. Don't remove them
 * as we rely on them in our sources. This includes: assert, errno, inttypes,
 * limits, stdarg, stdbool, stddef
 */
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
/* must not depend on any other c-header */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * We require:
 *   sizeof(void*) == sizeof(long)
 *   sizeof(long) == 4 || sizeof(long) == 8
 *   sizeof(int) == 4
 * The linux kernel requires the same from the toolchain, so this should work
 * just fine.
 */
#if __SIZEOF_POINTER__ != __SIZEOF_LONG__
#  error "sizeof(void*) != sizeof(long)"
#elif __SIZEOF_LONG__ != 4 && __SIZEOF_LONG__ != 8
#  error "sizeof(long) != 4 && sizeof(long) != 8"
#elif __SIZEOF_INT__ != 4
#  error "sizeof(int) != 4"
#endif

/*
 * Shortcuts for gcc attributes. See GCC manual for details. They're 1-to-1
 * mappings to the GCC equivalents. No additional magic here.
 */
#define _c_align_(_x) __attribute__((__aligned__(_x)))
#define _c_alignas_(_x) __attribute__((__aligned__(__alignof(_x))))
#define _c_alloc_(...) __attribute__((__alloc_size__(__VA_ARGS__)))
#define _c_cleanup_(_x) __attribute__((__cleanup__(_x)))
#define _c_const_ __attribute__((__const__))
#define _c_deprecated_ __attribute__((__deprecated__))
#define _c_hidden_ __attribute__((__visibility__("hidden")))
#define _c_likely_(_x) (__builtin_expect(!!(_x), 1))
#define _c_malloc_ __attribute__((__malloc__))
#define _c_packed_ __attribute__((__packed__))
#define _c_printf_(_a, _b) __attribute__((__format__(printf, _a, _b)))
#define _c_public_ __attribute__((__visibility__("default")))
#define _c_pure_ __attribute__((__pure__))
#define _c_sentinel_ __attribute__((__sentinel__))
#define _c_unlikely_(_x) (__builtin_expect(!!(_x), 0))
#define _c_unused_ __attribute__((__unused__))
#define _c_weak_ __attribute__((__weak__))
#define _c_weakref_(_x) __attribute__((__weakref__(#_x)))

/**
 * C_TYPE_MATCH() - match two variables/types for unqualified equality
 * @_a:         first variable/type
 * @_b:         second variable/type
 *
 * Compare two types, or types of two variables, for equality. Note that type
 * qualifiers are not respected by this comparison. Hence, only the actual
 * underlying types are compared.
 *
 * Return: 1 if both unqualified types are equal, 0 if not.
 */
#define C_TYPE_MATCH(_a, _b) __builtin_types_compatible_p(__typeof__(_a), __typeof__(_b))

/**
 * C_TYPE_IS_ARRAY() - evaluate whether given variable is of an array type
 * @_a:         variable/type to evaluate
 *
 * This function checks whether a given variable is an array type. Note that
 * the passed argument must either be an array or pointer, otherwise, this will
 * generate a syntax error.
 *
 * Note that "&a[0]" degrades an array to a pointer, and as such compares
 * unequal to "a" if it is an array. This is unique to array types.
 *
 * Return: 1 if it is an array type, 0 if not.
 */
#define C_TYPE_IS_ARRAY(_a) (!C_TYPE_MATCH(__typeof__(_a), &(*(__typeof__(_a)*)0)[0]))

/**
 * C_CC_IF() - conditional expression at compile time
 * @_cond:      condition
 * @_if:        if-clause
 * @_else:      else-clause
 *
 * This is a compile-time if-else-statement. Depending on whether the constant
 * expression @_cond is true or false, this evaluates to the passed clause. The
 * other clause is *not* evaluated, however, it may be checked for syntax
 * errors and *constant* expressions are evaluated.
 *
 * Return: Evaluates to either if-clause or else-clause, depending on whether
 *         the condition is true. The other clause is *not* evaluated.
 */
#define C_CC_IF(_cond, _if, _else) __builtin_choose_expr(!!(_cond), _if, _else)

/**
 * C_CC_IS_CONST() - check whether a value is known at compile time
 * @_expr:      expression
 *
 * This checks whether the value of @_expr is known at compile time. Note that
 * a negative result does not mean that it is *NOT* known. However, it means
 * that it cannot be guaranteed to be constant at compile time. Hence, false
 * negatives are possible.
 *
 * This macro *always* evaluates to a constant expression, regardless whether
 * the passed expression is constant.
 *
 * The passed in expression is *never* evaluated. Hence, it can safely be used
 * in combination with C_CC_IF() to avoid multiple evaluations of macro
 * parameters.
 *
 * Return: 1 if constant, 0 if not.
 */
#define C_CC_IS_CONST(_expr) __builtin_constant_p(_expr)

/**
 * C_CC_UNIQUE - generate unique compile-time integer
 *
 * This evaluates to a unique compile-time integer. Each occurrence of this
 * macro in the *preprocessed* C-code resolves to a different, unique integer.
 * Internally, it uses the __COUNTER__ gcc extension, and as such all
 * occurrences generate a dense set of integers.
 *
 * Return: This evaluates to an integer literal
 */
#define C_CC_UNIQUE __COUNTER__

/**
 * C_VAR() - generate unique variable name
 * @_x:         name of variable
 * @_uniq:      unique prefix, usually provided by @C_CC_UNIQUE, optional
 *
 * This macro shall be used to generate unique variable names, that will not be
 * shadowed by recursive macro invocations. It is effectively a
 * C_CONCATENATE of both arguments, but also provides a globally separated
 * prefix and makes the code better readable.
 *
 * The second argument is optional. If not given, __LINE__ is implied, and as
 * such the macro will generate the same identifier if used multiple times on
 * the same code-line (or within a macro). This should be used if recursive
 * calls into the macro are not expected.
 *
 * This helper may be used by macro implementations that might reasonable well
 * be called in a stacked fasion, like:
 *     c_max(foo, c_max(bar, baz))
 * Such a stacked call of c_max() might cause compiler warnings of shadowed
 * variables in the definition of c_max(). By using C_VAR(), such warnings
 * can be silenced as each evaluation of c_max() uses unique variable names.
 *
 * Return: This evaluates to a constant identifier
 */
#define C_VAR(...) C_INTERNAL_VAR(__VA_ARGS__, 2, 1)
#define C_INTERNAL_VAR(_x, _uniq, _num, ...) C_VAR ## _num (_x, _uniq)
#define C_VAR1(_x, _unused) C_VAR2(_x, C_CONCATENATE(line, __LINE__))
#define C_VAR2(_x, _uniq) C_CONCATENATE(c_internal_var_unique_, C_CONCATENATE(_uniq, _x))

/**
 * C_CC_ASSERT_MSG() - compile time assertion
 * @_cond:      condition
 * @_msg:       message to make the compiler print
 *
 * This is a compile-time assertion that can be used in any (constant)
 * expression. If @_cond evalutes to true, this is equivalent to a void
 * expression. If @_cond is false, this will cause a compiler error and print
 * @_msg into the compile log.
 *
 * XXX: Find some gcc hack to print @_msg while keeping the macro a constant
 * expression.
 *
 * Return: This macro evaluates to a void expression.
 */
#define C_CC_ASSERT_MSG(_cond, _msg) ((void)C_CC_ASSERT1_MSG((_cond), _msg))

/**
 * C_CC_ASSERT1_MSG() - compile time assertion
 * @_cond:      condition
 * @_msg:       message to make the compiler print
 *
 * This is the same as C_CC_ASSERT_MSG(), but evaluates to constant 1.
 *
 * Return: This macro evaluates to constant 1.
 */
#define C_CC_ASSERT1_MSG(_cond, _msg) (sizeof(int[!(_cond) * -1]) * 0 + 1)

/**
 * C_CC_ASSERT() - compile time assertion
 * @_cond:      condition
 *
 * Same as C_CC_ASSERT_MSG() but prints the condition as error message.
 *
 * Return: This macro evaluates to a void expression.
 */
#define C_CC_ASSERT(_cond) ((void)C_CC_ASSERT1(_cond))

/**
 * C_CC_ASSERT1() - compile time assertion
 * @_cond:      condition
 *
 * This is the same as C_CC_ASSERT(), but evaluates to constant 1.
 *
 * Return: This macro evaluates to constant 1.
 */
#define C_CC_ASSERT1(_cond) C_CC_ASSERT1_MSG((_cond), #_cond)

/**
 * C_CC_ASSERT_TO() - compile time assertion with explicit return value
 * @_cond:      condition to assert
 * @_expr:      expression to yield
 *
 * This is equivalent to C_CC_ASSERT1(_cond), but yields a return value of
 * @_expr, rather than constant 1.
 *
 * In case the compile-time assertion is false, this causes a compile-time
 * error and *also* evaluates as a void expression (and as such usually causes
 * a followup compile time error).
 *
 * Note that usually you'd do something like:
 *     (ASSERT(cond), expr)
 * thus using the comma-operator to yield a specific value. However,
 * suprisingly STD-C does *not* define the comma operator as constant
 * expression. Hence, we have to use C_CC_IF() to yield the same result.
 *
 * Return: This macro evaluates to @_expr.
 */
#define C_CC_ASSERT_TO(_cond, _expr) C_CC_IF(C_CC_ASSERT1(_cond), (_expr), ((void)0))

/**
 * C_CC_MACRO2() - provide save environment to a macro
 * @_call:      macro to call
 * @_x:         first argument
 * @_y:         second argument
 *
 * This function simplifies the implementation of macros. Whenever you
 * implement a macro, provide the internal macro name as @_call and its
 * arguments as @_x and @_y. Inside of your internal macro, you:
 *  - are safe against multiple evaluation errors
 *  - support constant folding
 *  - have unique variable names for recursive callers
 *  - have properly typed arguments
 *
 * Return: Result of @_call is returned.
 */
#define C_CC_MACRO2(_call, _x, _y) C_INTERNAL_CC_MACRO2(_call, C_CC_UNIQUE, (_x), C_CC_UNIQUE, (_y))
#define C_INTERNAL_CC_MACRO2(_call, _xq, _x, _yq, _y)                   \
        C_CC_IF(                                                        \
                (C_CC_IS_CONST(_x) && C_CC_IS_CONST(_y)),               \
                _call((_x), (_y)),                                      \
                __extension__ ({                                        \
                        const __auto_type C_VAR(X, _xq) = (_x);         \
                        const __auto_type C_VAR(Y, _yq) = (_y);         \
                        _call(C_VAR(X, _xq), C_VAR(Y, _yq));            \
                }))

/**
 * C_CC_MACRO3() - provide save environment to a macro
 * @_call:      macro to call
 * @_x:         first argument
 * @_y:         second argument
 * @_z:         third argument
 *
 * This is the 3-argument equivalent of C_CC_MACRO2().
 *
 * Return: Result of @_call is returned.
 */
#define C_CC_MACRO3(_call, _x, _y, _z) C_INTERNAL_CC_MACRO3(_call, C_CC_UNIQUE, (_x), C_CC_UNIQUE, (_y), C_CC_UNIQUE, (_z))
#define C_INTERNAL_CC_MACRO3(_call, _xq, _x, _yq, _y, _zq, _z)                  \
        C_CC_IF(                                                                \
                (C_CC_IS_CONST(_x) && C_CC_IS_CONST(_y) && C_CC_IS_CONST(_z)),  \
                _call((_x), (_y), (_z)),                                        \
                __extension__ ({                                                \
                        const __auto_type C_VAR(X, _xq) = (_x);                 \
                        const __auto_type C_VAR(Y, _yq) = (_y);                 \
                        const __auto_type C_VAR(Z, _zq) = (_z);                 \
                        _call(C_VAR(X, _xq), C_VAR(Y, _yq), C_VAR(Z, _zq));     \
                }))

/**
 * C_STRINGIFY() - stringify a token, but evaluate it first
 * @_x:         token to evaluate and stringify
 *
 * Return: Evaluates to a constant string literal
 */
#define C_STRINGIFY(_x) C_INTERNAL_STRINGIFY(_x)
#define C_INTERNAL_STRINGIFY(_x) #_x

/**
 * C_CONCATENATE() - concatenate two tokens, but evaluate them first
 * @_x:         first token
 * @_y:         second token
 *
 * Return: Evaluates to a constant identifier
 */
#define C_CONCATENATE(_x, _y) C_INTERNAL_CONCATENATE(_x, _y)
#define C_INTERNAL_CONCATENATE(_x, _y) _x ## _y

/**
 * C_ARRAY_SIZE() - calculate number of array elements at compile time
 * @_x:         array to calculate size of
 *
 * Return: Evaluates to a constant integer expression
 */
#define C_ARRAY_SIZE(_x) C_CC_ASSERT_TO(C_TYPE_IS_ARRAY(_x), sizeof(_x) / sizeof((_x)[0]))

/**
 * C_DECIMAL_MAX() - calculate maximum length of the decimal
 *                   representation of an integer
 * @_type: integer variable/type
 *
 * This calculates the bytes required for the decimal representation of an
 * integer of the given type. It accounts for a possible +/- prefix, but it
 * does *NOT* include the trailing terminating zero byte.
 *
 * Return: Evaluates to a constant integer expression
 */
#define C_DECIMAL_MAX(_type)                                                    \
        _Generic((_type){ 0 },                                                  \
                char:                   C_INTERNAL_DECIMAL_MAX(_type),          \
                signed char:            C_INTERNAL_DECIMAL_MAX(_type),          \
                unsigned char:          C_INTERNAL_DECIMAL_MAX(_type),          \
                signed short:           C_INTERNAL_DECIMAL_MAX(_type),          \
                unsigned short:         C_INTERNAL_DECIMAL_MAX(_type),          \
                signed int:             C_INTERNAL_DECIMAL_MAX(_type),          \
                unsigned int:           C_INTERNAL_DECIMAL_MAX(_type),          \
                signed long:            C_INTERNAL_DECIMAL_MAX(_type),          \
                unsigned long:          C_INTERNAL_DECIMAL_MAX(_type),          \
                signed long long:       C_INTERNAL_DECIMAL_MAX(_type),          \
                unsigned long long:     C_INTERNAL_DECIMAL_MAX(_type))
#define C_INTERNAL_DECIMAL_MAX(_type)                   \
        (1 + (sizeof(_type) <= 1 ?  3 :                 \
              sizeof(_type) <= 2 ?  5 :                 \
              sizeof(_type) <= 4 ? 10 :                 \
              C_CC_ASSERT_TO(sizeof(_type) <= 8, 20)))

/**
 * c_container_of() - cast a member of a structure out to the containing structure
 * @_ptr:       pointer to the member or NULL
 * @_type:      type of the container struct this is embedded in
 * @_member:    name of the member within the struct
 */
#define c_container_of(_ptr, _type, _member) c_internal_container_of(C_CC_UNIQUE, (_ptr), _type, _member)
#define c_internal_container_of(_uniq, _ptr, _type, _member)                                    \
        __extension__ ({                                                                        \
                const __typeof__( ((_type*)0)->_member ) *C_VAR(A, _uniq) = (_ptr);             \
                (_ptr) ? (_type*)( (char*)C_VAR(A, _uniq) - offsetof(_type, _member) ) : NULL;  \
        })

/**
 * c_max() - compute maximum of two values
 * @_a:         value A
 * @_b:         value B
 *
 * Calculate the maximum of both passed values. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: Maximum of both values is returned.
 */
#define c_max(_a, _b) C_CC_MACRO2(C_MAX, (_a), (_b))
#define C_MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))

/**
 * c_min() - compute minimum of two values
 * @_a:         value A
 * @_b:         value B
 *
 * Calculate the minimum of both passed values. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: Minimum of both values is returned.
 */
#define c_min(_a, _b) C_CC_MACRO2(C_MIN, (_a), (_b))
#define C_MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

/**
 * c_less_by() - calculate clamped difference of two values
 * @_a:         minuend
 * @_b:         subtrahend
 *
 * Calculate [_a - _b], but clamp the result to 0. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: This computes [_a - _b], if [_a > _b]. Otherwise, 0 is returned.
 */
#define c_less_by(_a, _b) C_CC_MACRO2(C_LESS_BY, (_a), (_b))
#define C_LESS_BY(_a, _b) ((_a) > (_b) ? (_a) - (_b) : 0)

/**
 * c_clamp() - clamp value to lower and upper boundary
 * @_x:         value to clamp
 * @_low:       lower boundary
 * @_high:      higher boundary
 *
 * This clamps @_x to the lower and higher bounds given as @_low and @_high.
 * All arguments are evaluated exactly once, and yield a constant expression if
 * all arguments are constant as well.
 *
 * Return: Clamped integer value.
 */
#define c_clamp(_x, _low, _high) C_CC_MACRO3(C_CLAMP, (_x), (_low), (_high))
#define C_CLAMP(_x, _low, _high) ((_x) > (_high) ? (_high) : (_x) < (_low) ? (_low) : (_x))

/**
 * c_negative_errno() - return negative errno
 *
 * This helper should be used to shut up gcc if you know 'errno' is valid (ie.,
 * errno is > 0). Instead of "return -errno;", use
 * "return c_negative_errno();" It will suppress bogus gcc warnings in case
 * it assumes 'errno' might be 0 (or <0) and thus the caller's error-handling
 * might not be triggered.
 *
 * This helper should be avoided whenever possible. However, occasionally we
 * really want to shut up gcc (especially with static/inline functions). In
 * those cases, gcc usually cannot deduce that some error paths are guaranteed
 * to be taken. Hence, making the return value explicit allows gcc to better
 * optimize the code.
 *
 * Note that you really should never use this helper to work around broken libc
 * calls or syscalls, not setting 'errno' correctly.
 *
 * Return: Negative error code is returned.
 */
static inline int c_negative_errno(void) {
        return _c_likely_(errno > 0) ? -errno : -EINVAL;
}

/**
 * c_clz() - count leading zeroes
 * @_val:       value to count leading zeroes of
 *
 * This counts the leading zeroes of the binary representation of @_val. Note
 * that @_val must be of an integer type greater than, or equal to, 'unsigned
 * int'. Also note that a value of 0 produces an undefined result (see your CPU
 * instruction manual for details why).
 *
 * This macro evaluates the argument exactly once, and if the input is
 * constant, it also evaluates to a constant expression.
 *
 * Note that this macro calculates the number of leading zeroes within the
 * scope of the integer type of @_val. That is, if the input is a 32bit type
 * with value 1, it yields 31. But if it is a 64bit type with the same value 1,
 * it yields 63.
 *
 * Return: Evaluates to an 'int', the number of leading zeroes.
 */
#define c_clz(_val)                                             \
        _Generic((_val),                                        \
                unsigned int: __builtin_clz(_val),              \
                unsigned long: __builtin_clzl(_val),            \
                unsigned long long: __builtin_clzll(_val))

/**
 * c_align_to() - align value to
 * @_val:       value to align
 * @_to:        align to multiple of this
 *
 * This aligns @_val to a multiple of @_to. If @_val is already a multiple of
 * @_to, @_val is returned unchanged. This function operates within the
 * boundaries of the type of @_val and @_to. Make sure to cast them if needed.
 *
 * The arguments of this macro are evaluated exactly once. If both arguments
 * are a constant expression, this also yields a constant return value.
 *
 * Note that @_to must be a power of 2. In case @_to is a constant expression,
 * this macro places a compile-time assertion on the popcount of @_to, to
 * verify it is a power of 2.
 *
 * Return: @_val aligned to a multiple of @_to
 */
#define c_align_to(_val, _to) C_CC_MACRO2(C_ALIGN_TO, (_val), (_to))
#define C_ALIGN_TO(_val, _to) (((_val) + (_to) - 1) & ~((_to) - 1))

/**
 * c_align() - align to native size
 * @_val:       value to align
 *
 * This is the same as c_align_to((_val), __SIZEOF_POINTER__).
 *
 * Return: @_val aligned to the native size
 */
#define c_align(_val) c_align_to((_val), __SIZEOF_POINTER__)

/**
 * c_align8() - align value to multiple of 8
 * @_val:       value to align
 *
 * This is the same as c_align_to((_val), 8).
 *
 * Return: @_val aligned to a multiple of 8.
 */
#define c_align8(_val) c_align_to((_val), 8)

/**
 * c_align_power2() - align value to next power of 2
 * @_val:       value to align
 *
 * This aligns @_val to the next higher power of 2. If it already is a power of
 * 2, the value is returned unchanged. 0 is treated as power of 2 (so 0 yields
 * 0). Furthermore, on overflow, this yields 0 as well.
 *
 * Note that this always operates within the bounds of the type of @_val.
 *
 * Return: @_val aligned to the next higher power of 2
 */
#define c_align_power2(_val) c_internal_align_power2(C_CC_UNIQUE, (_val))
#define c_internal_align_power2(_vq, _v)                                                                \
        __extension__ ({                                                                                \
                __auto_type C_VAR(v, _vq) = (_v);                                                       \
                /* cannot use ?: as gcc cannot do const-folding then (apparently..) */                  \
                if (C_VAR(v, _vq) == 1) /* clz(0) is undefined */                                       \
                        C_VAR(v, _vq) = 1;                                                              \
                else if (c_clz(C_VAR(v, _vq) - 1) < 1) /* shift overflow is undefined */                \
                        C_VAR(v, _vq) = 0;                                                              \
                else                                                                                    \
                        C_VAR(v, _vq) = ((__typeof__(C_VAR(v, _vq)))1) <<                               \
                                        (sizeof(C_VAR(v, _vq)) * 8 - c_clz(C_VAR(v, _vq) - 1));         \
                C_VAR(v, _vq);                                                                          \
        })

/**
 * c_div_round_up() - calculate integer quotient but round up
 * @_x:         dividend
 * @_y:         divisor
 *
 * Calculates [x / y] but rounds up the result to the next integer. All
 * arguments are evaluated exactly once, and yield a constant expression if all
 * arguments are constant.
 *
 * Note:
 * [(x + y - 1) / y] suffers from an integer overflow, even though the
 * computation should be possible in the given type. Therefore, we use
 * [x / y + !!(x % y)]. Note that on most CPUs a division returns both the
 * quotient and the remainder, so both should be equally fast. Furthermore, if
 * the divisor is a power of two, the compiler will optimize it, anyway.
 *
 * Return: The quotient is returned.
 */
#define c_div_round_up(_x, _y) C_CC_MACRO2(C_DIV_ROUND_UP, (_x), (_y))
#define C_DIV_ROUND_UP(_x, _y) ((_x) / (_y) + !!((_x) % (_y)))

#ifdef __cplusplus
}
#endif
