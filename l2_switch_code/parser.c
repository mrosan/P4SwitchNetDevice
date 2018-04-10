#include "p4-interface.h"
 #include "actions.h" // apply_table_* and action_code_*// sugar@63

 extern int verify_packet(packet_descriptor_t* pd);// sugar@65

 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@67
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@68
 
 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@72
 printf("p4_pe_header_too_short \n"); //MODDED
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_default\n"); //MODDED
 }// sugar@77
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_checksum\n"); //MODDED
 }// sugar@77
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_unhandled_select\n"); //MODDED
 }// sugar@77
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_index_out_of_bounds\n"); //MODDED
 }// sugar@77
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_header_too_long\n"); //MODDED
 }// sugar@77
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 printf(" p4_pe_out_of_packet\n"); //MODDED
 }// sugar@77
 static void// sugar@81
 extract_header_standard_metadata(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_standard_metadata].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_ethernet(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_ethernet].pointer = buf;// sugar@83
 }// sugar@90
 
 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94

 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122
 {// sugar@123
     uint32_t value32;// sugar@124
     (void)value32;// sugar@125
  return parse_state_parse_ethernet(pd, buf, tables);// sugar@21
// sugar@154
 }// sugar@189
 
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122
 {// sugar@123
     uint32_t value32;// sugar@124
     (void)value32;// sugar@125
     extract_header_ethernet(buf, pd);// sugar@130
     buf += pd->headers[header_instance_ethernet].length;// sugar@131
			{// sugar@25
			 if(verify_packet(pd)) {
			 			p4_pe_checksum(pd); // sugar@26 //always false, so p4_pe_checksum wont drop the packet

			 			}
			 return apply_table_smac(pd, tables);// sugar@27
		 	}// sugar@28
// sugar@154
 }// sugar@189
 
 void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@192
     parse_state_start(pd, pd->data, tables);// sugar@193
     
 }// sugar@194
