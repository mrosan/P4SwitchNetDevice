 #include <stdlib.h>// sugar@20
 #include <string.h>// sugar@21
 #include "p4-interface.h"// sugar@22
 #include "actions.h"// sugar@23
 
 extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@25

 extern void increase_counter (int counterid, int index);// sugar@27

 void apply_table_ipv4_fib_lpm(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@30
 void apply_table_sendout(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@30

 uint8_t reverse_buffer[4];// sugar@34
 void table_ipv4_fib_lpm_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@43
printf("extracting ip header\n"); //ADDED LINE
	 EXTRACT_INT32_BITS(pd, field_instance_ipv4_dstAddr, *(uint32_t*)key)// sugar@49
printf("extracted ip header\n"); //ADDED LINE
	 key += sizeof(uint32_t);// sugar@50
	 key -= 4;// sugar@58
	 int c, d;// sugar@59
	 for(c = 3, d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@60
	 for(c = 0; c < 4; c++) *(key+c) = *(reverse_buffer+c);// sugar@61
 }// sugar@62

 void table_sendout_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@43
	 EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_egress_port, *(uint32_t*)key)// sugar@49
	 key += sizeof(uint32_t);// sugar@50
 }// sugar@62

 void apply_table_ipv4_fib_lpm(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@68
 {// sugar@69
     debug("  :::: EXECUTING TABLE ipv4_fib_lpm\n");
     uint8_t* key[4];
     table_ipv4_fib_lpm_key(pd, (uint8_t*)key);

     uint8_t* value = lpm_lookup(tables[TABLE_ipv4_fib_lpm], (uint8_t*)key);
     struct ipv4_fib_lpm_action* res = (struct ipv4_fib_lpm_action*)value;
     int index; (void)index;
     if(res != NULL) {
       index = *(int*)(value+sizeof(struct ipv4_fib_lpm_action));
     }
     if(res == NULL) {
       debug("    :: NO RESULT, NO DEFAULT ACTION.\n");
     } else {
       switch (res->action_id) {
         case action_on_miss:
           debug("    :: EXECUTING ACTION on_miss...\n");
           action_code_on_miss(pd, tables);
           break;
         case action_fib_hit_nexthop:
           debug("    :: EXECUTING ACTION fib_hit_nexthop...\n");
           action_code_fib_hit_nexthop(pd, tables, res->fib_hit_nexthop_params);
           break;
       }
     }
     if (res != NULL) {
       switch (res->action_id) {
         case action_on_miss:
           return apply_table_sendout(pd, tables);
           break;
         case action_fib_hit_nexthop:
           return apply_table_sendout(pd, tables);
           break;
       }
     } else {// sugar@117
       debug("    :: IGNORING PACKET.\n");// sugar@118
       return;// sugar@119
     }// sugar@120
 }// sugar@121

 void apply_table_sendout(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@68
 {
     debug("  :::: EXECUTING TABLE sendout\n");
     uint8_t* key[2];
     table_sendout_key(pd, (uint8_t*)key);
     uint8_t* value = exact_lookup(tables[TABLE_sendout], (uint8_t*)key);
     struct sendout_action* res = (struct sendout_action*)value;
     int index; (void)index;
     if(res != NULL) {
       index = *(int*)(value+sizeof(struct sendout_action));
     }
     if(res == NULL) {
       debug("    :: NO RESULT, NO DEFAULT ACTION.\n");
     } else {
       switch (res->action_id) {
         case action_on_miss:
           debug("    :: EXECUTING ACTION on_miss...\n");
           action_code_on_miss(pd, tables);
           break;
         case action_rewrite_src_mac:
           debug("    :: EXECUTING ACTION rewrite_src_mac...\n");
           action_code_rewrite_src_mac(pd, tables, res->rewrite_src_mac_params);
           break;
       }
     }
     if (res != NULL) {// sugar@110
       switch (res->action_id) {// sugar@111
         case action_on_miss:// sugar@113
           // sugar@114
           break;// sugar@115
         case action_rewrite_src_mac:// sugar@113
           // sugar@114
           break;// sugar@115
       }// sugar@116
     } else {// sugar@117
       debug("    :: IGNORING PACKET.\n");// sugar@118
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

 uint32_t calculate_ipv4_checksum(packet_descriptor_t* pd) {// sugar@134
   uint32_t res = 0;// sugar@135
   void* payload_ptr;// sugar@136
   uint8_t* buf = malloc((144) / 8);// sugar@153
   memset(buf, 0, (144) / 8);// sugar@154
   if(pd->headers[header_instance_ipv4].pointer != NULL) {// sugar@209
     memcpy(buf + ((0) / 8), field_desc(pd, field_instance_ipv4_versionIhl).byte_addr, ((80) / 8));// sugar@213
     memcpy(buf + ((80) / 8), field_desc(pd, field_instance_ipv4_srcAddr).byte_addr, ((64) / 8));// sugar@213
   }// sugar@216
   res = csum16_add(res, calculate_csum16(buf, (144) / 8));// sugar@219
   res = (res == 0xffff) ? res : ((~res) & 0xffff);// sugar@220
   free(buf);// sugar@224
   return res & 0xffff;// sugar@225
 }// sugar@226

void reset_headers(packet_descriptor_t* packet_desc) {// sugar@229
		memset(packet_desc->headers[header_instance_standard_metadata].pointer, 0, header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t));// sugar@233
		packet_desc->headers[header_instance_ethernet].pointer = NULL;// sugar@235
		packet_desc->headers[header_instance_ipv4].pointer = NULL;// sugar@235
}// sugar@236
 void init_headers(packet_descriptor_t* packet_desc) {// sugar@237
		packet_desc->headers[header_instance_standard_metadata] = (header_descriptor_t) { .type = header_instance_standard_metadata, .length = header_info(header_instance_standard_metadata).bytewidth,// sugar@241
				                         .pointer = malloc(header_info(header_instance_standard_metadata).bytewidth * sizeof(uint8_t)),// sugar@242
				                         .var_width_field_bitwidth = 0 };// sugar@243
		packet_desc->headers[header_instance_ethernet] = (header_descriptor_t) { .type = header_instance_ethernet, .length = header_info(header_instance_ethernet).bytewidth, .pointer = NULL,// sugar@245
				                         .var_width_field_bitwidth = 0 };// sugar@246
		packet_desc->headers[header_instance_ipv4] = (header_descriptor_t) { .type = header_instance_ipv4, .length = header_info(header_instance_ipv4).bytewidth, .pointer = NULL,// sugar@245
				                         .var_width_field_bitwidth = 0 };// sugar@246
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
 if(pd->fields.attr_field_instance_ipv4_ttl == MODIFIED) {// sugar@280
     value32 = pd->fields.field_instance_ipv4_ttl;// sugar@281
     MODIFY_INT32_INT32_AUTO(pd, field_instance_ipv4_ttl, value32)// sugar@282
 }// sugar@283

 if(pd->headers[header_instance_ipv4].pointer != NULL)// sugar@289
 {// sugar@292
     value32 = calculate_ipv4_checksum(pd);// sugar@293
     MODIFY_INT32_INT32_BITS(pd, field_instance_ipv4_hdrChecksum, value32);// sugar@294
 }// sugar@295
 }// sugar@296


 int verify_packet(packet_descriptor_t* pd) {// sugar@300
   uint32_t value32;// sugar@301
   if(pd->headers[header_instance_ipv4].pointer != NULL)// sugar@306
   {// sugar@309
     EXTRACT_INT32_BITS(pd, field_instance_ipv4_hdrChecksum, value32);// sugar@310
     if(value32 != calculate_ipv4_checksum(pd)) {// sugar@311
       debug("       Checksum verification on field 'ipv4.hdrChecksum' by 'ipv4_checksum': FAILED\n");// sugar@312
       return 1;// sugar@313
     }// sugar@314
     else debug("       Checksum verification on field 'ipv4.hdrChecksum' by 'ipv4_checksum': SUCCESSFUL\n");// sugar@315
   }// sugar@316
   return 0;// sugar@317
 }// sugar@318

 
 void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@322
 {// sugar@323
     int value32;// sugar@324
     EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@325
     debug("### HANDLING PACKET ARRIVING AT PORT %" PRIu32 "...\n", value32);// sugar@326
     reset_headers(pd);// sugar@327
     parse_packet(pd, tables);// sugar@328
     update_packet(pd);// sugar@329
 }// sugar@330
