#ifndef _IRDA_RAW_H
#define _IRDA_RAW_H

typedef struct irda_raw_event {
	int pol; /* ¼«ÐÔ */
	int duration;  /* Âö³å¿í¶È, us */
} irda_raw_event, *p_irda_raw_event;

#endif /* _IRDA_RAW_H */

