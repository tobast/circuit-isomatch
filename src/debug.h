/** Few debug features. These flags should all be turned off in release. */

#ifdef DEBUG
#define DEBUG_FIND_LOW
#endif

#ifdef DEBUG_ALL
#define DEBUG_EQUAL
#define DEBUG_FIND
#endif

/* Debug printing for sub-equality matches */
#ifdef DEBUG_EQUAL
#define EQ_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define EQ_DEBUG(...) do {} while(false)
#endif

/* Debug printing for subcircuit finding */
#ifdef DEBUG_FIND
#define FIND_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define FIND_DEBUG(...) do {} while(false)
#endif

#ifdef DEBUG_FIND_LOW
#define FIND_DEBUG_2(...) fprintf(stderr, __VA_ARGS__)
#else
#define FIND_DEBUG_2(...) do {} while(false)
#endif

