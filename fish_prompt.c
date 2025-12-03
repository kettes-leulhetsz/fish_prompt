#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#include <unistd.h>	// write

// #include <unistr.h>
// #include <unigbrk.h>

// libunistring {{{

typedef uint32_t ucs4_t;
size_t u8_strlen (const uint8_t *s);
uint8_t *u8_strchr (const uint8_t *s, ucs4_t uc);
int u8_uctomb_aux (uint8_t *s, ucs4_t uc, ptrdiff_t n);
int u8_mbtouc (ucs4_t *puc, const uint8_t *s, size_t n);
uint8_t *u8_strncpy (uint8_t *dest, const uint8_t *src, size_t n);
const uint8_t *u8_grapheme_next (const uint8_t *s, const uint8_t *end);
bool uc_is_grapheme_break (ucs4_t a, ucs4_t b);
int uc_graphemeclusterbreak_property (ucs4_t uc);

#define FALLTHROUGH

// }}}



enum size_t { SZ = 32768 };
char out[SZ];
size_t written = 0;


const char cyellow[] = "\e[1m\e[33m"; 	// set_color yellow --bold 
const char cred[] = "\e[1m\e[31m";	// set_color red --bold
const char cgreen[] = "\e[1m\e[32m";	// set_color green --bold
const char cwhite[] = "\e[1m\e[37m";	// set_color white --bold
const char cnormal[] = "\e[30m\e(B\e[m";	// set_color normal

void print(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	written += vsnprintf(out + written, SZ - written, fmt, args); 
	va_end(args);
	if (written >= SZ - 1) {
		fprintf(stderr, "Buffer too small! How did you do that?!");
		exit(127);
	}
}

void username() {
	const char nouser[] = "<no username>";
	char* user = getenv("USER");
	print("%s%s ", cnormal, user ? user : nouser); 
}

void status(const char *s) {
	int64_t status = strtoll(s, NULL, 10);
	if (status) print("%s%lld ", cred, status);
}

void cmd_duration(const char *s) {
	uint64_t i = strtoull(s, NULL, 10); // saturates

//	if (i < 100) return;

	int ms = i % 1000;		i /= 1000;
	int seconds = i % 60;		i /= 60;
	int minutes = i % 60;		i /= 60;
	int hours = i % 24;		i /= 24;
	uint64_t days = i;
	int full = 0;
	
	print("%s[", cyellow);

	if (days > 0) {
		full = 1;
		print("%llud ", days);
	}

	if (full || hours > 0) {
		full = 1;
		print("%02d:", hours);
	}
	
	if (full || minutes > 0) {
		print("%02d:", minutes);
	}
	
	print("%02d.%03d] ", seconds, ms);

}

void cwd(int uid) {
	enum size_t { MAX = 32768 };
	static char _cwd[MAX]; // PATH_MAX is broken.
	char *cwd = getcwd(_cwd, MAX);
	
	if (!cwd) return;

	char *home = getenv("HOME");
	size_t homelen = 0;
	char *prefix = getenv("PREFIX");
	size_t prefixlen = 0;

	if(home) homelen = strlen(home);
	if(prefix) prefixlen = strlen(prefix);
	
	
	print(uid ? cgreen : cred);
	if(home && homelen && !strncmp(cwd, home, homelen)) {
		print("~");
		cwd += homelen;
	} else if (prefix && prefixlen && !strncmp(cwd, prefix, prefixlen)) {
		print("$P");
		cwd += prefixlen;
	}


	const uint8_t *dir = (uint8_t*)cwd;
	const uint8_t *end = dir + u8_strlen(dir);

	const uint8_t *slash;
	while ((slash = u8_strchr(dir, '/'))) {
		print("/");
		while(*slash == '/') slash++;
		if(*slash) {
			const uint8_t *gr = u8_grapheme_next(slash, end);
			ptrdiff_t bytes = gr - slash;
			u8_strncpy((uint8_t*)out + written, slash, bytes); //print("%c", *slash);
			written += bytes;
			slash = gr;
		}
		dir = slash;
	}
	if (dir && *dir) print("%s", dir);
	
}

void prompt(int uid) {
	if (uid) {
		print("%s> %s", cwhite, cnormal);
	} else {
		print("%s # %s", cwhite, cnormal);
	}
}

