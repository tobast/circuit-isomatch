/** Few debug features. These flags should all be turned off in release. */

#ifdef DEBUG_ALL
//#define DEBUG_EQUAL
#endif

/* Debug printing for sub-equality matches */
#ifdef DEBUG_EQUAL
#define EQ_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define EQ_DEBUG(...) do {} while(false)
#endif

