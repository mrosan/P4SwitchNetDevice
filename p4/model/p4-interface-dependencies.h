#ifndef P4_INTERFACE_DEP
#define P4_INTERFACE_DEP

// This header contains copy-pasted declarations and macros mostly from dpdk files,
// leaving the most-used parts in P4-interface.h (which is thus much more readable).

/* ############################################################################# */
/* ########################## copied from dataplane.h ########################## */

#define LOOKUP_EXACT   0
#define LOOKUP_LPM     1
#define LOOKUP_TERNARY 2

struct type_field_list {
    uint8_t fields_quantity;
    uint8_t** field_offsets;
    uint8_t* field_widths;
};

typedef struct vector_s {
    int size;
    int capacity;
    int data_size;
    void (*value_init)(void *);
    void **data;
    int socketid;
} vector_t;

typedef struct counter_s {
    char* name;
    uint8_t type;
    uint8_t min_width;
    int size;
    uint8_t saturating;
    vector_t *values;
    int socketid;
} counter_t;

typedef struct p4_register_s {
    char* name;
    uint8_t width;
    int size;
    //lock *locks;
    void* locks;
    uint8_t **values;
} p4_register_t;

/* ############################################################################## */
/* ########################## copied from handlers.h&c ########################## */
#define P4_MAX_NUMBER_OF_ACTION_PARAMETERS 10
#define P4_MAX_NUMBER_OF_FIELD_MATCHES 10

struct p4_ctrl_msg {
	uint8_t type;
	uint32_t xid;
	char* table_name;
	uint8_t action_type;
	char* action_name;
	int num_action_params;
	struct p4_action_parameter* action_params[P4_MAX_NUMBER_OF_ACTION_PARAMETERS];
	int num_field_matches;
	struct p4_field_match_header* field_matches[P4_MAX_NUMBER_OF_FIELD_MATCHES];
};

typedef void (*p4_msg_callback)(struct p4_ctrl_msg*);

/* ############################################################################## */
/* ########################## copied from messages.h ############################ */
#define P4_VERSION 1
#define true 1
#define false 0

struct p4_header {
	uint8_t version;	/* version */
	uint8_t type;		/* P4T constants */
	uint16_t length;	/* Length including this header */
	uint32_t xid;		/* Transaction ID */
};

enum p4_type {
	/* Immutable messages */
	P4T_HELLO = 0,
	P4T_ERROR = 1,
	P4T_ECHO_REQUEST = 2,
	P4T_ECHO_REPLY = 3,
	P4T_SUCCESS = 4,

	/* Switch informations */	
	P4T_GET_TABLE_DEFINITIONS = 100,
	P4T_GET_TABLE_COUNTERS = 101,
	P4T_GET_TABLE_ENTRIES = 102,

	/* Controller commands */
	P4T_SET_DEFAULT_ACTION = 103,
	P4T_ADD_TABLE_ENTRY = 104, /*with and without selector (action, action_profile)*/
	P4T_MODIFY_TABLE_ENTRY = 105,
	P4T_REMOVE_TABLE_ENTRY = 106,
	P4T_DROP_TABLE_ENTRIES = 107,
	P4T_ADD_AP_MEMBER = 108,
	P4T_REMOVE_AP_MEMBER = 109,

	/* Digest passed */
	P4T_DIGEST = 110
};

struct p4_hello {
	struct p4_header header;
	/* Hello element list */
	struct p4_hello_elem_header *elements; /* List of elements - 0 or more */
};

/* Hello elements types.
*/
enum p4_hello_elem_type {
	OFPHET_VERSIONBITMAP = 1 /* Bitmap of version supported. */
};

/* Common header for all Hello Elements */
struct p4_hello_elem_header {
	uint16_t type; /* One of OFPHET_*. */
	uint16_t length; /* Length in bytes of the element,
including this header, excluding padding. */
};

/* Version bitmap Hello Element */
struct p4_hello_elem_versionbitmap {
	uint16_t type; /* OFPHET_VERSIONBITMAP. */
	uint16_t length; /* Length in bytes of this element,
including this header, excluding padding. */
	/* Followed by:
		* - Exactly (length - 4) bytes containing the bitmaps, then
		* - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
		* bytes of all-zero bytes */
	uint32_t* bitmaps; /* List of bitmaps - supported versions */
};/* Version bitmap Hello Element */

struct p4_error {
	struct p4_header header;
	uint16_t error_code;
};

struct p4_success {
	struct p4_header header;
	uint32_t id;
};

#define P4_MAX_TABLE_NAME_LEN 128
#define P4_MAX_FIELD_NAME_LEN 128
#define P4_MAX_ACTION_NAME_LEN 128

