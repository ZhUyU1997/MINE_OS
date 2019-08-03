#ifndef __DTREE_H__
#define __DTREE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stddef.h>
#include <string.h>
#include <json.h>

struct dtnode_t {
	const char * name;
	physical_addr_t addr;
	struct json_value_t * value;
};

const char * dt_read_name(struct dtnode_t * n);
int dt_read_id(struct dtnode_t * n);
physical_addr_t dt_read_address(struct dtnode_t * n);
int dt_read_bool(struct dtnode_t * n, const char * name, int def);
int dt_read_int(struct dtnode_t * n, const char * name, int def);
long long dt_read_long(struct dtnode_t * n, const char * name, long long def);
double dt_read_double(struct dtnode_t * n, const char * name, double def);
char * dt_read_string(struct dtnode_t * n, const char * name, char * def);
u8_t dt_read_u8(struct dtnode_t * n, const char * name, u8_t def);
u16_t dt_read_u16(struct dtnode_t * n, const char * name, u16_t def);
u32_t dt_read_u32(struct dtnode_t * n, const char * name, u32_t def);
u64_t dt_read_u64(struct dtnode_t * n, const char * name, u64_t def);
struct dtnode_t * dt_read_object(struct dtnode_t * n, const char * name, struct dtnode_t * o);
int dt_read_array_length(struct dtnode_t * n, const char * name);
int dt_read_array_bool(struct dtnode_t * n, const char * name, int idx, int def);
int dt_read_array_int(struct dtnode_t * n, const char * name, int idx, int def);
long long dt_read_array_long(struct dtnode_t * n, const char * name, int idx, long long def);
double dt_read_array_double(struct dtnode_t * n, const char * name, int idx, double def);
char * dt_read_array_string(struct dtnode_t * n, const char * name, int idx, char * def);
u8_t dt_read_array_u8(struct dtnode_t * n, const char * name, int idx, u8_t def);
u16_t dt_read_array_u16(struct dtnode_t * n, const char * name, int idx, u16_t def);
u32_t dt_read_array_u32(struct dtnode_t * n, const char * name, int idx, u32_t def);
u64_t dt_read_array_u64(struct dtnode_t * n, const char * name, int idx, u64_t def);
struct dtnode_t * dt_read_array_object(struct dtnode_t * n, const char * name, int idx, struct dtnode_t * o);

#ifdef __cplusplus
}
#endif

#endif /* __DTREE_H__ */