void printtime() {
    time_t t = {0};
    time(&t);
    struct tm _tm = {0};
    localtime_r(&t, &_tm);
    print("%s%d:%d ", cnormal, _tm.tm_hour, _tm.tm_min);


}

int main(int argc, char *argv[]) {
//	username();
    printtime();
	if (argc > 1) status(argv[1]);
	if (argc > 2) cmd_duration(argv[2]);
	int uid = geteuid();
	cwd(uid);
	prompt(uid);
	
	write(1, out, written);
	return 0;
}














// copied from libunistring {{{

int
u8_mbtouc (ucs4_t *puc, const uint8_t *s, size_t n)
{
  uint8_t c = *s;

  if (c < 0x80)
    {
      *puc = c;
      return 1;
    }
  else if (c >= 0xc2)
    {
      if (c < 0xe0)
        {
          if (n >= 2)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  *puc = ((unsigned int) (c & 0x1f) << 6)
                         | (unsigned int) (s[1] ^ 0x80);
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              return 1;
            }
        }
      else if (c < 0xf0)
        {
          if (n >= 3)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  if ((s[2] ^ 0x80) < 0x40)
                    {
                      if ((c >= 0xe1 || s[1] >= 0xa0)
                          && (c != 0xed || s[1] < 0xa0))
                        {
                          *puc = ((unsigned int) (c & 0x0f) << 12)
                                 | ((unsigned int) (s[1] ^ 0x80) << 6)
                                 | (unsigned int) (s[2] ^ 0x80);
                          return 3;
                        }
                      /* invalid multibyte character */
                      *puc = 0xfffd;
                      return 3;
                    }
                  /* invalid multibyte character */
                  *puc = 0xfffd;
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              if (n == 1 || (s[1] ^ 0x80) >= 0x40)
                return 1;
              else
                return 2;
            }
        }
      else if (c < 0xf8)
        {
          if (n >= 4)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  if ((s[2] ^ 0x80) < 0x40)
                    {
                      if ((s[3] ^ 0x80) < 0x40)
                        {
                          if ((c >= 0xf1 || s[1] >= 0x90)
                              && (c < 0xf4 || (c == 0xf4 && s[1] < 0x90)))
                            {
                              *puc = ((unsigned int) (c & 0x07) << 18)
                                     | ((unsigned int) (s[1] ^ 0x80) << 12)
                                     | ((unsigned int) (s[2] ^ 0x80) << 6)
                                     | (unsigned int) (s[3] ^ 0x80);
                              return 4;
                            }
                          /* invalid multibyte character */
                          *puc = 0xfffd;
                          return 4;
                        }
                      /* invalid multibyte character */
                      *puc = 0xfffd;
                      return 3;
                    }
                  /* invalid multibyte character */
                  *puc = 0xfffd;
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              if (n == 1 || (s[1] ^ 0x80) >= 0x40)
                return 1;
              else if (n == 2 || (s[2] ^ 0x80) >= 0x40)
                return 2;
              else
                return 3;
            }
        }
    }
  /* invalid multibyte character */
  *puc = 0xfffd;
  return 1;
}


uint8_t *
u8_strncpy (uint8_t *dest, const uint8_t *src, size_t n)
{
  return (uint8_t *) strncpy ((char *) dest, (const char *) src, n);
}

size_t
u8_strlen (const uint8_t *s)
{
  return strlen ((const char *) s);
}



