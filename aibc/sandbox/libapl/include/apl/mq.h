/**
 *
 */
#ifndef APL__MQ_H
#define APL__MQ_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE


int apl_mq_open();
int apl_mq_close();
int apl_mq_unlink();
int apl_mq_send();
int apl_mq_receive();
int apl_mq_timed_send();
int apl_mq_timed_receive();
int apl_mq_set_attr();
int apl_mq_get_attr();

APL_END_C_DECLARE


#endif // APL__MQ_H

