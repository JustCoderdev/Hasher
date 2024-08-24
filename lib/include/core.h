/*
 * JustCoderdev's Core library v8
 */

#ifndef CORE_H_
#define CORE_H_

#define CORE_STR_VER 8

/* #define SGR_ENABLE 1 */

/* #define DEBUG_ENABLE 1 */
/* #define DEBUG_STRING_ENABLE 1 */
/* #define DEBUG_MEMDEB_ENABLE 1 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>


/* Types
 * ----------------------------------------------------------- */

typedef unsigned char uchar;

typedef unsigned char     n8;
typedef unsigned short    n16;
typedef unsigned int      n32;
typedef unsigned long int n64;

typedef signed char     i8;
typedef signed short    i16;
typedef signed int      i32;
typedef signed long int i64;

#define RL_BOOL_TYPE
typedef enum bool {
	true = (1 == 1),
	false = (1 != 1)
} bool;

typedef enum error {
	success = false,
	failure = true
} error;

typedef int Errno;


/* String module v4
 * ----------------------------------------------------------- */
#define CORE_STR_VER 4

typedef const char* CString;
typedef struct String {
	n32 count, capacity;
	char* chars;
} String;

#define STR_FMT "%.*s"
#define STR(STRING) (STRING).count, (STRING).chars

#define string_new(STRING, CAPACITY) string_new_(STRING, CAPACITY, __FILE__, __LINE__)
#define string_new_from(STRING, TEXT_LEN, TEXT) string_new_from_(STRING, TEXT_LEN, TEXT, __FILE__, __LINE__)
#define string_from(STRING, TEXT_LEN, TEXT) string_from_(STRING, TEXT_LEN, TEXT, __FILE__, __LINE__)
#define string_cpy(STRING, TEXT_LEN, TEXT) string_cpy_(STRING, TEXT_LEN, TEXT, __FILE__, __LINE__)
#define string_nterm(STRING) string_nterm_(STRING, __FILE__, __LINE__)
#define string_append(STRING, CHR) string_append_(STRING, CHR, __FILE__, __LINE__)
#define string_free(STRING) string_free_(STRING, __FILE__, __LINE__)

extern void string_new_(String* string, n32 capacity, char* file, int line);
extern void string_new_from_(String* string, n32 text_len, char* text, char* file, int line);
extern void string_from_(String* string, n32 text_len, char* text, char* file, int line);
extern void string_nterm_(String* string, char* file, int line); /* Null terminate a string */
extern void string_append_(String* string, char chr, char* file, int line);
extern void string_free_(String* string, char* file, int line);

extern void string_clear(String* string);
extern void string_fmt(String* string, CString format, ...);
extern void string_remove(String* string, n32 count);

extern bool string_equals(String strA, n32 strB_len, const char* strB);
extern bool string_equallit(String strA, CString strB);


/* Macros
 * ----------------------------------------------------------- */

#ifndef min /* clamp(0, x, y) = min(x, y) if x is unsigned */
#define min(A, B) ((A) > (B) ? (B) : (A))
#endif

#ifndef max
#define max(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef clamp
#define clamp(MIN, VAL, MAX) ((VAL) <= (MIN) ? (MIN) : ((VAL) >= (MAX) ? (MAX) : (VAL)))
#endif

#ifndef pclamp /* positive clamp */
#define pclamp(VAL, MAX) ((VAL) > (MAX) ? (MAX) : (VAL))
#endif

#ifndef oclamp /* overflow clamp */
#define oclamp(MIN, VAL, MAX) ((VAL) < (MIN) ? (MAX) : ((VAL) > (MAX) ? (MIN) : (VAL)))
#endif

#ifndef range_out
#define range_out(LB, VAL, HB) ((VAL) < (LB) || (VAL) > (HB))
#endif

#ifndef range_in
#define range_in(LB, VAL, HB) ((VAL) > (LB) && (VAL) < (HB))
#endif


