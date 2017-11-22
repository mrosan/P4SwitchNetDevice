#include "p4-interface.h"
#include "actions.h"

void
create_tables (lookup_table_t** t)
{
		int i;
		for (i=0; i<NB_TABLES; i++)
		{
				printf(" *** Setting properties for table %d\n",i);
				//setting table properties that are defined in table.c
				t[i] = (lookup_table_t *) malloc(sizeof(lookup_table_t));
				t[i]->id = table_config[i].id;
				t[i]->type = table_config[i].type;
				t[i]->key_size = table_config[i].key_size;
				t[i]->val_size = table_config[i].val_size;
				//allocating memory for default_val, but the value needs to be assigned in init_tables_callback
				t[i]->default_val = (uint8_t*)malloc(table_config[i].val_size);
				t[i]->min_size = table_config[i].min_size;
				t[i]->max_size = table_config[i].max_size;
				t[i]->table = NULL;
		}
	
		extern backend bg;
		bg = (backend_data_t *) malloc(sizeof(backend_data_t));
}

void
set_fake_backend (lookup_table_t** t, int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list))
{
		extern backend bg;
		bg->t = t;
		bg->pdc = p4_msg_digest_callback;
}

void
free_entries (table_entry_t* e)
{
		if( e ) {
				
				/*
				if(e->depth){ printf("Going to free table entry with key "); print_key(e->key,*e->depth); 
				}
				else { printf("Going to free entry without depth (probably exact_table) - "); print_key(e->key,2); }
				*/
				
				free_entries(e->next);
				free_entries(e->child);
				if (e->depth) free(e->depth);
				if (e->key) free(e->key);
				if (e->value) free(e->value);
				free(e);
		}
}

void
delete_tables (lookup_table_t** t)
{
		printf("\nDECONSTRUCTING TABLES\n");
		int i;
		for (i=0; i<NB_TABLES; i++)
		{
				free_entries(t[i]->table);
				free(t[i]->default_val);
		}
}

void
exact_add (lookup_table_t* t, uint8_t* key, uint8_t* value)
{
		printf("\n ------> performing exact_add with value %d\n\n",*value);

		//check whether the entry already exists
		int not_yet_in_list = 1;
		table_entry_t* p;
		for(p=t->table; p != NULL; p=p->next) {
				if ( compare_keys(key,p->key,t->key_size) ) {
						not_yet_in_list = 0;
						break;
				}    
		} 
		//add value if it's new
		if (not_yet_in_list) {
				//create entry
				table_entry_t *s = (table_entry_t *) malloc(sizeof(table_entry_t));
				s->depth = malloc(sizeof(uint8_t));
				s->depth = NULL;
				s->next = NULL;
				s->child = NULL;
				//set key
				s->key = (uint8_t*)malloc(t->key_size * sizeof(uint8_t));
				int j;
				for (j=0; j<t->key_size; j++) {
						s->key[j] = key[j];
				}
				//set value
				s->value = value;
				s->value = (uint8_t*)malloc(t->val_size);
		    memcpy(s->value, value, t->val_size);
				//set pointers
				if ( t->table ){
						s->next = t->table->next;
						t->table->next = s;
				} else {
						t->table = s;
				}
		//free previous value and assign the new one	
		} else {
				printf("~~~ exact_add: key already exist, overwriting value ~~~\n");
				free(p->value);
				p->value = (uint8_t*)malloc(t->val_size);
		    memcpy(p->value, value, t->val_size);
		}	
}