uint8_t *
u8_strchr (const uint8_t *s, ucs4_t uc)
{
  uint8_t c[6];

  if (uc < 0x80)
    {
      uint8_t c0 = uc;

      if (false)
        {
          /* Unoptimized code.  */
          for (;;)
            {
              uint8_t s0 = *s;
              if (s0 == c0)
                return (uint8_t *) s;
              s++;
              if (s0 == 0)
                break;
            }
        }
      else
        {
          /* Optimized code.
             strchr() is often so well optimized, that it's worth the
             added function call.  */
          return (uint8_t *) strchr ((const char *) s, c0);
        }
    }
  else
      /* Loops equivalent to strstr, optimized for a specific length (2, 3, 4)
         of the needle.  We use an algorithm similar to Boyer-Moore which
         is documented in lib/unistr/u8-chr.c.  There is additional
         complication because we need to check after every byte for
         a NUL byte, but the idea is the same. */
    switch (u8_uctomb_aux (c, uc, 6))
      {
      case 2:
        if (*s == 0 || s[1] == 0)
          break;
        {
          uint8_t c0 = c[0];
          uint8_t c1 = c[1];
          /* Search for { c0, c1 }.  */
          uint8_t s1 = s[1];

          for (;;)
            {
              /* Here s[0] != 0, s[1] != 0.
                 Test whether s[0..1] == { c0, c1 }.  */
              if (s1 == c1)
                {
                  if (*s == c0)
                    return (uint8_t *) s;
                  else
                    /* Skip the search at s + 1, because s[1] = c1 < c0.  */
                    goto case2_skip2;
                }
              else
                {
                  if (s1 == c0)
                    goto case2_skip1;
                  else
                    /* Skip the search at s + 1, because s[1] != c0.  */
                    goto case2_skip2;
                }
             case2_skip2:
              s++;
              s1 = s[1];
              if (s[1] == 0)
                break;
             case2_skip1:
              s++;
              s1 = s[1];
              if (s[1] == 0)
                break;
            }
        }
        break;

      case 3:
        if (*s == 0 || s[1] == 0 || s[2] == 0)
          break;
        {
          uint8_t c0 = c[0];
          uint8_t c1 = c[1];
          uint8_t c2 = c[2];
          /* Search for { c0, c1, c2 }.  */
          uint8_t s2 = s[2];

          for (;;)
            {
              /* Here s[0] != 0, s[1] != 0, s[2] != 0.
                 Test whether s[0..2] == { c0, c1, c2 }.  */
              if (s2 == c2)
                {
                  if (s[1] == c1 && *s == c0)
                    return (uint8_t *) s;
                  else
                    /* If c2 != c1:
                         Skip the search at s + 1, because s[2] == c2 != c1.
                       Skip the search at s + 2, because s[2] == c2 < c0.  */
                    if (c2 == c1)
                      goto case3_skip1;
                    else
                      goto case3_skip3;
                }
              else
                {
                  if (s2 == c1)
                    goto case3_skip1;
                  else if (s2 == c0)
                    /* Skip the search at s + 1, because s[2] != c1.  */
                    goto case3_skip2;
                  else
                    /* Skip the search at s + 1, because s[2] != c1.
                       Skip the search at s + 2, because s[2] != c0.  */
                    goto case3_skip3;
                }
             case3_skip3:
              s++;
              s2 = s[2];
              if (s[2] == 0)
                break;
             case3_skip2:
              s++;
              s2 = s[2];
              if (s[2] == 0)
                break;
             case3_skip1:
              s++;
              s2 = s[2];
              if (s[2] == 0)
                break;
            }
        }
        break;

      case 4:
        if (*s == 0 || s[1] == 0 || s[2] == 0 || s[3] == 0)
          break;
        {
          uint8_t c0 = c[0];
          uint8_t c1 = c[1];
          uint8_t c2 = c[2];
          uint8_t c3 = c[3];
          /* Search for { c0, c1, c2, c3 }.  */
          uint8_t s3 = s[3];

          for (;;)
            {
              /* Here s[0] != 0, s[1] != 0, s[2] != 0, s[3] != 0.
                 Test whether s[0..3] == { c0, c1, c2, c3 }.  */
              if (s3 == c3)
                {
                  if (s[2] == c2 && s[1] == c1 && *s == c0)
                    return (uint8_t *) s;
                  else
                    /* If c3 != c2:
                         Skip the search at s + 1, because s[3] == c3 != c2.
                       If c3 != c1:
                         Skip the search at s + 2, because s[3] == c3 != c1.
                       Skip the search at s + 3, because s[3] == c3 < c0.  */
                    if (c3 == c2)
                      goto case4_skip1;
                    else if (c3 == c1)
                      goto case4_skip2;
                    else
                      goto case4_skip4;
                }
              else
                {
                  if (s3 == c2)
                    goto case4_skip1;
                  else if (s3 == c1)
                    /* Skip the search at s + 1, because s[3] != c2.  */
                    goto case4_skip2;
                  else if (s3 == c0)
                    /* Skip the search at s + 1, because s[3] != c2.
                       Skip the search at s + 2, because s[3] != c1.  */
                    goto case4_skip3;
                  else
                    /* Skip the search at s + 1, because s[3] != c2.
                       Skip the search at s + 2, because s[3] != c1.
                       Skip the search at s + 3, because s[3] != c0.  */
                    goto case4_skip4;
                }
             case4_skip4:
              s++;
              s3 = s[3];
              if (s[3] == 0)
                break;
             case4_skip3:
              s++;
              s3 = s[3];
              if (s[3] == 0)
                break;
             case4_skip2:
              s++;
              s3 = s[3];
              if (s[3] == 0)
                break;
             case4_skip1:
              s++;
              s3 = s[3];
              if (s[3] == 0)
                break;
            }
        }
        break;
      }

  return NULL;
}


