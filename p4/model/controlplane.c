 #include "p4-interface.h"// sugar@22
 #include "actions.h"// sugar@23
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28

 extern void table_ipv4_fib_lpm_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_sendout_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31

 uint8_t reverse_buffer[4];

 void
 ipv4_fib_lpm_add(
		 uint8_t field_instance_ipv4_dstAddr[4],
		 uint8_t field_instance_ipv4_dstAddr_prefix_length,
		 struct ipv4_fib_lpm_action action)
 {
     uint8_t key[4];
		 memcpy(key+0, field_instance_ipv4_dstAddr, 4);
		 uint8_t prefix_length = 0;
		 prefix_length += field_instance_ipv4_dstAddr_prefix_length;
		 int c, d;
		 for(c = 3, d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);
		 for(c = 0; c < 4; c++) *(key+c) = *(reverse_buffer+c);
		 lpm_add_promote(TABLE_ipv4_fib_lpm, (uint8_t*)key, prefix_length, (uint8_t*)&action);
 }

 void// sugar@77
 ipv4_fib_lpm_setdefault(struct ipv4_fib_lpm_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_ipv4_fib_lpm, (uint8_t*)&action);// sugar@80
 }// sugar@81
 void
 sendout_add(
						 uint8_t field_instance_standard_metadata_egress_port[2],
						 struct sendout_action action)
 {
      uint8_t key[2];
 			memcpy(key+0, field_instance_standard_metadata_egress_port, 2);
 			exact_add_promote(TABLE_sendout, (uint8_t*)key, (uint8_t*)&action);
 }

 void// sugar@77
 sendout_setdefault(struct sendout_action action)// sugar@78
 {// sugar@79
     table_setdefault_promote(TABLE_sendout, (uint8_t*)&action);// sugar@80
 }// sugar@81

 void
 ipv4_fib_lpm_add_table_entry(struct p4_ctrl_msg* ctrl_m) {
		 uint8_t* field_instance_ipv4_dstAddr = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->bitmap);
		 uint16_t field_instance_ipv4_dstAddr_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->prefix_length;
		 if(strcmp("on_miss", ctrl_m->action_name)==0) {// sugar@95
				 struct ipv4_fib_lpm_action action;// sugar@96
				 action.action_id = action_on_miss;// sugar@97
				 printf("Reply from the control plane arrived.\n");// sugar@101
				 printf("Adding new entry to ipv4_fib_lpm with action on_miss\n");// sugar@102
				 ipv4_fib_lpm_add(
						 field_instance_ipv4_dstAddr,
						 field_instance_ipv4_dstAddr_prefix_length,
						 action);

		 } else if(strcmp("fib_hit_nexthop", ctrl_m->action_name)==0) {// sugar@95
				 struct ipv4_fib_lpm_action action;// sugar@96
				 action.action_id = action_fib_hit_nexthop;// sugar@97
				 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@99
				 memcpy(action.fib_hit_nexthop_params.dmac, dmac, 6);// sugar@100
				 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@99
				 memcpy(action.fib_hit_nexthop_params.port, port, 2);// sugar@100
				 printf("Reply from the control plane arrived.\n");// sugar@101
				 printf("Adding new entry to ipv4_fib_lpm with action fib_hit_nexthop\n");// sugar@102
				 ipv4_fib_lpm_add(
						 field_instance_ipv4_dstAddr,
						 field_instance_ipv4_dstAddr_prefix_length,
						 action);

		 } else  printf("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
}

 void
 sendout_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@86
 uint8_t* field_instance_standard_metadata_egress_port = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 if(strcmp("on_miss", ctrl_m->action_name)==0) {// sugar@95
     struct sendout_action action;// sugar@96
     action.action_id = action_on_miss;// sugar@97
     printf("Reply from the control plane arrived.\n");// sugar@101
     printf("Adding new entry to sendout with action on_miss\n");// sugar@102
     sendout_add( field_instance_standard_metadata_egress_port, action);
 } else// sugar@110
 if(strcmp("rewrite_src_mac", ctrl_m->action_name)==0) {// sugar@95
     struct sendout_action action;// sugar@96
     action.action_id = action_rewrite_src_mac;// sugar@97
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@99
 memcpy(action.rewrite_src_mac_params.smac, smac, 6);// sugar@100
     printf("Reply from the control plane arrived.\n");// sugar@101
     printf("Adding new entry to sendout with action rewrite_src_mac\n");// sugar@102
     sendout_add( field_instance_standard_metadata_egress_port, action);// sugar@109
 } else// sugar@110
 printf("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@111
 }

 void// sugar@115
 ipv4_fib_lpm_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 printf("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("on_miss", ctrl_m->action_name)==0) {// sugar@119
     struct ipv4_fib_lpm_action action;// sugar@120
     action.action_id = action_on_miss;// sugar@121
     printf("Message from the control plane arrived.\n");// sugar@125
     printf("Set default action for ipv4_fib_lpm with action on_miss\n");// sugar@126
     ipv4_fib_lpm_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("fib_hit_nexthop", ctrl_m->action_name)==0) {// sugar@119
     struct ipv4_fib_lpm_action action;// sugar@120
     action.action_id = action_fib_hit_nexthop;// sugar@121
 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@123
 memcpy(action.fib_hit_nexthop_params.dmac, dmac, 6);// sugar@124
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@123
 memcpy(action.fib_hit_nexthop_params.port, port, 2);// sugar@124
     printf("Message from the control plane arrived.\n");// sugar@125
     printf("Set default action for ipv4_fib_lpm with action fib_hit_nexthop\n");// sugar@126
     ipv4_fib_lpm_setdefault( action );// sugar@127
 } else// sugar@128
 printf("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 void// sugar@115
 sendout_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@116
 printf("Action name: %s\n", ctrl_m->action_name);// sugar@117
 if(strcmp("on_miss", ctrl_m->action_name)==0) {// sugar@119
     struct sendout_action action;// sugar@120
     action.action_id = action_on_miss;// sugar@121
     printf("Message from the control plane arrived.\n");// sugar@125
     printf("Set default action for sendout with action on_miss\n");// sugar@126
     sendout_setdefault( action );// sugar@127
 } else// sugar@128
 if(strcmp("rewrite_src_mac", ctrl_m->action_name)==0) {// sugar@119
     struct sendout_action action;// sugar@120
     action.action_id = action_rewrite_src_mac;// sugar@121
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@123
 memcpy(action.rewrite_src_mac_params.smac, smac, 6);// sugar@124
     printf("Message from the control plane arrived.\n");// sugar@125
     printf("Set default action for sendout with action rewrite_src_mac\n");// sugar@126
     sendout_setdefault( action );// sugar@127
 } else// sugar@128
 printf("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@129
 }// sugar@130
 
 
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@133
		printf("MSG from controller %d %s\n", ctrl_m->type, ctrl_m->table_name);// sugar@134
		if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@135
				if (strcmp("ipv4_fib_lpm", ctrl_m->table_name) == 0) ipv4_fib_lpm_add_table_entry(ctrl_m);// sugar@138
				else if (strcmp("sendout", ctrl_m->table_name) == 0) sendout_add_table_entry(ctrl_m);// sugar@138
				else printf("Table add entry: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@140
		}
		else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@142
				if (strcmp("ipv4_fib_lpm", ctrl_m->table_name) == 0) ipv4_fib_lpm_set_default_table_action(ctrl_m);// sugar@145
				else if (strcmp("sendout", ctrl_m->table_name) == 0) sendout_set_default_table_action(ctrl_m);// sugar@145
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
 */// sugar@170
 }// sugar@171
