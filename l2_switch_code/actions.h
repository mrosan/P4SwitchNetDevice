 #ifndef __ACTION_INFO_GENERATED_H__// sugar@14
 #define __ACTION_INFO_GENERATED_H__// sugar@15
 
 #include "p4-interface.h" //there were no includes originally

 #define FIELD(name, length) uint8_t name[(length + 7) / 8];// sugar@18

 enum actions {// sugar@21
 action_mac_learn,// sugar@27
 action__nop,// sugar@27
 action_forward,// sugar@27
 action_bcast,// sugar@27
 };// sugar@28
 
 struct action_forward_params {// sugar@33
 		FIELD(port, 9);// sugar@35
 };// sugar@36	
 
 struct smac_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
     };// sugar@45
 };// sugar@46
 
 struct dmac_action {// sugar@39
     int action_id;// sugar@40
     union {// sugar@41
 				struct action_forward_params forward_params;// sugar@44
     };// sugar@45
 };// sugar@46
 
 void apply_table_smac(packet_descriptor_t *pd, lookup_table_t** tables);// sugar@49
 void action_code_mac_learn(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void apply_table_dmac(packet_descriptor_t *pd, lookup_table_t** tables);// sugar@49
 void action_code_forward(packet_descriptor_t *pd, lookup_table_t **tables, struct action_forward_params);// sugar@52
 void action_code_bcast(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 #endif// sugar@56