enum p4_field_match_type {
	P4_FMT_EXACT = 0,
	P4_FMT_TERNARY = 1,
	P4_FMT_LPM = 2,
	P4_FMT_RANGE = 3,
	P4_FMT_VALID = 4
};

struct p4_field_match_desc {
	char name[P4_MAX_FIELD_NAME_LEN];
	uint8_t type;
};

enum p4_support_timeout {
	P4_STO_FALSE = 0,
	P4_STO_TRUE = 1
};

enum p4_action_type {
	P4_AT_ACTION = 0,
	P4_AT_ACTION_PROFILE = 1
};

struct p4_table_action_desc {
	uint8_t type;
	char name[P4_MAX_ACTION_NAME_LEN];
};

struct p4_table_definition {
	struct p4_header header;
	char name[P4_MAX_TABLE_NAME_LEN];
	uint8_t reads;
	uint16_t min_size;
	uint16_t max_size;
	uint16_t size;
	uint8_t support_timeout;
	uint8_t read_size; /* number of rules in reads */
	uint8_t action_size; /* number of actions */
	/* struct p4_field_match_desc[read_size]; */
	/* struct p4_table_action_desc[action_size]; */
};

#define MAX_ACTION_PARAMETER_NAME_LENGTH 128
#define MAX_ACTION_PARAMETER_BITMAP_LENGTH 128
struct p4_action_parameter {
	char name[MAX_ACTION_PARAMETER_NAME_LENGTH];
	uint32_t length;
	char bitmap[MAX_ACTION_PARAMETER_BITMAP_LENGTH];
};

struct p4_action {
        struct p4_table_action_desc description; /* Action name of action profile name */
        uint8_t param_size;
	/* struct p4_action_parameter params[param_size]; */
};

struct p4_set_default_action {
	struct p4_header header;
	char table_name[P4_MAX_TABLE_NAME_LEN];
	struct p4_action action; /* Action or action profile instance */
};

struct p4_field_match_header {
	char name[P4_MAX_FIELD_NAME_LEN];
	uint8_t type; /* p4_field_match_type */
};

#define MAX_FIELD_LENGTH 256

struct p4_field_match_lpm {
	struct p4_field_match_header header;
	uint16_t prefix_length;
	char bitmap[MAX_FIELD_LENGTH];
};

struct p4_field_match_exact {
	struct p4_field_match_header header;
        uint16_t length;
	char bitmap[MAX_FIELD_LENGTH];
};

struct p4_field_match_range {
	struct p4_field_match_header header;
        uint16_t length;
	uint8_t is_signed; /* boolean variable */
        char min_bitmap[MAX_FIELD_LENGTH];
	char max_bitmap[MAX_FIELD_LENGTH];
};

struct p4_field_match_ternary {
	struct p4_field_match_header header;
        uint16_t length;
	uint8_t priority;
        char bitmap[MAX_FIELD_LENGTH];
	char mask[MAX_FIELD_LENGTH];
};

struct p4_field_match_valid {
	struct p4_field_match_header header;
        uint16_t length;
	uint8_t is_valid; /* could be true or false - boolean variable */
        char bitmap[MAX_FIELD_LENGTH];
};

struct p4_add_table_entry {
	struct p4_header header;
	char table_name[P4_MAX_TABLE_NAME_LEN];
	uint8_t read_size;
	/* struct p4_field_match matches[read_size]; */
	/* struct p4_action; */
};

#define P4_MAX_FIELD_LIST_NAME_LEN 128
#define P4_MAX_FIELD_NAME_LENGTH 128
#define P4_MAX_FIELD_VALUE_LENGTH 32

struct p4_digest_field {
        char name[P4_MAX_FIELD_NAME_LENGTH];
        uint32_t length;
        char value[P4_MAX_FIELD_VALUE_LENGTH];	
};

struct p4_digest {
	struct p4_header header;
	char field_list_name[P4_MAX_FIELD_LIST_NAME_LEN];
	uint8_t list_size;
	/* struct p4_digest_field field_list[list_size]; */
};