int
u8_uctomb_aux (uint8_t *s, ucs4_t uc, ptrdiff_t n)
{
  int count;

  if (uc < 0x80)
    /* The case n >= 1 is already handled by the caller.  */
    return -2;
  else if (uc < 0x800)
    count = 2;
  else if (uc < 0x10000)
    {
      if (uc < 0xd800 || uc >= 0xe000)
        count = 3;
      else
        return -1;
    }
  else if (uc < 0x110000)
    count = 4;
  else
    return -1;

  if (n < count)
    return -2;

  switch (count) /* note: code falls through cases! */
    {
    case 4: s[3] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x10000;
      FALLTHROUGH;
    case 3: s[2] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x800;
      FALLTHROUGH;
    case 2: s[1] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0xc0;
  /*case 1:*/ s[0] = uc;
    }
  return count;
}


const uint8_t *
u8_grapheme_next (const uint8_t *s, const uint8_t *end)
{
  ucs4_t prev;
  int mblen;

  if (s == end)
    return NULL;

  for (s += u8_mbtouc (&prev, s, end - s); s != end; s += mblen)
    {
      ucs4_t next;

      mblen = u8_mbtouc (&next, s, end - s);
      if (uc_is_grapheme_break (prev, next))
        break;

      prev = next;
    }

  return s;
}

enum
{
  GBP_OTHER        = 0,
  GBP_CR           = 1,
  GBP_LF           = 2,
  GBP_CONTROL      = 3,
  GBP_EXTEND       = 4,
  GBP_PREPEND      = 5,
  GBP_SPACINGMARK  = 6,
  GBP_L            = 7,
  GBP_V            = 8,
  GBP_T            = 9,
  GBP_LV           = 10,
  GBP_LVT          = 11,
  GBP_RI           = 12,
  GBP_ZWJ          = 13,
  GBP_EB           = 14, /* obsolete */
  GBP_EM           = 15, /* obsolete */
  GBP_GAZ          = 16, /* obsolete */
  GBP_EBG          = 17  /* obsolete */
};

/* Evaluates to true if there is an extended grapheme cluster break between
   code points with GBP_* values A and B, false if there is not.  The comments
   are the grapheme cluster boundary rules from in UAX #29. */
#define UC_IS_GRAPHEME_BREAK(A, B)                                      \
  (/* GB1 and GB2 are covered--just use a GBP_CONTROL character, such   \
      as 0, for sot and eot. */                                         \
                                                                        \
   /* GB3 */                                                            \
   (A) == GBP_CR && (B) == GBP_LF ? false :                             \
                                                                        \
   /* GB4 */                                                            \
   (A) == GBP_CONTROL || (A) == GBP_CR || (A) == GBP_LF ? true :        \
                                                                        \
   /* GB5 */                                                            \
   (B) == GBP_CONTROL || (B) == GBP_CR || (B) == GBP_LF ? true :        \
                                                                        \
   /* GB6 */                                                            \
   (A) == GBP_L && ((B) == GBP_L || (B) == GBP_V                        \
                    || (B) == GBP_LV || (B) == GBP_LVT) ? false :       \
                                                                        \
   /* GB7 */                                                            \
   ((A) == GBP_LV || (A) == GBP_V)                                      \
   && ((B) == GBP_V || (B) == GBP_T) ? false :                          \
                                                                        \
   /* GB8 */                                                            \
   ((A) == GBP_LVT || (A) == GBP_T) && (B) == GBP_T ? false :           \
                                                                        \
   /* GB9 */                                                            \
   (B) == GBP_EXTEND || (B) == GBP_ZWJ ? false :                        \
                                                                        \
   /* GB9a */                                                           \
   (B) == GBP_SPACINGMARK ? false :                                     \
                                                                        \
   /* GB9b */                                                           \
   (A) == GBP_PREPEND ? false :                                         \
                                                                        \
   /* GB10 -- incomplete */                                             \
   ((A) == GBP_EB || (A) == GBP_EBG) && (B) == GBP_EM ? false :         \
                                                                        \
   /* GB11 */                                                           \
   (A) == GBP_ZWJ && ((B) == GBP_GAZ || (B) == GBP_EBG) ? false         \
                                                                        \
   /* GB999 */                                                          \
   : true)

