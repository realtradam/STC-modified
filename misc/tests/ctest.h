/* Copyright 2011-2023 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CTEST_H
#define CTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define CTEST_IMPL_FORMAT_PRINTF(a, b) __attribute__ ((format(printf, a, b)))
#else
#define CTEST_IMPL_FORMAT_PRINTF(a, b)
#endif

#include <inttypes.h> /* intmax_t, uintmax_t, PRI* */
#include <stdbool.h> /* bool, true, false */
#include <stddef.h> /* size_t */

typedef void (*ctest_nullary_run_func)(void);
typedef void (*ctest_unary_run_func)(void*);
typedef void (*ctest_setup_func)(void*);
typedef void (*ctest_teardown_func)(void*);

union ctest_run_func_union {
    ctest_nullary_run_func nullary;
    ctest_unary_run_func unary;
};

#define CTEST_IMPL_PRAGMA(x) _Pragma (#x)
#define CTEST_CONTAINER_OF(p, T, m) \
    ((T*)((char*)(p) + 0*sizeof((p) == &((T*)0)->m) - offsetof(T, m)))

#if defined(__GNUC__)
#if defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
/* the GCC argument will work for both gcc and clang  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w) \
    CTEST_IMPL_PRAGMA(GCC diagnostic push) \
    CTEST_IMPL_PRAGMA(GCC diagnostic ignored "-W" #w)
#define CTEST_IMPL_DIAG_POP() \
    CTEST_IMPL_PRAGMA(GCC diagnostic pop)
#else
/* the push/pop functionality wasn't in gcc until 4.6, fallback to "ignored"  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w) \
    CTEST_IMPL_PRAGMA(GCC diagnostic ignored "-W" #w)
#define CTEST_IMPL_DIAG_POP()
#endif
#else
/* leave them out entirely for non-GNUC compilers  */
#define CTEST_IMPL_DIAG_PUSH_IGNORED(w)
#define CTEST_IMPL_DIAG_POP()
#endif

struct ctest {
    uint32_t magic0, padding;

    const char* ssname;  // suite name
    const char* ttname;  // test name
    union ctest_run_func_union run;

    void* data;
    ctest_setup_func* setup;
    ctest_teardown_func* teardown;

    int32_t skip;
    uint32_t magic1;
};

#define CTEST_IMPL_NAME(name) ctest_##name
#define CTEST_IMPL_FNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_run)
#define CTEST_IMPL_TNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname)
#define CTEST_IMPL_DATA_SNAME(sname) CTEST_IMPL_NAME(sname##_data)
#define CTEST_IMPL_DATA_TNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_data)
#define CTEST_IMPL_SETUP_FNAME(sname) CTEST_IMPL_NAME(sname##_setup)
#define CTEST_IMPL_SETUP_FPNAME(sname) CTEST_IMPL_NAME(sname##_setup_ptr)
#define CTEST_IMPL_SETUP_TPNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_setup_ptr)
#define CTEST_IMPL_TEARDOWN_FNAME(sname) CTEST_IMPL_NAME(sname##_teardown)
#define CTEST_IMPL_TEARDOWN_FPNAME(sname) CTEST_IMPL_NAME(sname##_teardown_ptr)
#define CTEST_IMPL_TEARDOWN_TPNAME(sname, tname) CTEST_IMPL_NAME(sname##_##tname##_teardown_ptr)

#ifdef __APPLE__
#define CTEST_IMPL_SECTION __attribute__ ((used, section ("__DATA, .ctest"), aligned(1)))
#elif !defined _MSC_VER
#define CTEST_IMPL_SECTION __attribute__ ((used, section (".ctest"), aligned(1)))
#else
#define CTEST_IMPL_SECTION
#endif

#define CTEST_IMPL_STRUCT(sname, tname, tskip, tdata, tsetup, tteardown) \
    static struct ctest CTEST_IMPL_TNAME(sname, tname) CTEST_IMPL_SECTION = { \
        0xBADCAFE0, 0, \
        #sname, \
        #tname, \
        { (ctest_nullary_run_func) CTEST_IMPL_FNAME(sname, tname) }, \
        tdata, \
        (ctest_setup_func*) tsetup, \
        (ctest_teardown_func*) tteardown, \
        tskip, \
        0xBADCAFE1, \
    }

