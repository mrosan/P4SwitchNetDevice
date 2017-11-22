 #include <stdlib.h>// sugar@20
 #include <string.h>// sugar@21
#include "p4-interface.h"
 #include "actions.h"// sugar@23
 
 extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@25

 extern void increase_counter (int counterid, int index);// sugar@27

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@30
 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@30

 uint8_t reverse_buffer[6];// sugar@34
 void table_smac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@43
 			EXTRACT_BYTEBUF(pd, field_instance_ethernet_srcAddr, key)// sugar@53
 			//printf("This is the what [table_smac_key] extracted as key: ");//MODDED
      //print_mac48(key);//MODDED
 			key += 6;// sugar@54
 }// sugar@62

 void table_dmac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@43
 			EXTRACT_BYTEBUF(pd, field_instance_ethernet_dstAddr, key)// sugar@53
 			//printf("This is the what [table_dmac_key] extracted as key: ");//MODDED
      //print_mac48(key);//MODDED
 			key += 6;// sugar@54
 }// sugar@62

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@68
 {// sugar@69
     printf("  :::: EXECUTING TABLE smac\n");// sugar@70
     uint8_t* key[6];// sugar@71
     table_smac_key(pd, (uint8_t*)key);// sugar@72
     //printf("----> "); print_mac48(*key);
     //uint8_t key[6] = {0,0,0,0,0,0}; //MODDED

     uint8_t* value = exact_lookup(tables[TABLE_smac], (uint8_t*)key);// sugar@73
     //printf("This is the value [apply_table_smac] received from exact_lookup: %d\n", *value);//MODDED
     struct smac_action* res = (struct smac_action*)value;// sugar@74
     int index; (void)index;// sugar@75
     if(res != NULL) {// sugar@78
       index = *(int*)(value+sizeof(struct smac_action));// sugar@79
     }// sugar@82
     //printf("index == %d\n",index); //MODDED
     if(res == NULL) {// sugar@85
       printf("    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
     } else {// sugar@87
       switch (res->action_id) {// sugar@88
         case action_mac_learn:// sugar@90
           printf("    :: EXECUTING ACTION mac_learn...\n");// sugar@91
           action_code_mac_learn(pd, tables);// sugar@95
           break;// sugar@96
         case action__nop:// sugar@90
           printf("    :: EXECUTING ACTION _nop...\n");// sugar@91
           action_code__nop(pd, tables);// sugar@95
           break;// sugar@96
         default: printf("didnt find match for the action %d, doing nothing\n",res->action_id); break;//MODDED
       }// sugar@97
     }// sugar@98
     
     if (res != NULL) {// sugar@110
       switch (res->action_id) {// sugar@111
         case action_mac_learn:// sugar@113
           return apply_table_dmac(pd, tables);// sugar@114
           break;// sugar@115
         case action__nop:// sugar@113
           return apply_table_dmac(pd, tables);// sugar@114
           break;// sugar@115
         default: printf("didnt find match to the action %d, doing nothing\n",res->action_id); break;//MODDED
       }// sugar@116
     } else {// sugar@117
       printf("    :: IGNORING PACKET.\n");// sugar@118
       return;// sugar@119
     }// sugar@120
 }// sugar@121

 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@68
 {// sugar@69
     printf("  :::: EXECUTING TABLE dmac\n");// sugar@70
     
     uint8_t* key[6];// sugar@71
     table_dmac_key(pd, (uint8_t*)key);// sugar@72
     
     uint8_t* value = exact_lookup(tables[TABLE_dmac], (uint8_t*)key);// sugar@73
     //printf("This is the value [apply_table_dmac] received from exact_lookup: %d\n", *value);//MODDED
     
     struct dmac_action* res = (struct dmac_action*)value;// sugar@74
     int index; (void)index;// sugar@75
     if(res != NULL) {// sugar@78
       index = *(int*)(value+sizeof(struct dmac_action));// sugar@79
     }// sugar@82
     if(res == NULL) {// sugar@85
       printf("    :: NO RESULT, NO DEFAULT ACTION.\n");// sugar@86
     } else {// sugar@87
       switch (res->action_id) {// sugar@88
         case action_forward:// sugar@90
           printf("    :: EXECUTING ACTION forward...\n");// sugar@91
           action_code_forward(pd, tables, res->forward_params);// sugar@93
           break;// sugar@96
         case action_bcast:// sugar@90
           printf("    :: EXECUTING ACTION bcast...\n");// sugar@91
           action_code_bcast(pd, tables);// sugar@95
           break;// sugar@96
           
         default: printf("didnt find match to the action %d, doing nothing\n",res->action_id); break;//MODDED
       }// sugar@97
     }// sugar@98
     if (res != NULL) {// sugar@110
       switch (res->action_id) {// sugar@111
         case action_forward:// sugar@113
           // sugar@114
           break;// sugar@115
         case action_bcast:// sugar@113
           // sugar@114
           break;// sugar@115
         
         default: printf("didnt find match to the action %d, doing nothing\n",res->action_id); break;//MODDED  
       }// sugar@116
     } else {// sugar@117
       printf("    :: IGNORING PACKET.\n");// sugar@118
       return;// sugar@119
     }// sugar@120
 }// sugar@121


 uint16_t csum16_add(uint16_t num1, uint16_t num2) {// sugar@125
     if(num1 == 0) return num2;// sugar@126
     uint32_t tmp_num = num1 + num2;// sugar@127
     while(tmp_num > 0xffff)// sugar@128
         tmp_num = ((tmp_num & 0xffff0000) >> 16) + (tmp_num & 0xffff);// sugar@129
     return (uint16_t)tmp_num;// sugar@130
 }// sugar@131

 void reset_headers(packet_descriptor_t* packet_desc) {// sugar@229
 		memset(packet_desc->headers[header_instance_standard_metadata].pointer, 0, header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t));// sugar@233
 		packet_desc->headers[header_instance_ethernet].pointer = NULL;// sugar@235
 }// sugar@236
 
 void init_headers(packet_descriptor_t* packet_desc) {// sugar@237
 			packet_desc->headers[header_instance_standard_metadata] = (header_descriptor_t) { 
					 .type = header_instance_standard_metadata, 
					 .length = header_info(header_instance_standard_metadata).bytewidth,// sugar@241
				   .pointer = malloc(header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t)),// sugar@242
				   .var_width_field_bitwidth = 0 
		 	};// sugar@243
 
 			packet_desc->headers[header_instance_ethernet] = (header_descriptor_t) { 
 						.type = header_instance_ethernet, 
 						.length = header_info(header_instance_ethernet).bytewidth, 
 						.pointer = NULL,// sugar@245
           	.var_width_field_bitwidth = 0 
      };// sugar@246      
 }// sugar@247


 void init_keyless_tables() {// sugar@255
 }// sugar@263

 void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@265
     init_headers(pd);// sugar@266
     reset_headers(pd);// sugar@267
     init_keyless_tables();// sugar@268
     pd->dropped=0;// sugar@269
 }// sugar@270

 void update_packet(packet_descriptor_t* pd) {// sugar@273
     uint32_t value32, res32;// sugar@274
     (void)value32, (void)res32;// sugar@275

 }// sugar@296


 int verify_packet(packet_descriptor_t* pd) {// sugar@300
   uint32_t value32;// sugar@301
   return 0;// sugar@317
 }// sugar@318

 
 void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@322
 {// sugar@323
     int value32;// sugar@324
     EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@325
     printf("### HANDLING PACKET ARRIVING AT PORT %" PRIu32 "...\n", value32);// sugar@326
     //reset_headers(pd);
     parse_packet(pd, tables);// sugar@328
     update_packet(pd);// sugar@329
 }// sugar@330