#define UC_GRAPHEME_BREAKS_FOR(A)                                       \
  (  (UC_IS_GRAPHEME_BREAK(A, GBP_OTHER)       << GBP_OTHER)            \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_CR)          << GBP_CR)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_LF)          << GBP_LF)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_CONTROL)     << GBP_CONTROL)          \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_EXTEND)      << GBP_EXTEND)           \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_PREPEND)     << GBP_PREPEND)          \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_SPACINGMARK) << GBP_SPACINGMARK)      \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_L)           << GBP_L)                \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_V)           << GBP_V)                \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_T)           << GBP_T)                \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_LV)          << GBP_LV)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_LVT)         << GBP_LVT)              \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_RI)          << GBP_RI)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_ZWJ)         << GBP_ZWJ)              \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_EB)          << GBP_EB)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_EM)          << GBP_EM)               \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_GAZ)         << GBP_GAZ)              \
   | (UC_IS_GRAPHEME_BREAK(A, GBP_EBG)         << GBP_EBG))

static const unsigned long int gb_table[18] =
  {
    UC_GRAPHEME_BREAKS_FOR(0),  /* GBP_OTHER */
    UC_GRAPHEME_BREAKS_FOR(1),  /* GBP_CR */
    UC_GRAPHEME_BREAKS_FOR(2),  /* GBP_LF */
    UC_GRAPHEME_BREAKS_FOR(3),  /* GBP_CONTROL */
    UC_GRAPHEME_BREAKS_FOR(4),  /* GBP_EXTEND */
    UC_GRAPHEME_BREAKS_FOR(5),  /* GBP_PREPEND */
    UC_GRAPHEME_BREAKS_FOR(6),  /* GBP_SPACINGMARK */
    UC_GRAPHEME_BREAKS_FOR(7),  /* GBP_L */
    UC_GRAPHEME_BREAKS_FOR(8),  /* GBP_V */
    UC_GRAPHEME_BREAKS_FOR(9),  /* GBP_T */
    UC_GRAPHEME_BREAKS_FOR(10), /* GBP_LV */
    UC_GRAPHEME_BREAKS_FOR(11), /* GBP_LVT */
    UC_GRAPHEME_BREAKS_FOR(12), /* GBP_RI */
    UC_GRAPHEME_BREAKS_FOR(13), /* GBP_ZWJ */
    UC_GRAPHEME_BREAKS_FOR(14), /* GBP_EB */
    UC_GRAPHEME_BREAKS_FOR(15), /* GBP_EM */
    UC_GRAPHEME_BREAKS_FOR(16), /* GBP_GAZ */
    UC_GRAPHEME_BREAKS_FOR(17), /* GBP_EBG */
  };

bool
uc_is_grapheme_break (ucs4_t a, ucs4_t b)
{
  int a_gcp, b_gcp;

  if ((a | b) < 0x300)
    {
      /* GB3 is the only relevant rule for this case. */
      return a != '\r' || b != '\n';
    }

  a_gcp = uc_graphemeclusterbreak_property (a);
  b_gcp = uc_graphemeclusterbreak_property (b);
  return (gb_table[a_gcp] >> b_gcp) & 1;
}

#include "gbrkprop.h"

int
uc_graphemeclusterbreak_property (ucs4_t uc)
{
  unsigned int index1 = uc >> gbrkprop_header_0;
  if (index1 < gbrkprop_header_1)
    {
      int lookup1 = unigbrkprop.level1[index1];
      if (lookup1 >= 0)
        {
          unsigned int index2 = (uc >> gbrkprop_header_2) & gbrkprop_header_3;
          int lookup2 = unigbrkprop.level2[lookup1 + index2];
          if (lookup2 >= 0)
            {
              unsigned int index3 = uc & gbrkprop_header_4;
              return unigbrkprop.level3[lookup2 + index3];
            }
        }
    }
  return GBP_OTHER;
}


// }}}



