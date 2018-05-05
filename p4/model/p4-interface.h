/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef P4_INTERFACE
#define P4_INTERFACE

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include "parser.h"
#include "data_plane_data.h"
#include "p4-interface-dependencies.h"


// Structs for the lookup tables
typedef struct table_entry table_entry_t;
struct table_entry {                   
    uint8_t* key;         
    uint8_t* value;
    uint8_t* mask;
    table_entry_t* next;
    table_entry_t* child;
};

typedef struct lookup_table_s {
    char* name;
    unsigned id;
    uint8_t type;
    uint8_t key_size;
    uint8_t val_size;
    int min_size;
    int max_size;
    void* default_val;
    table_entry_t* table;
    //int counter;
    //int socketid;
    //int instance;
} lookup_table_t;

typedef int(*p4_msg_digest)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list);

typedef struct backend_data_s {
    lookup_table_t** t;
    p4_msg_digest pdc;
    uint8_t* metadata;
} backend_data_t;


// Structs used by the packet_descriptor
typedef struct field_reference_s {
    header_instance_t header;
    int meta;
    int bitwidth;
    int bytewidth;
    int bytecount;
    int bitoffset;
    int byteoffset;
    uint32_t mask;
    int fixed_width;     // Determines whether the field has a fixed width
    uint8_t* byte_addr;  // Pointer to the byte containing the first bit of the field in the packet
} field_reference_t;

typedef struct header_reference_s {
    header_instance_t header_instance;
    int bytewidth;
    int var_width_field;
} header_reference_t;

typedef struct header_descriptor_s {
    header_instance_t   type;
    void *              pointer;
    uint32_t            length;
    int                 var_width_field_bitwidth;
} header_descriptor_t;

typedef struct packet_descriptor_s {
    uint8_t*              				data;
    header_descriptor_t 					headers[HEADER_INSTANCE_COUNT+1];
    parsed_fields_t     				  fields;
    void*                         wrapper; //packet* originally (pointer to the original packet)
    uint8_t             				  dropped;
} packet_descriptor_t;


//Global variables
typedef backend_data_t* backend;
typedef void* digest;
extern lookup_table_t table_config[NB_TABLES];
#define MAX_PORTS 255
#define BROADCAST_PORT 100  //hard-coded in p4-generated file
#define ETH_ADDR_LEN 6	
#define ETH_TYPE_LEN 2
#define VLAN_TAG_LEN 4


//Functions for lookup table creation and deletion
void set_fake_backend (lookup_table_t** t, int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list));
void create_tables (lookup_table_t** t);
void delete_tables (lookup_table_t** t);
void free_entries (table_entry_t* e);
void free_entry (table_entry_t* e);
void table_setdefault (lookup_table_t* t, uint8_t* value);


//Functions for managing the lookup tables
int   exact_add (lookup_table_t* t, uint8_t* key,                        uint8_t* value);
int     lpm_add (lookup_table_t* t, uint8_t* key, uint8_t prefix_length, uint8_t* value);
int ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask,         uint8_t* value);
uint8_t*    exact_lookup (lookup_table_t* t, uint8_t* key);
uint8_t*      lpm_lookup (lookup_table_t* t, uint8_t* key);
uint8_t*  ternary_lookup (lookup_table_t* t, uint8_t* key);
int   exact_remove (lookup_table_t* t, uint8_t* key                        );
int     lpm_remove (lookup_table_t* t, uint8_t* key, uint8_t prefix_length );
int ternary_remove (lookup_table_t* t, uint8_t* key                        );


//Functions for packet handling
void init_dataplane(packet_descriptor_t* packet, lookup_table_t** tables);
void initialize(packet_descriptor_t* pd, int inport);
void handle_packet(packet_descriptor_t* packet, lookup_table_t** tables);


//Utility functions
void debug(char* str, ...);
void print_key(uint8_t* key, int n);
void print_lpm_tree(table_entry_t* t);
int get_outport(packet_descriptor_t* p);
int compare_keys(uint8_t* key1, uint8_t* key2, uint8_t length);
int compare_ternary_keys(uint8_t* key1, uint8_t* key2, uint8_t length, uint8_t* mask);


//Functions for the error-free building of the P4 switch
void generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list);
void table_setdefault_promote  (int tableid, uint8_t* value);
void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);
void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);
void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);
backend create_backend(int num_of_threads, int queue_size, char* controller_name, int controller_port, p4_msg_callback cb);
void launch_backend(backend bg);
uint16_t calculate_csum16(const void* buf, uint16_t len);


#endif // P4_INTERFACE
