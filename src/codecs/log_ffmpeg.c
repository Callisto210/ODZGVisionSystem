#include <stdlib.h>
#include "log_common.h"

int eq(int x, int y) {
	return (x == y) ? 1 : 0;
}

int lt(int x, int y) {
	return (x < y) ? 1 : 0;
}

struct fabulous_debug_int_t avcodec_open2_debug_t[] = {
	{0, eq, FABULOUS_INFO, "OK"},
	{0, lt, FABULOUS_ERROR, "Codec initialize failed"},
	{0, NULL, 0, NULL}
};
