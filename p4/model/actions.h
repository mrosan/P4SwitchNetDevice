 #ifndef __ACTION_INFO_GENERATED_H__// sugar@14
 #define __ACTION_INFO_GENERATED_H__// sugar@15
 
  #include "p4-interface.h"

 #define FIELD(name, length) uint8_t name[(length + 7) / 8];

	enum actions {
		action_on_miss,
		action_fib_hit_nexthop,
		action_rewrite_src_mac,
	};
	
	struct action_fib_hit_nexthop_params {
		FIELD(dmac, 48);
		FIELD(port, 9);
	};
	
	struct action_rewrite_src_mac_params {
		FIELD(smac, 48);
	};
 
 
	struct ipv4_fib_lpm_action {
		 int action_id;
		 union{
				struct action_fib_hit_nexthop_params fib_hit_nexthop_params;
		 };
	};

	struct sendout_action {
		 int action_id;
		 union {
				struct action_rewrite_src_mac_params rewrite_src_mac_params;
		 };
	};
 
 
 void apply_table_ipv4_fib_lpm(packet_descriptor_t *pd, lookup_table_t** tables);// sugar@49
 void action_code_on_miss(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code_fib_hit_nexthop(packet_descriptor_t *pd, lookup_table_t **tables, struct action_fib_hit_nexthop_params);// sugar@52
 void apply_table_sendout(packet_descriptor_t *pd, lookup_table_t** tables);// sugar@49
 void action_code_on_miss(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54
 void action_code_rewrite_src_mac(packet_descriptor_t *pd, lookup_table_t **tables, struct action_rewrite_src_mac_params);// sugar@52
 #endif// sugar@56