/*
struct p4_header *create_p4_header(char* buffer, uint16_t offset, uint16_t maxlength);
struct p4_header *unpack_p4_header(char* buffer, uint16_t offset);
void check_p4_header( struct p4_header* a, struct p4_header* b);
struct p4_header* netconv_p4_header(struct p4_header* m);

struct p4_add_table_entry* create_p4_add_table_entry(char* buffer, uint16_t offset, uint16_t maxlength);
struct p4_field_match_lpm* add_p4_field_match_lpm(struct p4_add_table_entry* add_table_entry, uint16_t maxlength);
struct p4_add_table_entry* unpack_p4_add_table_entry(char* buffer, uint16_t offset);
struct p4_field_match_header* unpack_p4_field_match_header(char* buffer, uint16_t offset);
struct p4_field_match_lpm* unpack_p4_field_match_lpm(char* buffer, uint16_t offset);
struct p4_field_match_exact* add_p4_field_match_exact(struct p4_add_table_entry* add_table_entry, uint16_t maxlength);
struct p4_field_match_exact* unpack_p4_field_match_exact(char* buffer, uint16_t offset);
struct p4_field_match_range* add_p4_field_match_range(struct p4_add_table_entry* add_table_entry, uint16_t maxlength);
struct p4_field_match_range* unpack_p4_field_match_range(char* buffer, uint16_t offset);
struct p4_field_match_valid* add_p4_field_match_valid(struct p4_add_table_entry* add_table_entry, uint16_t maxlength);
struct p4_field_match_valid* unpack_p4_field_match_valid(char* buffer, uint16_t offset);
struct p4_field_match_ternary* add_p4_field_match_ternary(struct p4_add_table_entry* add_table_entry, uint16_t maxlength);
struct p4_field_match_ternary* unpack_p4_field_match_ternary(char* buffer, uint16_t offset);
struct p4_action* add_p4_action(struct p4_header* header, uint16_t maxlength);
struct p4_action* unpack_p4_action(char* buffer, uint16_t offset);
struct p4_action_parameter* add_p4_action_parameter(struct p4_header* header, struct p4_action* action, uint16_t maxlength);
struct p4_action_parameter* unpack_p4_action_parameter(char* buffer, uint16_t offset);
struct p4_set_default_action* create_p4_set_default_action(char* buffer, uint16_t offset, uint16_t maxlength);
struct p4_set_default_action* unpack_p4_set_default_action(char* buffer, uint16_t offset);
struct p4_digest* create_p4_digest(char* buffer, uint16_t offset, uint16_t maxlength);
struct p4_digest* unpack_p4_digest(char* buffer, uint16_t offset);
struct p4_digest_field* add_p4_digest_field(struct p4_digest* digest, uint16_t maxlength);
struct p4_digest_field* unpack_p4_digest_field(char* buffer, uint16_t offset);

struct p4_field_match_lpm* netconv_p4_field_match_lpm(struct p4_field_match_lpm* m);
struct p4_field_match_exact* netconv_p4_field_match_exact(struct p4_field_match_exact* m);
struct p4_field_match_range* netconv_p4_field_match_range(struct p4_field_match_range* m);
struct p4_field_match_ternary* netconv_p4_field_match_ternary(struct p4_field_match_ternary* m);
struct p4_field_match_valid* netconv_p4_field_match_valid(struct p4_field_match_valid* m);
struct p4_digest_field* netconv_p4_digest_field(struct p4_digest_field* m);
struct p4_action* netconv_p4_action( struct p4_action* m);
struct p4_action_parameter* netconv_p4_action_parameter(struct p4_action_parameter* m);
struct p4_set_default_action* netconv_p4_set_default_action(struct p4_set_default_action* m);
struct p4_field_match_header* netconv_p4_field_match_complex(struct p4_field_match_header *m, int* size);
struct p4_add_table_entry* netconv_p4_add_table_entry(struct p4_add_table_entry* m);
*/

/* ################################################################################## */
/* ####################### copied from dpdk_primitives.h mostly ##################### */

#define FIELD_FIXED_WIDTH(f) (f != header_instance_var_width_field[field_instance_header[f]])
#define FIELD_FIXED_POS(f)   (f <= header_instance_var_width_field[field_instance_header[f]] || header_instance_var_width_field[field_instance_header[f]] == -1)

#define FIELD_DYNAMIC_BITWIDTH(pd, f) (FIELD_FIXED_WIDTH(f) ? field_instance_bit_width[f] : (pd)->headers[field_instance_header[f]].var_width_field_bitwidth)
#define FIELD_DYNAMIC_BYTEOFFSET(pd, f) (field_instance_byte_offset_hdr[f] + (FIELD_FIXED_POS(f) ? 0 : ((pd)->headers[field_instance_header[f]].var_width_field_bitwidth / 8)))

