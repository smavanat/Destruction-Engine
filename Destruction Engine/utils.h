#include <stdint.h>;
// A simple implmenentation of a multimap on entities, where instead of storing pointers, we
// simply store indecies into component arrays. This could break when objects are deleted from the 
// array, so need to be careful.
typedef struct {
	uint32_t* keys;
	uint32_t** values;
} multiMap;

typedef struct {

} memoryArena;
