#include "p4-interface.h"
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

  void action_code__nop(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
 }// sugar@451

void action_code_mac_learn(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
     uint32_t value32, res32, mask32;// sugar@440
     (void)value32; (void)res32; (void)mask32;// sugar@441
   
    struct type_field_list fields;// sugar@369
    fields.fields_quantity = 2;// sugar@371
    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@372
    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@373
    fields.field_offsets[0] = (uint8_t*) field_desc(pd, field_instance_ethernet_srcAddr).byte_addr;// sugar@377
    fields.field_widths[0]  =            field_desc(pd, field_instance_ethernet_srcAddr).bitwidth;// sugar@378
    fields.field_offsets[1] = (uint8_t*) field_desc(pd, field_instance_standard_metadata_ingress_port).byte_addr;// sugar@377
    fields.field_widths[1]  =            field_desc(pd, field_instance_standard_metadata_ingress_port).bitwidth;// sugar@378

    generate_digest(bg,"mac_learn_digest",0,&fields); sleep(1);// sugar@384

}// sugar@451

void action_code_forward(packet_descriptor_t* pd, lookup_table_t** tables , struct action_forward_params parameters) {// sugar@439
   uint32_t value32, res32, mask32;// sugar@440
   (void)value32; (void)res32; (void)mask32;// sugar@441
	 MODIFY_INT32_BYTEBUF(pd, field_instance_standard_metadata_egress_port, parameters.port, 2)// sugar@187
}// sugar@451

void action_code_bcast(packet_descriptor_t* pd, lookup_table_t** tables ) {// sugar@439
   uint32_t value32, res32, mask32;// sugar@440
   (void)value32; (void)res32; (void)mask32;// sugar@441
	 value32 = 100;// sugar@144
	 MODIFY_INT32_INT32_AUTO(pd, field_instance_standard_metadata_egress_port, value32)// sugar@41
}// sugar@451

