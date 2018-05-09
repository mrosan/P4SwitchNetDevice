#include "p4-interface.h"
#include "actions.h"

// Initializes the switch's lookup tables, from the data provided in table_config (global variable)
//PARAM t: pointer to an array of lookup tables
void
create_tables (lookup_table_t** t)
{
	int i;
	for (i=0; i<NB_TABLES; i++) {
		printf("Setting properties for table %d.\n",i);
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

// Recursive function for freeing the lookup tables and their values within
//PARAM e: pointer to a table entry
void
free_entries (table_entry_t* e)
{
	if( e ) {				
		free_entries(e->next);
		free_entries(e->child);
		free_entry(e);
		//e = NULL;
	}
}

// Function that frees a table_entry_t
//PARAM e: pointer to a table entry
void
free_entry (table_entry_t* e) {
	if (e->mask) free(e->mask);
	if (e->key) free(e->key);
	if (e->value) free(e->value);
	free(e);
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
	printf("Performing exact_add with value %d and key ",*value); print_key(key,t->key_size);
		
	table_entry_t* p = t->table;			// for searching the key
	table_entry_t* q = NULL;				// for inserting the new entry
	int match_possible = 1;
		
	while ( p && match_possible) {
		int c = compare_keys(key,p->key,t->key_size);
		if ( c < 0 ) {						// haven't found matching entry yet
			q = p;
			p = p->next;
		} else if ( c > 0 ) {				// certainly won't find matching entry
			match_possible = 0;
		} else {							// found a matching entry
			//printf("~~~ exact_add: key already exists, overwriting value ~~~\n");
			free(p->value);
			p->value = (uint8_t*)malloc(t->val_size);
			memcpy(p->value, value, t->val_size);
			return 0;
		}
	}
		
	// if the function didn't return -- create a new entry
	//create entry
	table_entry_t *res = (table_entry_t *) malloc(sizeof(table_entry_t));
	res->mask = NULL;
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
// Note: the table_entry type's mask value is used here as depth, which contains the prefix length in bytes
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//PARAM prefix_length: the number of bytes which determines the size of the prefix
//PARAM value: the pointer to the new value we'd like to add
//RETURNS: 0 if no error occurred, error number otherwise
int
lpm_add (lookup_table_t* t, uint8_t* key, uint8_t prefix_length, uint8_t* value)
{
	printf("Performing lpm_add with value %d, depth %d and key ",*value,prefix_length/8); print_key(key,t->key_size);

	uint8_t depth = prefix_length/8;
	// The prefix length should be at least 1 byte. At depth == 0, the user should just set the default_val
	if ( 0 < depth && depth <= t->key_size ) {
		
		uint8_t d = 1;						// current depth
		table_entry_t* p = t->table;		// pointer to current entry
		table_entry_t* q = NULL;			// pointer to last entry
		table_entry_t* res = NULL;			// pointer to the entry where the value needs to be inserted
				
		//go down in the tree to the desired depth, create nodes along the way if necessary
		while( d <= depth ) {
			int match_on_this_depth = 0;
			//see if we already have a matching table_entry on this depth
			while ( p && !match_on_this_depth ) {
				int c = compare_keys(p->key,key,d);
				if ( c < 0 ) {
					//printf("~~~ lpm_add: haven't found yet any matching entry at depth %d\n",d);
					q = p;
					p = p->next;
				} else if ( c > 0 ) {
					//printf("~~~ lpm_add: matching entry doesn't exist at depth %d\n",d);
					p = NULL;
				} else {
					//printf("~~~ lpm_add: found matching entry at depth %d\n",d);
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
				s->mask = malloc(sizeof(uint8_t));
				*(s->mask) = d;
				//at first, set the rest to NULL
				s->next = NULL;
				s->child = NULL;
				s->value = NULL;
				//then link it to the tree
				if ( res ) { // if the new entry's depth isn't 1
					// new entry comes after pointer q, but we have to determine
					// whether it is the "child" of q, or the "next" of q
					if ( *(res->mask) == *(q->mask) ) {
						s->next = q->child;
						q->child = s;
					} else {
						s->next = q->next;
						q->next = s;
					}
				} else { // if the new entry's depth is 1
					if ( q ) { // if it doesn't go to the very first position
						s->next = q->next;
						q->next = s;
					} else {
						t->table = s;
					}
				}
				q = s;
				res = s;
				//printf("~~~ lpm_add: created entry on level %d with key ",d);	print_key(res->key,d);
			}
			d++;
		}
				
		//finally (once we reached the desired depth) assign value
		if ( res->value ) {
			//printf("~~~ lpm_add: value already exists at this entry! Overwriting value...\n");
			free(res->value);
		} else {
			//printf("~~~ lpm_add: added value to the entry.\n");
		}
		res->value = (uint8_t*)malloc(t->val_size);
	    memcpy(res->value, value, t->val_size);
		    
	} else {
		return 1;
	}

	return 0;
}

// Function for adding a new entry to a lookup table with longest prefix match
// This function can be called on any table, regardless of table type.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//PARAM mask: an array consisting of 0 and 255 values, matching the key in length
//PARAM value: the pointer to the new value we'd like to add
//RETURNS: 0 if no error occurred, error number otherwise
int
ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
	printf("Performing ternary_add with value %d and key ",*value); print_key(key,t->key_size);
	
	table_entry_t* p = t->table;
		
	while ( p ) {
		int c = compare_ternary_keys(key,p->key,t->key_size,p->mask);
		if ( c != 0 ) {
			p = p->next;
		} else {
			//printf("~~~ ternary_add: key already exists, overwriting value ~~~\n");
			free(p->value);
			p->value = (uint8_t*)malloc(t->val_size);
			memcpy(p->value, value, t->val_size);
			return 0;
		}
	}
		
	// if the function didn't return -- create a new entry
	//create entry
	table_entry_t *res = (table_entry_t *) malloc(sizeof(table_entry_t));
	res->mask = NULL;
	res->next = NULL;
	res->child = NULL;
	//set its key
	res->key = (uint8_t*)malloc(t->key_size * sizeof(uint8_t));
	int j;
	for (j=0; j<t->key_size; j++) {
		res->key[j] = key[j];
	}
	//set its mask
	res->mask = (uint8_t*)malloc(t->key_size * sizeof(uint8_t));
	for (j=0; j<t->key_size; j++) {
		res->mask[j] = mask[j];
	}
	//set its value
	res->value = value;
	res->value = (uint8_t*)malloc(t->val_size);
	memcpy(res->value, value, t->val_size);
	//set the pointers
	res->next = t->table;
	t->table = res;
		
	return 0;
}

// Function for searching in a table by exactly matching keys.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: the (action) value of the matching entry, or the default value in case of no match
uint8_t*
exact_lookup (lookup_table_t* t, uint8_t* key)
{
	printf("Performing exact_lookup with key "); print_key(key,t->key_size);

	uint8_t* res = NULL;
	table_entry_t* s = t->table;
	while ( s ) {
		int c = compare_keys(key,s->key,t->key_size);
		if ( c < 0 ) { // haven't found matching entry yet
			s = s->next;
		} else if ( c > 0 ) { // certainly won't find matching entry
			s = NULL;
		} else { // found a matching entry
			res = s->value;
			s = NULL;
		}
	}

	if(res) {
		printf("   ::: exact_lookup was successful with value %d and key ",*res); print_key(key,t->key_size); 	
	} else {
		printf("   ::: exact_lookup returning with default_value for key "); print_key(key,t->key_size);
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
	printf("Performing lpm_lookup with key "); print_key(key,t->key_size);
	
	table_entry_t* s = t->table;
	uint8_t* res = NULL;
	// parse tree until either there's no match on the current depth, or we reach the "bottom"
	while ( s ) {
		int d = *(s->mask); // mask at lpm trees is only a number
		int c = compare_keys(s->key,key,d);
		if ( c < 0 ) {
			//printf("   ::: lpm_lookup: searching for a match at depth %d for key ", d); print_key(key,d);
			s = s->next;
		} else if ( c > 0 ) {
			//printf("   ::: lpm_lookup: no match exists at depth %d for key ", d); print_key(key,d);
			s = NULL;
		} else {
			//printf("   ::: lpm_lookup: matching entry at depth %d for key ", d); print_key(key,d);
			if (s->value) {
				res = s->value;
				printf("   ::: lpm_lookup: found value %d belonging to this entry \n", *s->value);
			}
			s = s->child;
		}
	}
		
	// if we never assigned value to the result while we parsed the tree, assign default value
	if (!res) {
		res = t->default_val;
		printf("   ::: lpm_lookup returning with default value\n");
	}
		
	return res;
}

// This function looks up an entry (from a TERNARY type table) that matches the given key.
// The keys within the table may be masked, and the lookup takes it into consideration, returning the first fitting match.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: 0 if there were no errors
uint8_t*
ternary_lookup (lookup_table_t* t, uint8_t* key)
{
	printf("Performing ternary_lookup with key "); print_key(key,t->key_size);

	uint8_t* res = NULL;
	table_entry_t* s = t->table;
	while ( s ) {
		int c = compare_ternary_keys(key,s->key,t->key_size,s->mask);
		if ( c != 0 ) {
			s = s->next;
		} else {
			res = s->value;
			s = NULL;
		}
	}

	if(res) {
		printf("   ::: ternary_lookup was successful with value %d and key ",*res); print_key(key,t->key_size); 	
	} else {
		printf("   ::: ternary_lookup didn't find match for key "); print_key(key,t->key_size);
		res = t->default_val;
	}	

	return res;
}

// This function deletes an entry (from an EXACT type table) that matches the given key.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: 0 if there were no errors
int
exact_remove (lookup_table_t* t, uint8_t* key)
{
	printf("Performing exact_remove with key "); print_key(key,t->key_size);
	
	table_entry_t* s = t->table;
	table_entry_t* q = NULL;
	while ( s ) {
		int c = compare_keys(key,s->key,t->key_size);
		if ( c < 0 ) {
			q = s;
			s = s->next;
		} else if ( c > 0 ) {
			s = NULL;
		} else {
			if (q) {
				q->next = s->next;
			} else {
				t->table = s->next;
			}
			free_entry(s);
			s = NULL;
			printf("   ::: exact_remove successful for key "); print_key(key,t->key_size);
		}
	}
	return 0;
}

// This function deletes an entry (from a LPM type table) that matches the given key and depth.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//PARAM prefix_length: the number of bytes which determines the size of the prefix
//only the entry with matching prefix length (depth) will be deleted
//RETURNS: 0 if there were no errors
int
lpm_remove (lookup_table_t* t, uint8_t* key, uint8_t prefix_length)
{
	printf("Performing lpm_remove with key "); print_key(key,t->key_size);
	
	uint8_t depth = prefix_length/8;
	if ( 0 < depth && depth <= t->key_size ) {
		table_entry_t* s = t->table;
		table_entry_t* q = NULL;
		while ( s ) {
			int d = *(s->mask); // mask at lpm trees is only a number
			int c = compare_keys(s->key,key,d);
			if ( c < 0 ) {
				q = s;
				s = s->next;
			} else if ( c > 0 ) {
				s = NULL;
			} else {
				if (d == depth) {
					// Entries can only safely deleted if it doesnt have any child.
					// If that's the case, only the value will be removed.
					if( !s->child ) {
						if (q) {
							if ( *(s->mask) == *(q->mask) ) {
								q->next = s->next;
							} else {
								q->child = s->next;
							}
						} else {
							t->table = s->next;
						}
						free_entry(s);
						printf("   ::: lpm_remove removed the ENTRY at depth %d for key ", d); print_key(key,d);
					} else {
						if (s->value){
							free(s->value);
							s->value = NULL;
							printf("   ::: lpm_remove removed the VALUE at depth %d for key ", d); print_key(key,d);
						}
					}
					s = NULL;
				} else {
					q = s;
					s = s->child;
				}
			}
		}
		return 0;
	} else {
		return 1;
	}
}

// This function deletes ALL entries that has a matching masked key.
//PARAM t: pointer to a lookup table
//PARAM key: a pointer to an array of bytes, which is used for identifying the entries
//RETURNS: 0 if there were no errors
int
ternary_remove (lookup_table_t* t, uint8_t* key)
{
	printf("Performing ternary_remove with key "); print_key(key,t->key_size);

	table_entry_t* s = t->table;
	table_entry_t* q = NULL;
	while ( s ) {
		int c = compare_ternary_keys(key,s->key,t->key_size,s->mask);
		if ( c != 0 ) {
			q = s;
			s = s->next;
		} else {
			if (q) {
				q->next = s->next;
			} else {
				t->table = s->next;
			}
			free_entry(s);
			s = NULL;
		}
	}
	return 0;
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
// Used by exact and lpm functions
//PARAM key1: the first key
//PARAM key2: the second key
//PARAM length: the length of the keys in bytes.
//RETURNS -1 is key1 is smaller, 1 if key2 is smaller, 0 if they match
int
compare_keys(uint8_t* key1, uint8_t* key2, uint8_t length)
{
	int result = 0;
	int i = 0;
	while(i<length && result==0) {
		if (key1[i] < key2[i]) {
			result = -1;
		} else if(key1[i] > key2[i]) {
			result = 1;
		}
		i++;
	}
	return result;
}

// Function for comparing two keys of the same length, determining which key is smaller
// Used by ternary functions
//PARAM key1: the first key
//PARAM key2: the second key
//PARAM length: the length of the keys in bytes.
//PARAM mask: an array consisting of 0 and 255 values, matching the key in length
//where the mask's value is 0 the key is inspected, while at 255 it isn't
//RETURNS -1 is key1 is smaller, 1 if key2 is smaller, 0 if they match
int
compare_ternary_keys(uint8_t* key1, uint8_t* key2, uint8_t length, uint8_t* mask)
{
	int result = 0;
	int i = 0;
	while(i<length && result==0) {
		//if the mask is null, there is no mask
		if ( !mask || mask[i] == 0 ) {
			if (key1[i] < key2[i]) {
				result = -1;
			} else if(key1[i] > key2[i]) {
				result = 1;
			}
		}
		i++;
	}
	return result;
}

// Function which is called within the P4 switch, in order to use the controller.
// Redirects to the digest callback function.
void
generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{
	bg->pdc(bg->t,name,receiver,digest_field_list);	
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
	printf("[");
	int i;
	for (i=0;i<n-1;i++){
		//printf("%02x ", key[i]);
		printf("%d,", key[i]);
	}
	printf("%d]\n", key[n-1]);
}

// This function was used for debugging, it prints the keys within an lpm tree.
void
print_lpm_tree(table_entry_t* t)
{
	if( t ) {
		print_key(t->key, *(t->mask));
		
		printf("next: ");
		print_lpm_tree(t->next);
		
		printf("child: ");
		print_lpm_tree(t->child);
		
	} else {
		printf("NULL\n");
	}
}

void
debug (char* str,...)
{
	int dbg = 1;
	if(dbg) {
		printf("Debug MSG:	%s \n", str);
	}
}

uint16_t
calculate_csum16(const void* buf, uint16_t len)
{
	//uint16_t sum = 0;
	uint32_t sum = 0;
	
	uintptr_t ptr = (uintptr_t)buf;
	typedef uint16_t __attribute__((__may_alias__)) u16_p;
	const u16_p *u16 = (const u16_p *)ptr;

	while (len >= (sizeof(*u16) * 4)) {
		sum += u16[0];
		sum += u16[1];
		sum += u16[2];
		sum += u16[3];
		len -= sizeof(*u16) * 4;
		u16 += 4;
	}
	while (len >= sizeof(*u16)) {
		sum += *u16;
		len -= sizeof(*u16);
		u16 += 1;
	}
	/* if length is in odd bytes */
	if (len == 1)
	{
		sum += *((const uint8_t *)u16);
	}
		
	sum = ((sum & 0xffff0000) >> 16) + (sum & 0xffff);
	sum = ((sum & 0xffff0000) >> 16) + (sum & 0xffff);
	
	return (uint16_t) sum;
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

