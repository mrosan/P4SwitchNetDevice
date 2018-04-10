#include "p4-interface.h"
#include "actions.h"

// Initializes the switch's lookup tables, from the data provided in table_config (global variable)
//PARAM t: pointer to an array of lookup tables
void
create_tables (lookup_table_t** t)
{
		int i;
		for (i=0; i<NB_TABLES; i++) {
				printf(" *** Setting properties for table %d\n",i);
				//setting table properties that are defined in table.c
				t[i] = (lookup_table_t *) malloc(sizeof(lookup_table_t));
				t[i]->id = table_config[i].id;
				t[i]->type = table_config[i].type;
				t[i]->key_size = table_config[i].key_size;
				t[i]->val_size = table_config[i].val_size;
				//allocating memory for default_val, but the value will need to be assigned in init_tables_callback
				t[i]->default_val = (uint8_t*)malloc(table_config[i].val_size);
				t[i]->min_size = table_config[i].min_size;
				t[i]->max_size = table_config[i].max_size;
				t[i]->table = NULL;
		}
	
		extern backend bg;
		bg = (backend_data_t *) malloc(sizeof(backend_data_t));
}

// Initializes a "backend", which the P4 switch tries to use to access the lookup tables
//PARAM t: pointer to an array of lookup tables
//PARAM p4_msg_digest_callback: pointer to the callback function (which is set by the user in the simulation file)
void
set_fake_backend (lookup_table_t** t, int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list))
{
		extern backend bg;
		bg->t = t;
		bg->pdc = p4_msg_digest_callback;
}

// Recursive function for freeing the lookup tables and their values within
//PARAM e: pointer to a table entry
void
free_entries (table_entry_t* e)
{
		if( e ) {				
				free_entries(e->next);
				free_entries(e->child);
				if (e->depth) free(e->depth);
				if (e->key) free(e->key);
				if (e->value) free(e->value);
				free(e);
		}
}

// Function for initiating the deletion of all the lookup tables.
//PARAM t: pointer to an array of lookup tables
void
delete_tables (lookup_table_t** t)
{
		printf("Destructing tables.\n");
		int i;
		for (i=0; i<NB_TABLES; i++) {
				free_entries(t[i]->table);
				free(t[i]->default_val);
		}
}

// Function for adding a new entry to a lookup table with EXACT key match
// This function can be called on any table, regardless of table type.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//PARAM value: the pointer to the new value we'd like to add
//RETURNS: 0 if no error occurred
int
exact_add (lookup_table_t* t, uint8_t* key, uint8_t* value)
{
		printf("\n ------> performing exact_add with value %d\n\n",*value);
		
		table_entry_t* p = t->table;	// for searching the key
		table_entry_t* q = NULL;			// for inserting the new entry
		int match_possible = 1;
		
		while ( p && match_possible) {
				int c = compare_keys(key,p->key,t->key_size);
				if ( c < 0 ) {						// haven't found matching entry yet
						q = p;
						p = p->next;
				} else if ( c > 0 ) {			// certainly won't find matching entry
						match_possible = 0;
				} else {									// found a matching entry
						printf("~~~ exact_add: key already exist, overwriting value ~~~\n");
						free(p->value);
						p->value = (uint8_t*)malloc(t->val_size);
						memcpy(p->value, value, t->val_size);
						return 0;
				}
		}
		
		// if the function didn't return -- create a new entry
		//create entry
		table_entry_t *res = (table_entry_t *) malloc(sizeof(table_entry_t));
		res->depth = malloc(sizeof(uint8_t));
		res->depth = NULL;
		res->next = NULL;
		res->child = NULL;
		//set its key
		res->key = (uint8_t*)malloc(t->key_size * sizeof(uint8_t));
		int j;
		for (j=0; j<t->key_size; j++) {
				res->key[j] = key[j];
		}
		//set its value
		res->value = value;
		res->value = (uint8_t*)malloc(t->val_size);
    memcpy(res->value, value, t->val_size);
		//set the pointers
		if ( q ) {
				res->next = q->next;
				q->next = res;
		} else {
				res->next = p;	//either NULL or the first element
				t->table = res;
		}
		
		return 0;
}

