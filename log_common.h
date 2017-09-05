#define FABULOUS_VERBOSE	0
#define FABULOUS_INFO		1
#define FABULOUS_WARN		2
#define FABULOUS_ERROR		3

#define FABULOUS_DEBUG_INT(function, ...) do {	\
	int ret;				\
	ret = function(__VA_ARGS__);		\
} while(0)

#define FABULOUS_DEBUG_PTR(function, ...) do {	\
	void *ret;				\
	ret = function(__VA_ARGS__);		\
} while(0)

#define NSDI		FABULOUS_DEBUG_INT
#define NSDP		FABULOUS_DEBUG_PTR