#define field_desc(pd, f) (field_reference_t) \
               { \
                 .header     = field_instance_header[f], \
                 .meta       = header_instance_is_metadata[field_instance_header[f]], \
                 .bitwidth   =   FIELD_DYNAMIC_BITWIDTH(pd, f), \
                 .bytewidth  =  (FIELD_DYNAMIC_BITWIDTH(pd, f) + 7) / 8, \
                 .bytecount  = ((FIELD_DYNAMIC_BITWIDTH(pd, f) + 7 + field_instance_bit_offset[f]) / 8), \
                 .bitoffset  = field_instance_bit_offset[f], \
                 .byteoffset = FIELD_DYNAMIC_BYTEOFFSET(pd, f), \
                 .mask       = field_instance_mask[f], \
                 .fixed_width= FIELD_FIXED_WIDTH(f), \
                 .byte_addr  = (((uint8_t*)(pd)->headers[field_instance_header[f]].pointer)+(FIELD_DYNAMIC_BYTEOFFSET(pd, f))), \
               }

#define header_info(h) (header_reference_t) \
               { \
                 .header_instance = h, \
                 .bytewidth       = header_instance_byte_width[h], \
                 .var_width_field  = header_instance_var_width_field[h], \
               }

#define FIELD_BITCOUNT(pd, field) (field_desc(pd, field).bitwidth + field_desc(pd, field).bitoffset)

#define FIELD_MASK(pd, field) field_desc(pd, field).mask

#define FIELD_BYTES(pd, field) ( \
     field_desc(pd, field).bytecount == 1 ? (*(uint8_t*)  field_desc(pd, field).byte_addr) : \
    (field_desc(pd, field).bytecount == 2 ? (*(uint16_t*) field_desc(pd, field).byte_addr) : \
                                            (*(uint32_t*) field_desc(pd, field).byte_addr) ) )

#define FIELD_MASKED_BYTES(pd, field) (FIELD_BYTES(pd, field) & FIELD_MASK(pd, field))

#define BITS_MASK1(pd, field) (FIELD_MASK(pd, field) & 0xff)
#define BITS_MASK2(pd, field) (FIELD_MASK(pd, field) & (0xffffffff >> ((4 - (FIELD_BITCOUNT(pd, field) - 1) / 8) * 8)) & 0xffffff00)
#define BITS_MASK3(pd, field) (FIELD_MASK(pd, field) & (0xff << (((FIELD_BITCOUNT(pd, field) - 1) / 8) * 8)))

#define MODIFY_INT32_BYTEBUF(pd, dstfield, src, srclen) { \
    value32 = 0; \
    memcpy(&value32, src, srclen); \
    MODIFY_INT32_INT32_AUTO(pd, dstfield, value32); \
}

// Modifies a field in the packet by the given source and length [ONLY BYTE ALIGNED]
#define MODIFY_BYTEBUF_BYTEBUF(pd, dstfield, src, srclen) { \
    /*TODO: If the src contains a signed negative value, then the following memset is incorrect*/ \
    memset(field_desc(pd, dstfield).byte_addr, 0, field_desc(pd, dstfield).bytewidth - srclen); \
    memcpy(field_desc(pd, dstfield).byte_addr + (field_desc(pd, dstfield).bytewidth - srclen), src, srclen); \
}

// Modifies a field in the packet by a uint32_t value (no byteorder conversion) [MAX 4 BYTES]
// assuming `uint32_t res32' is in the scope
#define MODIFY_INT32_INT32_BITS(pd, dstfield, value32) { \
    if(field_desc(pd, dstfield).bytecount == 1) \
        res32 = (FIELD_BYTES(pd, dstfield) & ~FIELD_MASK(pd, dstfield)) | (value32 << (8 - FIELD_BITCOUNT(pd, dstfield)) & FIELD_MASK(pd, dstfield)); \
    else if(field_desc(pd, dstfield).bytecount == 2) \
        res32 = (FIELD_BYTES(pd, dstfield) & ~FIELD_MASK(pd, dstfield)) | \
                (value32 &  BITS_MASK1(pd, dstfield)) | \
               ((value32 & (BITS_MASK3(pd, dstfield) >> (16 - field_desc(pd, dstfield).bitwidth))) << (16 - field_desc(pd, dstfield).bitwidth)); \
    else \
        res32 = (FIELD_BYTES(pd, dstfield) & ~FIELD_MASK(pd, dstfield)) | \
                (value32 &  BITS_MASK1(pd, dstfield)) | \
               ((value32 & (BITS_MASK2(pd, dstfield) >> field_desc(pd, dstfield).bitoffset)) << field_desc(pd, dstfield).bitoffset) | \
               ((value32 & (BITS_MASK3(pd, dstfield) >> (field_desc(pd, dstfield).bytecount * 8 - field_desc(pd, dstfield).bitwidth))) << (field_desc(pd, dstfield).bytecount * 8 - field_desc(pd, dstfield).bitwidth)); \
    memcpy(field_desc(pd, dstfield).byte_addr, &res32, field_desc(pd, dstfield).bytecount); \
}