void
lpm_add (lookup_table_t* t, uint8_t* key, uint8_t prefix_length, uint8_t* value)
{
		uint8_t depth = prefix_length/8;
		if ( 0 < depth && depth <= t->key_size ) {
				uint8_t d = 1;
				table_entry_t* res = t->table;
				table_entry_t* p = t->table;
				//go down in the tree to the desired depth
				while( d <= depth ) {
						int match_on_this_depth = 0;
						
						//see if we already have a matching table_entry on this level
						while ( p && !match_on_this_depth ) {
								//compare the key with the current key
								//if we found a match, save it as result then go one level deeper
								if (compare_keys(p->key,key,d)) { 
										printf("Found matching entry on level %d\n",d);
										match_on_this_depth = 1;
										res = p;
										p = p->child;	
								} else {
										printf("Didn't match on level %d compared to key ",d);	print_key(p->key,d);
										p = p->next;
								}
						}
						
						//if we didn't find the match, create the entry
						if ( !match_on_this_depth ) {
								table_entry_t* q = (table_entry_t *) malloc(sizeof(table_entry_t));
								//assign key value
								q->key = malloc(d * sizeof(uint8_t));
								int j = 0;
								while ( j < d ){
									q->key[j] = key[j];
									j++;
								}
								//assign depth value
								q->depth = malloc(sizeof(uint8_t));
								*(q->depth) = d;
								//set its pointers to NULL
								q->next = NULL;
								q->child = NULL;
								q->value = NULL;
								//link it to the tree and select it				
								if ( res ) {				//if the tree wasn't empty
										q->next = res->child;
										res->child = q;
										res = res->child;
								} else {						//if the tree was empty
										t->table = q;
										res = q;
								}
								p = NULL;
								printf("Added entry on level %d with key ",d);	print_key(q->key,d);
						}
						
						d++;
				}
				//now that we reached the desired depth, set value
				if ( !res->value ){
						printf("Adding value to the entry.\n");
						res->value = (uint8_t*)malloc(t->val_size);
		    		memcpy(res->value, value, t->val_size);
				} else {
						printf("Value already exists at this entry! Overwriting value...\n");
						free(res->value);
						res->value = (uint8_t*)malloc(t->val_size);
		    		memcpy(res->value, value, t->val_size);
				}
		} else {
				printf("Error: lmp_add unsuccessful due to incorrect mask format.\n");
		}
}

void
ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{

}

void
generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{

		bg->pdc(bg->t,name,receiver,digest_field_list);
		
}

uint8_t*
exact_lookup (lookup_table_t* t, uint8_t* key)
{ 
	  uint8_t* res = NULL;
	  table_entry_t* s;
	  // go through the table with linear search
		for(s=t->table; s != NULL; s=s->next) {	
				if ( compare_keys(key,s->key,t->key_size) ) {
					res = s->value;
					break;
				}    
		}
		// return default value if the search was unsuccessful
		if(res) {
			//printf("   ::: exact_lookup was successful with value %02x and key ",res[0]);
			printf("   ::: exact_lookup was successful with value %d and key ",*res);
			print_mac48(key); 	
		} else {
			printf("   ::: exact_lookup didn't find match for key ");
			print_mac48(key);
			res = t->default_val;
		}	
    return res;
}
 
uint8_t*
lpm_lookup (lookup_table_t* t, uint8_t* key)
{
		uint8_t* res = NULL;
		table_entry_t* s = t->table;
		// parse tree until either there's no match on the current depth, or we reach the "bottom"
		while ( s ) {
				int d = *(s->depth);
				// if there is a match, go one level deeper
				if ( compare_keys(s->key,key,d) ) {
						//set result only if the node has a value
						if (s->value) {
								res = s->value;
								printf("   ::: lpm_lookup found match at depth %d for key ", d); print_key(key,d);
						}
						s = s->child;
				// if current key didn't match, move to the next one on this level
				} else {
						s = s->next;
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

uint16_t
calculate_csum16(const void* buf, uint16_t length)
{
    uint16_t value16 = 0;
    return value16;
} 
 
void
table_setdefault(lookup_table_t* t, uint8_t* value)
{
		if( t->default_val ) free(t->default_val);
    t->default_val = value;
}

int
compare_keys(uint8_t* key1, uint8_t* key2, uint8_t length)
{
		int match = 1;
		int i = 0;
		while(i<length && match){
				if (key1[i] != key2[i]){
						match = 0;
				}
				i++;
		}
		return match;
}

void
print_key(uint8_t* key, int n)
{
		int i;
		for (i=0;i<n;i++){
				//printf("%02x ", key[i]);
				printf("%d ", key[i]);
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

void
print_mac48(uint8_t key[6])
{
		printf("%02x:%02x:%02x:%02x:%02x:%02x \n", key[0],key[1],key[2],key[3],key[4],key[5]);
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