// Function for adding a new entry to a lookup table with longest prefix match
// This function can be called on any table, regardless of table type.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//PARAM prefix_length: the number of bytes which determines the size of the prefix
//PARAM value: the pointer to the new value we'd like to add
//RETURNS: 0 if no error occurred, error number otherwise
int
lpm_add (lookup_table_t* t, uint8_t* key, uint8_t prefix_length, uint8_t* value)
{
		uint8_t depth = prefix_length/8;
		// The prefix length should be at least 1 byte. At depth == 0, the user should just set the default_val
		if ( 0 < depth && depth <= t->key_size ) {
		
				uint8_t d = 1;									// current depth
				table_entry_t* p = t->table;		// pointer to current entry
				table_entry_t* q = NULL;				// pointer to last entry
				table_entry_t* res = NULL;			// pointer to the entry where the value needs to be inserted
				
				//go down in the tree to the desired depth, create nodes along the way if necessary
				while( d <= depth ) {
						int match_on_this_depth = 0;
						//see if we already have a matching table_entry on this depth
						while ( p && !match_on_this_depth ) {
								int c = compare_keys(p->key,key,d);
								if ( c < 0 ) {						// haven't found matching entry at this depth yet
										q = p;
										p = p->next;
								} else if ( c > 0 ) {			// certainly won't find matching entry on this depth
										p = NULL;
								} else {									// found a matching entry
										printf("Found matching entry at depth %d\n",d);
										match_on_this_depth = 1;
										res = p;
										q = p;
										p = p->child;
								}
						}
						
						//if there was no matching entry on this depth, create one (without assigning value)
						if ( !match_on_this_depth ) {
								table_entry_t* s = (table_entry_t *) malloc(sizeof(table_entry_t));
								//assign key -- only the necessary bytes are allocated!
								s->key = malloc(d * sizeof(uint8_t));
								int j = 0;
								while ( j < d ){
										s->key[j] = key[j];
										j++;
								}
								//set depth
								s->depth = malloc(sizeof(uint8_t));
								*(s->depth) = d;
								//at first, set the rest to NULL
								s->next = NULL;
								s->child = NULL;
								s->value = NULL;
								//then link it to the tree	
								if ( res ) {																	//if the tree wasn't empty
										// new entry comes after pointer q,
										// but we have to determine
										// whether it is the "child" of q,
										// or the "next" of q
										if ( *(res->depth) == *(q->depth) ) { 		// p is one depth lower than q (p can be NULL, though)
												q->child = s;
										} else {																	// p is on the same level as q (p can be NULL, though)
												q->next = s;
										}
										s->next = p;
										
								} else {																			//if the tree was empty
										t->table = s;
										q = s;
										p = s->child; //NULL
								}
								res = s;
								printf("Added entry on level %d with key ",d);	print_key(q->key,d);
						}
						d++;
				}
				
				//finally (once we reached the desired depth) assign value
				if ( res->value ) {
						printf("Value already exists at this entry! Overwriting value...\n");
						free(res->value);
				} else {
						printf("Adding value to the entry.\n");
				}
				res->value = (uint8_t*)malloc(t->val_size);
		    memcpy(res->value, value, t->val_size);
		    
		} else {
				return 1;
		}

		return 0;
}

int
ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
		return 1;
}

void
generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{

		bg->pdc(bg->t,name,receiver,digest_field_list);
		
}

// Function for searching in a table by exactly matching keys.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: the (action) value of the matching entry, or the default value in case of no match
uint8_t*
exact_lookup (lookup_table_t* t, uint8_t* key)
{
		uint8_t* res = NULL;
	  table_entry_t* s = t-> table;
	  
	  while ( s ) {
	  		int c = compare_keys(key,s->key,t->key_size);
	  		if ( c < 0 ) {						// haven't found matching entry yet
						s = s->next;
				} else if ( c > 0 ) {			// certainly won't find matching entry
						s = NULL;
				} else {									// found a matching entry
						res = s->value;
						s = NULL;
				}
	  }
	  
	  if(res) {
			printf("   ::: exact_lookup was successful with value %d and key ",*res); print_key(key,t->key_size); 	
		} else {
			printf("   ::: exact_lookup didn't find match for key "); print_key(key,t->key_size);
			res = t->default_val;
		}	
		
    return res;
}

