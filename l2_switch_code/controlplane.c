#include "p4-interface.h"
 #include "actions.h"// sugar@23
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28

 extern void table_smac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c
 extern void table_dmac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c

 uint8_t reverse_buffer[6];// sugar@35
 
 
 //#### ####
		 void smac_add( uint8_t field_instance_ethernet_srcAddr[6], struct smac_action action)
		 {// sugar@50
				  uint8_t key[6];// sugar@51
		 			memcpy(key+0, field_instance_ethernet_srcAddr, 6);// sugar@56
		 			exact_add_promote(TABLE_smac, (uint8_t*)key, (uint8_t*)&action);// sugar@74
		 }// sugar@75

		 void// sugar@77
		 smac_setdefault(struct smac_action action)// sugar@78
		 {// sugar@79
				 table_setdefault_promote(TABLE_smac, (uint8_t*)&action);// sugar@80
		 }// sugar@81
		 
		 void// sugar@39
		 dmac_add( uint8_t field_instance_ethernet_dstAddr[6], struct dmac_action action)// sugar@49
		 {// sugar@50
				  uint8_t key[6];// sugar@51
		 			memcpy(key+0, field_instance_ethernet_dstAddr, 6);// sugar@56
		 			exact_add_promote(TABLE_dmac, (uint8_t*)key, (uint8_t*)&action);// sugar@74
		 }// sugar@75

		 void// sugar@77
		 dmac_setdefault(struct dmac_action action)// sugar@78
		 {// sugar@79
				 table_setdefault_promote(TABLE_dmac, (uint8_t*)&action);// sugar@80
		 }// sugar@81
 //#### #### #### ####
 
 
 
 
 void// sugar@85
 smac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
	 uint8_t* field_instance_ethernet_srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
		if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@95
				 struct smac_action action;// sugar@96
				 action.action_id = action_mac_learn;// sugar@97
				 printf("Reply from the control plane arrived.\n");// sugar@101
				 printf("Adding new entry to smac with action mac_learn\n");// sugar@102
				 smac_add( field_instance_ethernet_srcAddr, action);// sugar@109
				 
		} else if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@95
				 struct smac_action action;// sugar@96
				 action.action_id = action__nop;// sugar@97
				 printf("Reply from the control plane arrived.\n");// sugar@101
				 printf("Adding new entry to smac with action _nop\n");// sugar@102
				 smac_add( field_instance_ethernet_srcAddr, action);// sugar@109
				 
		 } else printf("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 
 void// sugar@85
 dmac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 		uint8_t* field_instance_ethernet_dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 		if(strcmp("forward", ctrl_m->action_name)==0) 
 		{// sugar@95
     	struct dmac_action action;// sugar@96
     	action.action_id = action_forward;// sugar@97
 			uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;
 			memcpy(action.forward_params.port, port, 2);// sugar@100
     	printf("Reply from the control plane arrived.\n");// sugar@101
     	printf("Adding new entry to dmac with action forward\n");// sugar@102
     	dmac_add( field_instance_ethernet_dstAddr, action);// sugar@109
     	
 		} else if(strcmp("bcast", ctrl_m->action_name)==0) 
 		{// sugar@95
			struct dmac_action action;// sugar@96
			action.action_id = action_bcast;// sugar@97
			printf("Reply from the control plane arrived.\n");// sugar@101
			printf("Adding new entry to dmac with action bcast\n");// sugar@102
			dmac_add( field_instance_ethernet_dstAddr, action);// sugar@109
			
 		} else printf("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }// sugar@112
 
 void// sugar@115
 smac_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 		printf("Action name: %s\n", ctrl_m->action_name);// sugar@117
 		if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@119
				 struct smac_action action;// sugar@120
				 action.action_id = action_mac_learn;// sugar@121
				 printf("Message from the control plane arrived.\n");// sugar@125
				 printf("Set default action for smac with action mac_learn\n");// sugar@126
				 smac_setdefault( action );// sugar@127
		} else if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@119
				 struct smac_action action;// sugar@120
				 action.action_id = action__nop;// sugar@121
				 printf("Message from the control plane arrived.\n");// sugar@125
				 printf("Set default action for smac with action _nop\n");// sugar@126
				 smac_setdefault( action );// sugar@127
		} else printf("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 
 void// sugar@115
 dmac_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
	 printf("Action name: %s\n", ctrl_m->action_name);// sugar@117
	 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@119
			struct dmac_action action;// sugar@120
			action.action_id = action_forward;// sugar@121
			uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@123
			memcpy(action.forward_params.port, port, 2);// sugar@124
			printf("Message from the control plane arrived.\n");// sugar@125
			printf("Set default action for dmac with action forward\n");// sugar@126
			dmac_setdefault( action );// sugar@127
	 } else if(strcmp("bcast", ctrl_m->action_name)==0) {// sugar@119
			struct dmac_action action;// sugar@120
			action.action_id = action_bcast;// sugar@121
			printf("Message from the control plane arrived.\n");// sugar@125
			printf("Set default action for dmac with action bcast\n");// sugar@126
			dmac_setdefault( action );// sugar@127
	 } else printf("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@133
     printf("MSG from controller %d %s\n", ctrl_m->type, ctrl_m->table_name);// sugar@134
     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@135
				 if (strcmp("smac", ctrl_m->table_name) == 0)// sugar@137
						 smac_add_table_entry(ctrl_m);// sugar@138
				 else// sugar@139
				 if (strcmp("dmac", ctrl_m->table_name) == 0)// sugar@137
						 dmac_add_table_entry(ctrl_m);// sugar@138
				 else printf("Table add entry: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@140
			} else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@142
				 if (strcmp("smac", ctrl_m->table_name) == 0)// sugar@144
						 smac_set_default_table_action(ctrl_m);// sugar@145
					else if (strcmp("dmac", ctrl_m->table_name) == 0)// sugar@144
						 dmac_set_default_table_action(ctrl_m);// sugar@145
				 else printf("Table setdefault: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@147
			}// sugar@148
 }// sugar@149
 
 
 
 
 
 
 
 backend bg;// sugar@153
 void init_control_plane()// sugar@154
 {// sugar@155
     printf("Creating control plane connection...\n");// sugar@156
     bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);// sugar@157
     launch_backend(bg);// sugar@158
 /*// sugar@159
 struct smac_action action;// sugar@161
 action.action_id = action_mac_learn;// sugar@162
 smac_setdefault(action);// sugar@163
 printf("smac setdefault\n");// sugar@164
 struct dmac_action action2;// sugar@166
 action2.action_id = action_bcast;// sugar@167
 dmac_setdefault(action2);// sugar@168
 printf("dmac setdefault\n");// sugar@169
 */// sugar@170
 }// sugar@171