#ifdef __cplusplus

#define CTEST_SETUP(sname) \
    template <> void CTEST_IMPL_SETUP_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#define CTEST_TEARDOWN(sname) \
    template <> void CTEST_IMPL_TEARDOWN_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#define CTEST_FIXTURE(sname) \
    template <typename T> void CTEST_IMPL_SETUP_FNAME(sname)(T* _self) { } \
    template <typename T> void CTEST_IMPL_TEARDOWN_FNAME(sname)(T* _self) { } \
    struct CTEST_IMPL_DATA_SNAME(sname)

#define CTEST_IMPL_CTEST(sname, tname, tskip) \
    static void CTEST_IMPL_FNAME(sname, tname)(void); \
    CTEST_IMPL_STRUCT(sname, tname, tskip, NULL, NULL, NULL); \
    static void CTEST_IMPL_FNAME(sname, tname)(void)

#define CTEST_IMPL_CTEST_F(sname, tname, tskip) \
    static struct CTEST_IMPL_DATA_SNAME(sname) CTEST_IMPL_DATA_TNAME(sname, tname); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self); \
    static void (*CTEST_IMPL_SETUP_TPNAME(sname, tname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_SETUP_FNAME(sname)<struct CTEST_IMPL_DATA_SNAME(sname)>; \
    static void (*CTEST_IMPL_TEARDOWN_TPNAME(sname, tname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_TEARDOWN_FNAME(sname)<struct CTEST_IMPL_DATA_SNAME(sname)>; \
    CTEST_IMPL_STRUCT(sname, tname, tskip, &CTEST_IMPL_DATA_TNAME(sname, tname), &CTEST_IMPL_SETUP_TPNAME(sname, tname), &CTEST_IMPL_TEARDOWN_TPNAME(sname, tname)); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#else

#define CTEST_SETUP(sname) \
    static void CTEST_IMPL_SETUP_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self); \
    static void (*CTEST_IMPL_SETUP_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_SETUP_FNAME(sname); \
    static void CTEST_IMPL_SETUP_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#define CTEST_TEARDOWN(sname) \
    static void CTEST_IMPL_TEARDOWN_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self); \
    static void (*CTEST_IMPL_TEARDOWN_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*) = &CTEST_IMPL_TEARDOWN_FNAME(sname); \
    static void CTEST_IMPL_TEARDOWN_FNAME(sname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#define CTEST_FIXTURE(sname) \
    struct CTEST_IMPL_DATA_SNAME(sname); \
    static void (*CTEST_IMPL_SETUP_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*); \
    static void (*CTEST_IMPL_TEARDOWN_FPNAME(sname))(struct CTEST_IMPL_DATA_SNAME(sname)*); \
    struct CTEST_IMPL_DATA_SNAME(sname)

#define CTEST_IMPL_CTEST(sname, tname, tskip) \
    static void CTEST_IMPL_FNAME(sname, tname)(void); \
    CTEST_IMPL_STRUCT(sname, tname, tskip, NULL, NULL, NULL); \
    static void CTEST_IMPL_FNAME(sname, tname)(void)

#define CTEST_IMPL_CTEST_F(sname, tname, tskip) \
    static struct CTEST_IMPL_DATA_SNAME(sname) CTEST_IMPL_DATA_TNAME(sname, tname); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self); \
    CTEST_IMPL_STRUCT(sname, tname, tskip, &CTEST_IMPL_DATA_TNAME(sname, tname), &CTEST_IMPL_SETUP_FPNAME(sname), &CTEST_IMPL_TEARDOWN_FPNAME(sname)); \
    static void CTEST_IMPL_FNAME(sname, tname)(struct CTEST_IMPL_DATA_SNAME(sname)* _self)

#endif

void CTEST_LOG(const char* fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);
void CTEST_ERR(const char* fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);  // doesn't return

#define CTEST(sname, tname) CTEST_IMPL_CTEST(sname, tname, 0)
#define CTEST_SKIP(sname, tname) CTEST_IMPL_CTEST(sname, tname, 1)

#define CTEST_F(sname, tname) CTEST_IMPL_CTEST_F(sname, tname, 0)
#define CTEST_F_SKIP(sname, tname) CTEST_IMPL_CTEST_F(sname, tname, 1)

void assert_str(const char* cmp, const char* exp, const char* real, const char* caller, int line);
#define ASSERT_STREQ(exp, real) assert_str("==", exp, real, __FILE__, __LINE__)
#define ASSERT_STRNE(exp, real) assert_str("!=", exp, real, __FILE__, __LINE__)
#define ASSERT_SUBSTR(substr, real) assert_str("=~", substr, real, __FILE__, __LINE__)
#define ASSERT_NOT_SUBSTR(substr, real) assert_str("!~", substr, real, __FILE__, __LINE__)

void assert_wstr(const char* cmp, const wchar_t *exp, const wchar_t *real, const char* caller, int line);
#define ASSERT_WSTREQ(exp, real) assert_wstr("==", exp, real, __FILE__, __LINE__)
#define ASSERT_WSTRNE(exp, real) assert_wstr("!=", exp, real, __FILE__, __LINE__)

#define CTEST_FLT_EPSILON 1e-5
#define CTEST_DBL_EPSILON 1e-12

void assert_compare(const char* cmp, intmax_t exp, intmax_t real, const char* caller, int line);
#define ASSERT_EQ(v1, v2) assert_compare("==", v1, v2, __FILE__, __LINE__)
#define ASSERT_NE(v1, v2) assert_compare("!=", v1, v2, __FILE__, __LINE__)
#define ASSERT_LT(v1, v2) assert_compare("<", v1, v2, __FILE__, __LINE__)
#define ASSERT_LE(v1, v2) assert_compare("<=", v1, v2, __FILE__, __LINE__)
#define ASSERT_GT(v1, v2) assert_compare(">", v1, v2, __FILE__, __LINE__)
#define ASSERT_GE(v1, v2) assert_compare(">=", v1, v2, __FILE__, __LINE__)

void assert_interval(intmax_t low, intmax_t high, intmax_t real, const char* caller, int line);
#define ASSERT_INTERVAL(low, high, real) assert_interval(low, high, real, __FILE__, __LINE__)

void assert_pointers(const char* cmp, const void* exp, const void* real, const char* caller, int line);
#define ASSERT_PTR_EQ(exp, real) ((void)sizeof((exp) == (real)), assert_pointers("==", exp, real, __FILE__, __LINE__))
#define ASSERT_PTR_NE(exp, real) ((void)sizeof((exp) != (real)), assert_pointers("!=", exp, real, __FILE__, __LINE__))
#define ASSERT_NULL(real) ASSERT_PTR_EQ(NULL, real)
#define ASSERT_NOT_NULL(real) ASSERT_PTR_NE(NULL, real)

void assert_bool(bool exp, bool real, const char* caller, int line);
#define ASSERT_TRUE(real) assert_bool(true, real, __FILE__, __LINE__)
#define ASSERT_FALSE(real) assert_bool(false, real, __FILE__, __LINE__)

void assert_fail(const char* caller, int line);
#define ASSERT_FAIL() assert_fail(__FILE__, __LINE__)

void assert_dbl_compare(const char* cmp, double exp, double real, double tol, const char* caller, int line);
#define ASSERT_NEAR(exp, real, tol) assert_dbl_compare("==", exp, real, tol, __FILE__, __LINE__)
#define ASSERT_NOT_NEAR(exp, real, tol) assert_dbl_compare("!=", exp, real, tol, __FILE__, __LINE__)
#define ASSERT_DOUBLE_EQ(exp, real) assert_dbl_compare("==", exp, real, -CTEST_DBL_EPSILON, __FILE__, __LINE__)
#define ASSERT_DOUBLE_NE(exp, real) assert_dbl_compare("!=", exp, real, -CTEST_DBL_EPSILON, __FILE__, __LINE__)
#define ASSERT_DOUBLE_LT(v1, v2) assert_dbl_compare("<", v1, v2, 0.0, __FILE__, __LINE__)
#define ASSERT_DOUBLE_GT(v1, v2) assert_dbl_compare(">", v1, v2, 0.0, __FILE__, __LINE__)

#define ASSERT_FLOAT_EQ(v1, v2) assert_dbl_compare("==", v1, v2, -CTEST_FLT_EPSILON, __FILE__, __LINE__)
#define ASSERT_FLOAT_NE(v1, v2) assert_dbl_compare("!=", v1, v2, -CTEST_FLT_EPSILON, __FILE__, __LINE__)
#define ASSERT_FLOAT_LT(v1, v2) assert_dbl_compare("<", (float)(v1), (float)(v2), 0.0, __FILE__, __LINE__)
#define ASSERT_FLOAT_GT(v1, v2) assert_dbl_compare(">", (float)(v1), (float)(v2), 0.0, __FILE__, __LINE__)

#ifdef CTEST_MAIN

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if !defined(_WIN32) || defined(__GNUC__)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

static size_t ctest_errorsize;
static char* ctest_errormsg;
#define MSG_SIZE 4096
static char ctest_errorbuffer[MSG_SIZE];
static jmp_buf ctest_err;
static int color_output = 1;
static const char* suite_name;

typedef int (*ctest_filter_func)(struct ctest*);

#define ANSI_BLACK    "\033[0;30m"
#define ANSI_RED      "\033[0;31m"
#define ANSI_GREEN    "\033[0;32m"
#define ANSI_YELLOW   "\033[0;33m"
#define ANSI_BLUE     "\033[0;34m"
#define ANSI_MAGENTA  "\033[0;35m"
#define ANSI_CYAN     "\033[0;36m"
#define ANSI_GREY     "\033[0;37m"
#define ANSI_DARKGREY "\033[01;30m"
#define ANSI_BRED     "\033[01;31m"
#define ANSI_BGREEN   "\033[01;32m"
#define ANSI_BYELLOW  "\033[01;33m"
#define ANSI_BBLUE    "\033[01;34m"
#define ANSI_BMAGENTA "\033[01;35m"
#define ANSI_BCYAN    "\033[01;36m"
#define ANSI_WHITE    "\033[01;37m"
#define ANSI_NORMAL   "\033[0m"

CTEST(suite, test) { }

static void vprint_errormsg(const char* const fmt, va_list ap) CTEST_IMPL_FORMAT_PRINTF(1, 0);
static void print_errormsg(const char* const fmt, ...) CTEST_IMPL_FORMAT_PRINTF(1, 2);

static void vprint_errormsg(const char* const fmt, va_list ap) {
    // (v)snprintf returns the number that would have been written
    const int ret = vsnprintf(ctest_errormsg, ctest_errorsize, fmt, ap);
    if (ret < 0) {
        ctest_errormsg[0] = 0x00;
    } else {
        const size_t size = (size_t) ret;
        const size_t s = (ctest_errorsize <= size ? size -ctest_errorsize : size);
        // ctest_errorsize may overflow at this point
        ctest_errorsize -= s;
        ctest_errormsg += s;
    }
}

static void print_errormsg(const char* const fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);
}

static void msg_start(const char* color, const char* title) {
    if (color_output) {
        print_errormsg("%s", color);
    }
    print_errormsg("  %s: ", title);
}

static void msg_end(void) {
    if (color_output) {
        print_errormsg(ANSI_NORMAL);
    }
    print_errormsg("\n");
}

void CTEST_LOG(const char* fmt, ...)
{
    va_list argp;
    msg_start(ANSI_BLUE, "LOG");

    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);

    msg_end();
}

CTEST_IMPL_DIAG_PUSH_IGNORED(missing-noreturn)

void CTEST_ERR(const char* fmt, ...)
{
    va_list argp;
    msg_start(ANSI_YELLOW, "ERR");

    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);

    msg_end();
    longjmp(ctest_err, 1);
}