// Function for searching in a tree by using longest prefix match on the keys.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: the (action) value of the matching entry, or the default value in case of no match
uint8_t*
lpm_lookup (lookup_table_t* t, uint8_t* key)
{
		table_entry_t* s = t->table;
		uint8_t* res = NULL;
		// parse tree until either there's no match on the current depth, or we reach the "bottom"
		while ( s ) {
				int d = *(s->depth);
				int c = compare_keys(s->key,key,d);
				if ( c < 0 ) {						// haven't found matching entry on this depth yet
						s = s->next;
				} else if ( c > 0 ) {			// certainly won't find matching entry on this depth
						s = NULL;
				} else {									// found a matching entry
						// set result only if the node has a value
						if (s->value) {
								res = s->value;
								printf("   ::: lpm_lookup found value at depth %d for key ", d); print_key(key,d);
						}
						// go one level deeper
						s = s->child;
				}
		}
		
		// if we never assigned value to the result while we parsed the tree, assign default value
		if (!res) {
				res = t->default_val;
				printf("   ::: lpm_lookup assigned the default value.\n");
		}
		
		return res;
}

uint8_t*
ternary_lookup (lookup_table_t* t, uint8_t* key)
{
		return NULL;
}

// Function for setting a lookup table's default action value,
// which will be returned if the lookup was unsuccessful.
//PARAM t: pointer to a lookup table
//PARAM value: pointer to a table action
void
table_setdefault(lookup_table_t* t, uint8_t* value)
{
		if( t->default_val ) free(t->default_val);
    t->default_val = value;
}

// Function for comparing two keys of the same length, determining which key is smaller
// Used by _lookup and _add functions
//PARAM key1: the first key
//PARAM key2: the second key
//PARAM length: the length of the keys in bytes.
//RETURNS -1 is key1 is smaller, 1 if key2 is smaller, 0 if they match
int
compare_keys(uint8_t* key1, uint8_t* key2, uint8_t length)
{
		int result = 0;
		int i = 0;
		while(i<length && result==0){
				if (key1[i] < key2[i]){
						result = -1;
				} else if(key1[i] > key2[i])
				{
						result = 1;
				}
				i++;
		}
		return result;
}

// Function to be called after init_dataplane.
// While init_dataplane is a compiler-generated function, this one isn't:
// further initialization steps can be specified.
void
initialize(packet_descriptor_t* pd, int inport)
{
		uint32_t res32; //needed for the macro
		// sets the packet descriptor's ingress port field, which isn't set by default
  	MODIFY_INT32_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, (uint32_t) inport);
}

// Getter function for extracting the port, through which the packet should be forwarded
int
get_outport(packet_descriptor_t* p)
{
		//originally, there were two macros:
		//#define EXTRACT_EGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_egress_port) 
		//#define EXTRACT_INGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_ingress_port)
		
		int port = GET_INT32_AUTO(p, field_instance_standard_metadata_egress_port);
		return port;
}

void
print_key(uint8_t* key, int n)
{
		int i;
		for (i=0;i<n;i++){
				//printf("%02x ", key[i]);
				printf("%d", key[i]);
		}
		printf("\n");
}

void
debug (char* str,...)
{
		int dbg = 1;
    if(dbg)
    {
    	printf("Debug MSG:	%s \n", str);
    }
}

uint16_t
calculate_csum16(const void* buf, uint16_t length)
{
    uint16_t value16 = 0;
    return value16;
}

void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value)
{ 
	extern backend bg; 
	exact_add(bg->t[tableid],key,value); 
}

void lpm_add_promote (int tableid, uint8_t* key, uint8_t depth, uint8_t* value)
{ 
	extern backend bg; 
	lpm_add(bg->t[tableid],key,depth,value); 
}

void ternary_add_promote (int tableid, uint8_t* key, uint8_t* mask, uint8_t* value)
{ 
	extern backend bg; 	
	ternary_add(bg->t[tableid],key,mask,value); 
}

void table_setdefault_promote  (int tableid, uint8_t* value)
{ 
	extern backend bg;
	table_setdefault(bg->t[tableid],value); 
}

backend 
create_backend (int num_of_threads, int queue_size, char* controller_name, int controller_port, p4_msg_callback cb)
{
    //do nothing
    extern backend bg;
		return (backend) bg;
}

void launch_backend (backend bg) { /*do nothing*/ }

