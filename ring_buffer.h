/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef ring_buffer_H
#define ring_buffer_H
    
    #include "project.h"
    #include <stdint.h>

    typedef struct {
        volatile uint8_t * const buffer;
        int head;
        int tail;
        const int maxlen;
    } circ_bbuf_t;

    #define CIRC_BBUF_DEF(x,y)                \
        uint8 x##_data_space[y];              \
        circ_bbuf_t x = {                     \
            .buffer = x##_data_space,         \
            .head = 0,                        \
            .tail = 0,                        \
            .maxlen = y                       \
        }

    /*
     * Method: circ_buf_pop
     * Returns:
     *  0 - Success
     * -1 - Empty
     */
    int circ_bbuf_pop(circ_bbuf_t *c, uint8_t *data);

    /*
     * Method: circ_buf_push
     * Returns:
     *  0 - Success
     * -1 - Out of space
     */
    int circ_bbuf_push(circ_bbuf_t *c, uint8_t data);

#endif
/* [] END OF FILE */