CTEST_IMPL_DIAG_POP()

void assert_str(const char* cmp, const char* exp, const char*  real, const char* caller, int line) {
    if ((!exp ^ !real) || (exp && (
        (cmp[1] == '=' && ((cmp[0] == '=') ^ (strcmp(real, exp) == 0))) ||
        (cmp[1] == '~' && ((cmp[0] == '=') ^ (strstr(real, exp) != NULL)))
    ))) {
        CTEST_ERR("%s:%d  assertion failed, '%s' %s '%s'", caller, line, exp, cmp, real);
    }
}

void assert_wstr(const char* cmp, const wchar_t *exp, const wchar_t *real, const char* caller, int line) {
    if ((!exp ^ !real) || (exp && (
        (cmp[1] == '=' && ((cmp[0] == '=') ^ (wcscmp(real, exp) == 0))) ||
        (cmp[1] == '~' && ((cmp[0] == '=') ^ (wcsstr(real, exp) != NULL)))
    ))) {
        CTEST_ERR("%s:%d  assertion failed, '%ls' %s '%ls'", caller, line, exp, cmp, real);
    }
}

static bool get_compare_result(const char* cmp, int c3, bool eq) {
    if (cmp[0] == '<')
        return c3 < 0 || ((cmp[1] == '=') & eq);
    if (cmp[0] == '>')
        return c3 > 0 || ((cmp[1] == '=') & eq);
    return (cmp[0] == '=') == eq;
}

