#define FABULOUS_VERBOSE	0x8
#define FABULOUS_INFO		0x4
#define FABULOUS_WARN		0x2
#define FABULOUS_ERROR		0x1
#define FABULOUS_NOTHING	0x0

#define LEVEL_VERBOSE		FABULOUS_VERBOSE
#define LEVEL_INFO		LEVEL_VERBOSE | FABULOUS_INFO
#define LEVEL_WARN		LEVEL_INFO | FABULOUS_WARN
#define LEVEL_ERROR		LEVEL_WARN | FABULOUS_ERROR

struct fabulous_debug_int_t {
	int code;
	int level;
	char *str;
};

struct fabulous_debug_ptr_t {
	void *code;
	int level;
	char *str;
};

#define FABULOUS_DEBUG_INT(function, retval) do {			\
	extern struct fabulous_debug_int_t * function_debug_t;		\
	struct fabulous_debug_int_t *s;					\
	s = function_debug_t;						\
	for (; s->str; s++)						\
		if (s->code == retval)					\
			if (s->level & FABULOUS_CURRENT)		\
				printf("%s (%s @ %d): %s", "function",	\
				    __FILE__, __LINE__, s->str);	\
} while(0)

#define FABULOUS_DEBUG_PTR(function, retval) do {			\
	extern struct fabulous_debug_ptr_t *function_debug_t;		\
	struct fabulous_debug_ptr_t *s;					\
	s = function_debug_t;						\
	for (; s->str; s++)						\
		if (s->code == retval)					\
			if (s->level & FABULOUS_CURRENT)		\
				printf("%s (%s @ %d): %s", "function",	\
				    __FILE__, __LINE__, s->str);	\
} while(0)

#define NSDI		FABULOUS_DEBUG_INT
#define NSDP		FABULOUS_DEBUG_PTR
