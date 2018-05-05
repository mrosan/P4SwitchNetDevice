 #include "p4-interface.h"// sugar@20
 #include "actions.h"// sugar@21
 #include <unistd.h>// sugar@22
 #include <arpa/inet.h>// sugar@23

 extern backend bg;// sugar@25

  void action_code_no_op(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

  void action_code_drop(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451


void action_code_on_miss(packet_descriptor_t* pd, lookup_table_t** tables ) 
{
   uint32_t value32, res32, mask32;
   (void)value32; (void)res32; (void)mask32;
}

void action_code_fib_hit_nexthop(packet_descriptor_t* pd, lookup_table_t** tables , struct action_fib_hit_nexthop_params parameters) {

		//printf("----------------> Inside action_code_fib_hit_nexthop\n");

		uint32_t value32, res32, mask32;
		(void)value32; (void)res32; (void)mask32;
		
		if(6 < field_desc(pd, field_instance_ethernet_dstAddr).bytewidth) {
				 MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet_dstAddr, parameters.dmac, 6);
		} else {
				 MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet_dstAddr, parameters.dmac + 
								(6 - field_desc(pd, field_instance_ethernet_dstAddr).bytewidth), 
								field_desc(pd, field_instance_ethernet_dstAddr).bytewidth)
		}

		MODIFY_INT32_BYTEBUF(pd, field_instance_standard_metadata_egress_port, parameters.port, 2)

		value32 = -1;
		res32 = pd->fields.field_instance_ipv4_ttl;
		pd->fields.attr_field_instance_ipv4_ttl = MODIFIED;

		value32 += res32;
		pd->fields.field_instance_ipv4_ttl = value32;
		pd->fields.attr_field_instance_ipv4_ttl = MODIFIED;

}// sugar@451

void action_code_rewrite_src_mac(packet_descriptor_t* pd, lookup_table_t** tables , struct action_rewrite_src_mac_params parameters) 
{
		uint32_t value32, res32, mask32;
		(void)value32; (void)res32; (void)mask32;
		if(6 < field_desc(pd, field_instance_ethernet_srcAddr).bytewidth) 
		{
				MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet_srcAddr, parameters.smac, 6);
		} else 
		{
				MODIFY_BYTEBUF_BYTEBUF(pd, field_instance_ethernet_srcAddr, parameters.smac + 
																		(6 - field_desc(pd, field_instance_ethernet_srcAddr).bytewidth), 
																							field_desc(pd, field_instance_ethernet_srcAddr).bytewidth)
		}

}