/* Dinamic arrays implementation (WIP)
 * ----------------------------------------------------------- */

/* typedef struct VoidArray { */
/* 	char* items; */
/* 	char item_size; */
/* 	n64 count; */
/* 	n64 capacity; */
/* } VoidArray; */

#define arr_new(arr, cap) \
	do { \
		if((arr)->capacity < (cap)) { \
			(arr)->items = drealloc((arr)->items, (cap) * sizeof(*(arr)->items)); \
			if((arr)->items == NULL) { \
				printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
						__FILE__, __LINE__, strerror(errno)); \
				exit(failure); \
			} \
			(arr)->capacity = (cap); \
		} \
		(arr)->count = 0; \
	} while(0)

#define arr_from(arr, src, src_len) do { \
	if((arr)->capacity < (src_len)) { \
		(arr)->items = drealloc((arr)->items, (src_len) * sizeof(*(arr)->items)); \
		if((arr)->items == NULL) { \
			printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
					__FILE__, __LINE__, strerror(errno)); \
			exit(failure); \
		} \
		(arr)->capacity = (src_len); \
	} \
	(void)memcpy((arr)->items, (src)->items, (src_len)); \
	(arr)->count = (src_len); \
} while(0)

#define arr_at_end(arr, off) ((arr)->items[(arr)->count - 1 - (off)])

#define arr_append(arr, item) do { \
	if((arr)->capacity < (arr)->count + 1) { \
		(arr)->items = drealloc((arr)->items, ((arr)->count + 1) * 2 * sizeof(*(arr)->items)); \
		if((arr)->items == NULL) { \
			printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
					__FILE__, __LINE__, strerror(errno)); \
			exit(failure); \
		} \
		(arr)->capacity = ((arr)->count + 1) * 2; \
	} \
	(arr)->items[(arr)->count] = (item); \
	(arr)->count++; \
} while(0)


/* Bit v1
 * ----------------------------------------------------------- */
#define CORE_BIT_VER 1

extern void printb(n8 byte);
extern void printw(n32 word);

extern void savebuff(FILE *file, char *buffer, n64 buff_len);


/* Buffer v1
 * ----------------------------------------------------------- */
#define CORE_BUF_VER 1

/* Return the index of the chr or -1 */
extern i64 buffer_find_chr(char chr, char *buffer, n64 len);

/* Return the index of the first char that is not a delimiter or -1 */
extern i64 buffer_skip_str(char *del, n64 del_len, char *buffer, n64 buff_len);

/* Copy from src to dest buffers until the delimiter is reached */
extern n64 buffer_copy_until_chr(char delimiter,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len);

/* Copy from src to dest buffers until the delimiter is reached */
extern n64 buffer_copy_until_str(char *delimiter, n64 del_len,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len);


/* Logger v3
 * ----------------------------------------------------------- */
#define CORE_LOG_VER 3

#ifndef CORE_LOG_MODULE
#define CORE_LOG_MODULE NULL
#endif

#ifndef CORE_LOG_STREAM
#define CORE_LOG_STREAM stdout
#endif

typedef enum {
	LL_INFO = 0, LL_DEBUG, LL_WARN, LL_ERROR, LL_COUNT
} LogLevel;

#define CORE_INFO   LL_INFO , CORE_LOG_MODULE, __FILE__, __LINE__
#define CORE_DEBUG  LL_DEBUG, CORE_LOG_MODULE, __FILE__, __LINE__
#define CORE_WARN   LL_WARN , CORE_LOG_MODULE, __FILE__, __LINE__
#define CORE_ERROR  LL_ERROR, CORE_LOG_MODULE, __FILE__, __LINE__

extern void core_log(LogLevel level, CString module,
		CString file, int line, CString format, ...);
extern void core_test(bool pass, CString expected,
		CString got, CString format, ...);


/* Memdeb v1
 * ----------------------------------------------------------- */
#define CORE_MEM_VER 1

