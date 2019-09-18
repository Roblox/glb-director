#ifndef __CONFIG_TYPES_H__

#define __CONFIG_TYPES_H__

#define FILE_FORMAT_VERSION 3
#define MAX_TABLE_ENTRIES 0x10000
#define MAX_NUM_BACKENDS 0x100
#define MAX_NUM_BACKEND_IDXS 0x10
#define MAX_NUM_BINDS 0x100

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
    uint32_t idxs[MAX_NUM_BACKEND_IDXS];
} __attribute__((__packed__));
typedef struct table_entry_ table_entry;


#endif
