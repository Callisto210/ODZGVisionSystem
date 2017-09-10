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
	int (*cmp)(int, int);
	int level;
	char *str;
};

struct fabulous_debug_ptr_t {
	void *code;
	int level;
	char *str;
};

#define FABULOUS_DEBUG_INT(F, R, U, ...) do {				\
	extern struct fabulous_debug_int_t F##_debug_t;			\
	struct fabulous_debug_int_t *s;					\
	const char *u = (U);						\
	s = &F##_debug_t;						\
	for (; s->cmp; s++)						\
		if (s->cmp((R), s->code))				\
			if (s->level & FABULOUS_CURRENT) {		\
				if (u == NULL)				\
					printf("[%s @ %d] %s: %s\n",	\
					    __FILE__, __LINE__,		\
					     #F, s->str);		\
				else {					\
					char tmp[100];			\
					snprintf(tmp, 100, u, __VA_ARGS__);	\
					printf("[%s @ %d] %s (%s): %s\n",	\
					    __FILE__, __LINE__,		\
					    #F, tmp, s->str);		\
				}					\
			}						\
} while(0)

#define FABULOUS_DEBUG_PTR(F, R, U, ...) do {				\
	extern struct fabulous_debug_ptr_t F##_debug_t;			\
	struct fabulous_debug_ptr_t *s;					\
	s = &F##_debug_t;						\
	const char *u = (U);						\
	for (; s->str; s++)						\
		if (s->code == (R))					\
			if (s->level & FABULOUS_CURRENT) {		\
				if (u == NULL)				\
					printf("[%s @ %d] %s: %s\n",	\
					    __FILE__, __LINE__,		\
					     "#F", s->str);		\
				else {					\
					char tmp[100];			\
					snprintf(tmp, 100, u, __VA_ARGS__);	\
					printf("[%s @ %d] %s (%s): %s\n",	\
					    __FILE__, __LINE__,		\
					    "#F", u, s->str);		\
				}					\
			}						\
} while(0)

#define NSDI		FABULOUS_DEBUG_INT
#define NSDP		FABULOUS_DEBUG_PTR