#define dmalloc(SIZE)        malloc_(SIZE, __FILE__, __LINE__)
#define dfree(PTR)           free_(PTR, __FILE__, __LINE__)
#define dcalloc(NMEMB, SIZE) calloc_(NMEMB, SIZE, __FILE__, __LINE__)
#define drealloc(PTR, SIZE)  realloc_(PTR, SIZE, __FILE__, __LINE__)

extern void *malloc_(size_t size, char* file, int line);
extern void free_(void* ptr, char* file, int line);
extern void *calloc_(size_t nmemb, size_t size, char* file, int line);
extern void *realloc_(void* ptr, size_t size, char* file, int line);


/* Net v1
 * ----------------------------------------------------------- */
#define CORE_NET_VER 1

typedef union ip4_addr {
	struct in_addr addr;
	struct { n8 D, C, B, A; } part;
	n8 bytes_rev[4];
} in_addr;

extern struct in_addr addr_to_bytes(n8 A, n8 B, n8 C, n8 D);
extern error hostname_resolve(const char *hostname, struct in_addr *address);


/* Stuff v1
 * ----------------------------------------------------------- */
#define CORE_STF_VER 1

extern char* shift(int* argc, char*** argv);


/* Select Graphic Rendition (SGR)
 * ----------------------------------------------------------- */
#if SGR_ENABLE

#define CSI    "\033["
#define M      "m"
#define Z      ";"
#define RESET  "0"

/* Text styles */
#define BOLD       "1"
#define DIM        "2"
#define ITALIC     "3"
#define UNDERLINE  "4"
#define SLW_BLINK  "5"
#define FST_BLINK  "6"
#define SWAP       "7"
#define CONCEAL    "8"
#define CROSS      "9"

/* Text style disabler */
#define NO_BOLD       "21"
#define NO_DIM        "22"
#define NO_ITALIC     "23"
#define NO_UNDERLINE  "24"
#define NO_BLINK      "25"

#define NO_SWAP     "27"
#define NO_CONCEAL  "28"
#define NO_CROSS    "29"

/* Foreground colors 3-bit */
#define FG_BLACK         "30"
#define FG_RED           "31"
#define FG_GREEN         "32"
#define FG_YELLOW        "33"
#define FG_BLUE          "34"
#define FG_MAGENTA       "35"
#define FG_CYAN          "36"
#define FG_WHITE         "37"
#define FG_PAL(CODE)     "38;5;" #CODE
#define FG_RGB(R, G, B)  "38;2;" #R";"#G";"#B
#define FG_DEFAULT       "39"

/* Background colors 3-bit */
#define BG_BLACK         "40"
#define BG_RED           "41"
#define BG_GREEN         "42"
#define BG_YELLOW        "43"
#define BG_BLUE          "44"
#define BG_MAGENTA       "45"
#define BG_CYAN          "46"
#define BG_WHITE         "47"
#define BG_PAL(CODE)     "48;5;" #CODE
#define BG_RGB(R, G, B)  "48;2;" #R";"#G";"#B
#define BG_DEFAULT       "49"

/* Foreground bright colors 3-bit */
#define FG_BR_BLACK  "90"
#define FG_BR_RED    "91"
#define FG_BR_GREEN  "92"
#define FG_BR_YELLO  "93"
#define FG_BR_BLUE   "94"
#define FG_BR_MAGEN  "95"
#define FG_BR_CYAN   "96"
#define FG_BR_WHITE  "97"

/* Background colors 3-bit */
#define BG_BR_BLACK  "100"
#define BG_BR_RED    "101"
#define BG_BR_GREEN  "102"
#define BG_BR_YELLO  "103"
#define BG_BR_BLUE   "104"
#define BG_BR_MAGEN  "105"
#define BG_BR_CYAN   "106"
#define BG_BR_WHITE  "107"

#endif /* SGR_ENABLE */


#endif /* CORE_ */