void assert_compare(const char* cmp, intmax_t exp, intmax_t real, const char* caller, int line) {
    int c3 = (real < exp) - (exp < real);

    if (!get_compare_result(cmp, c3, c3 == 0)) {
        CTEST_ERR("%s:%d  assertion failed, %" PRIdMAX " %s %" PRIdMAX "", caller, line, exp, cmp, real);
    }
}

void assert_interval(intmax_t low, intmax_t high, intmax_t real, const char* caller, int line) {
    if (real < low || real > high) {
        CTEST_ERR("%s:%d  expected %" PRIdMAX "-%" PRIdMAX ", got %" PRIdMAX, caller, line, low, high, real);
    }
}

static bool approximately_equal(double a, double b, double epsilon) {
    double d = a - b;
    if (d < 0) d = -d;
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    return d <= (a > b ? a : b)*epsilon;     /* D.Knuth */
}

/* tol < 0 means it is an epsilon, else absolute error */
void assert_dbl_compare(const char* cmp, double exp, double real, double tol, const char* caller, int line) {
    double diff = exp - real;
    double absdiff = diff < 0 ? -diff : diff;
    int c3 = (real < exp) - (exp < real);
    bool eq = tol < 0 ? approximately_equal(exp, real, -tol) : absdiff <= tol;

    if (!get_compare_result(cmp, c3, eq)) {
        const char* tolstr = "tol";
        if (tol < 0) {
            tolstr = "eps";
            tol = -tol;
        }
        CTEST_ERR("%s:%d  assertion failed, %.8g %s %.8g (diff %.4g, %s %.4g)", caller, line, exp, cmp, real, diff, tolstr, tol);
    }
}