// Modifies a field in the packet by a uint32_t value with byte conversion (always) [MAX 4 BYTES]
// assuming `uint32_t res32' is in the scope
#define MODIFY_INT32_INT32_HTON(pd, dstfield, value32) { \
    if(field_desc(pd, dstfield).bytecount == 1) \
        res32 = (FIELD_BYTES(pd, dstfield) & ~FIELD_MASK(pd, dstfield)) | ((value32 << (8 - FIELD_BITCOUNT(pd, dstfield))) & FIELD_MASK(pd, dstfield)); \
    memcpy(field_desc(pd, dstfield).byte_addr, &res32, field_desc(pd, dstfield).bytecount); \
}
// Modifies a field in the packet by a uint32_t value with byte conversion when necessary [MAX 4 BYTES]
// assuming `uint32_t res32' is in the scope
#define MODIFY_INT32_INT32_AUTO(pd, dstfield, value) { \
    if(field_desc(pd, dstfield).meta) MODIFY_INT32_INT32_BITS(pd, dstfield, value) else MODIFY_INT32_INT32_HTON(pd, dstfield, value) \
}

// Gets the value of a field
//removed rte, now true and false results are the same
#define GET_INT32_AUTO(pd, field) (field_desc(pd, field).meta ? \
    (field_desc(pd, field).bytecount == 1 ? (FIELD_MASKED_BYTES(pd, field) >> (8 - FIELD_BITCOUNT(pd, field))) : \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK1(pd, field)) | \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK2(pd, field)) >> field_desc(pd, field).bitoffset) | \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK3(pd, field)) >> (field_desc(pd, field).bytecount * 8 - field_desc(pd, field).bitwidth)))) :\
    (field_desc(pd, field).bytecount == 1 ? (FIELD_MASKED_BYTES(pd, field) >> (8 - FIELD_BITCOUNT(pd, field))) : \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK1(pd, field)) | \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK2(pd, field)) >> field_desc(pd, field).bitoffset) | \
                                        ((FIELD_BYTES(pd, field) & BITS_MASK3(pd, field)) >> (field_desc(pd, field).bytecount * 8 - field_desc(pd, field).bitwidth)))))

// Extracts a field to the given uint32_t variable (no byteorder conversion) [MAX 4 BYTES]
#define EXTRACT_INT32_BITS(pd, field, dst) { \
    if(field_desc(pd, field).bytecount == 1) \
        dst = FIELD_MASKED_BYTES(pd, field) >> (8 - FIELD_BITCOUNT(pd, field)); \
    else if(field_desc(pd, field).bytecount == 2) \
        dst = (FIELD_BYTES(pd, field) & BITS_MASK1(pd, field)) | \
             ((FIELD_BYTES(pd, field) & BITS_MASK3(pd, field)) >> (16 - field_desc(pd, field).bitwidth)); \
    else \
        dst = (FIELD_BYTES(pd, field) & BITS_MASK1(pd, field)) | \
             ((FIELD_BYTES(pd, field) & BITS_MASK2(pd, field)) >> field_desc(pd, field).bitoffset) | \
             ((FIELD_BYTES(pd, field) & BITS_MASK3(pd, field)) >> (field_desc(pd, field).bytecount * 8 - field_desc(pd, field).bitwidth)); \
}

// Extracts a field to the given uint32_t variable with byte conversion (always) [MAX 4 BYTES]
#define EXTRACT_INT32_NTOH(pd, field, dst) { \
    if(field_desc(pd, field).bytecount == 1) \
        dst =                  FIELD_MASKED_BYTES(pd, field)  >> (8  - FIELD_BITCOUNT(pd, field)); \
}

// Extracts a field to the given uint32_t variable with byte conversion when necessary [MAX 4 BYTES]
#define EXTRACT_INT32_AUTO(pd, field, dst) { \
    if(field_desc(pd, field).meta) EXTRACT_INT32_BITS(pd, field, dst) else EXTRACT_INT32_NTOH(pd, field, dst) \
}

// Extracts a field to the given destination [ONLY BYTE ALIGNED]
#define EXTRACT_BYTEBUF(pd, field, dst) { \
    memcpy(dst, field_desc(pd, field).byte_addr, field_desc(pd, field).bytewidth); \
}

/* ############################################################################## */
/* ########################## copied from dpdk_lib.h&c ########################## */

#ifndef NB_TABLES
#define NB_TABLES 0 //defined as 2 in data_plane_data.h 
#endif




#endif //P4_INTERFACE_DEP
