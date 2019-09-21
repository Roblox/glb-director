#ifndef __GLB_CONFIG_TYPES_H__

#define __GLB_CONFIG_TYPES_H__

typedef enum {FALSE, TRUE} boolean;

typedef struct {
	uint32_t file_fmt_ver;
	uint32_t num_tables;
	uint32_t table_entries;
	uint32_t max_num_backends;
	uint32_t max_num_binds;
} bin_file_header;

typedef struct {
	uint32_t inet_family;

	union {
		char v6[16];
		struct {
			uint32_t v4;
			char reserved[12];
		};
	} ip;

	uint16_t state;
	uint16_t health;
} backend_entry;

typedef struct {
	uint32_t inet_family;

	union {
		char v6[16];
		struct {
			uint32_t v4;
			char _[12];
		};
	} ip;

	uint16_t ip_bits;

	uint16_t port_start;
	uint16_t port_end;
	uint8_t ipproto;
	uint8_t reserved;
} bind_entry;

struct table_entry_ {
    uint32_t num_idxs;    
    uint32_t idxs[GLB_MAX_GUE_HOPS];
} __attribute__((__packed__));
typedef struct table_entry_ table_entry;


#endif /* __GLB_CONFIG_TYPES_H__ */