void assert_pointers(const char* cmp, const void* exp, const void* real, const char* caller, int line) {
    if ((exp == real) != (cmp[0] == '=')) {
        CTEST_ERR("%s:%d  assertion failed (0x%02llx) %s (0x%02llx)", caller, line,
                  (unsigned long long)(uintptr_t)exp , cmp, (unsigned long long)(uintptr_t)real);
    }
}

void assert_bool(bool exp, bool real, const char* caller, int line) {
    if (exp != real) {
        CTEST_ERR("%s:%d  should be %s", caller, line, exp ? "true" : "false");
    }
}

void assert_fail(const char* caller, int line) {
    CTEST_ERR("%s:%d  shouldn't come here", caller, line);
}


static int suite_all(struct ctest* t) {
    (void) t; // avoid unused parameter warning
    return 1;
}

static int suite_filter(struct ctest* t) {
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

static void color_print(const char* color, const char* text) {
    if (color_output)
        printf("%s%s" ANSI_NORMAL "\n", color, text);
    else
        printf("%s\n", text);
}

#ifndef CTEST_NO_SEGFAULT
#include <signal.h>
static void sighandler(int signum)
{
    const char msg_color[] = ANSI_BRED "[SIGSEGV: Segmentation fault]" ANSI_NORMAL "\n";
    const char msg_nocolor[] = "[SIGSEGV: Segmentation fault]\n";

    const char* msg = color_output ? msg_color : msg_nocolor;
    intptr_t n = write(1, msg, (unsigned int)strlen(msg));
    (void)n;
    /* "Unregister" the signal handler and send the signal back to the process
     * so it can terminate as expected */
    signal(signum, SIG_DFL);
#if !defined(_WIN32) || defined(__CYGWIN__)
    kill(getpid(), signum);
#endif
}
#endif

int ctest_main(int argc, const char *argv[]);
#ifdef __GNUC__
__attribute__((no_sanitize_address))
#endif
int ctest_main(int argc, const char *argv[])
{
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int idx = 1;
    static ctest_filter_func filter = suite_all;

#ifndef CTEST_NO_SEGFAULT
    signal(SIGSEGV, sighandler);
#endif

    if (argc == 2) {
        suite_name = argv[1];
        filter = suite_filter;
    }
#ifdef CTEST_NO_COLORS
    color_output = 0;
#else
    color_output = isatty(1);
#endif
    clock_t t1 = clock();

    uint32_t* magic_begin = &CTEST_IMPL_TNAME(suite, test).magic1;
    uint32_t* magic_end = &CTEST_IMPL_TNAME(suite, test).magic0, *m;
    size_t num_ints = sizeof(struct ctest)/sizeof *m;

#if (defined __TINYC__ && defined __unix__)
    #define CTEST_IMPL_MAGIC_SEEK 10 /* search 4*(1+10) bytes outside ctest entry bounds */
#else
    #define CTEST_IMPL_MAGIC_SEEK 0 /* access only 4 bytes outside outer ctest entry bounds */
#endif
    for (m = magic_begin; magic_begin - m <= num_ints + CTEST_IMPL_MAGIC_SEEK; --m) {
        if (*m == 0xBADCAFE1) {
            magic_begin = m;
            m -= num_ints - 1;
        }
    }
    for (m = magic_end; m - magic_end <= num_ints; ++m) {
        if (*m == 0xBADCAFE0) {
            magic_end = m;
            m += num_ints - 1;
        }
    }
    magic_begin = &CTEST_CONTAINER_OF(magic_begin, struct ctest, magic1)->magic0;

    static struct ctest* test;
    for (m = magic_begin; m <= magic_end; m += num_ints) {
        while (*m != 0xBADCAFE0) ++m;
        test = CTEST_CONTAINER_OF(m, struct ctest, magic0);
        if (test == &CTEST_IMPL_TNAME(suite, test)) continue;
        if (filter(test)) total++;
    }

    for (m = magic_begin; m <= magic_end; m += num_ints) {
        while (*m != 0xBADCAFE0) ++m;
        test = CTEST_CONTAINER_OF(m, struct ctest, magic0);
        if (test == &CTEST_IMPL_TNAME(suite, test)) continue;
        if (filter(test)) {
            ctest_errorbuffer[0] = 0;
            ctest_errorsize = MSG_SIZE-1;
            ctest_errormsg = ctest_errorbuffer;
            printf("TEST %d/%d %s:%s ", idx, total, test->ssname, test->ttname);
            fflush(stdout);
            if (test->skip) {
                color_print(ANSI_BYELLOW, "[SKIPPED]");
                num_skip++;
            } else {
                int result = setjmp(ctest_err);
                if (result == 0) {
                    if (test->setup && *test->setup) (*test->setup)(test->data);
                    if (test->data)
                        test->run.unary(test->data);
                    else
                        test->run.nullary();
                    if (test->teardown && *test->teardown) (*test->teardown)(test->data);
                    // if we got here it's ok
#ifdef CTEST_NO_COLOR_OK
                    printf("[OK]\n");
#else
                    color_print(ANSI_BGREEN, "[OK]");
#endif
                    num_ok++;
                } else {
                    color_print(ANSI_BRED, "[FAIL]");
                    num_fail++;
                }
                if (ctest_errorsize != MSG_SIZE-1) printf("%s", ctest_errorbuffer);
            }
            idx++;
        }
    }
    clock_t t2 = clock();

    const char* color = (num_fail) ? ANSI_BRED : ANSI_GREEN;
    char results[80];
    snprintf(results, sizeof(results), "RESULTS: %d tests (%d ok, %d failed, %d skipped) ran in %.1f ms",
             total, num_ok, num_fail, num_skip, (double)(t2 - t1)*1000.0/CLOCKS_PER_SEC);
    color_print(color, results);
    return num_fail;
}

#endif

#ifdef __cplusplus
}
#endif

#endif

