#ifndef __CIRCLE_BUFFER_H__
#define __CIRCLE_BUFFER_H__

#include "irda_raw.h"

int ir_event_put(p_irda_raw_event pd);
int ir_event_get(p_irda_raw_event pd);
int ir_event_get_timeout(p_irda_raw_event pd, int timeout_us);

#endif