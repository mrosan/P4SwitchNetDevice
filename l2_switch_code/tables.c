// #include "dataplane.h"// sugar@18
#include "p4-interface.h"
 #include "actions.h"// sugar@19
 #include "data_plane_data.h"// sugar@20

 lookup_table_t table_config[NB_TABLES] = {// sugar@22
 {// sugar@25
  .name= "smac",// sugar@26
  .id = TABLE_smac,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 6,// sugar@29
  .val_size = sizeof(struct smac_action),// sugar@30
  .min_size = 0, //512,// sugar@31
  .max_size = 255 //512// sugar@32
 },// sugar@33
 {// sugar@25
  .name= "dmac",// sugar@26
  .id = TABLE_dmac,// sugar@27
  .type = LOOKUP_EXACT,// sugar@28
  .key_size = 6,// sugar@29
  .val_size = sizeof(struct dmac_action),// sugar@30
  .min_size = 0, //512,// sugar@31
  .max_size = 255 //512// sugar@32
 },// sugar@33
 };// sugar@34
 counter_t counter_config[NB_COUNTERS] = {// sugar@36
 };// sugar@51
 p4_register_t register_config[NB_REGISTERS] = {// sugar@53
 };// sugar@66
