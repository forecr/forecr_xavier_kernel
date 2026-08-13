/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GSP_CTRL_FIFO_H
#define GSP_CTRL_FIFO_H

#define GSP_CTRL_FIFO_QUEUE_SIZE                65536U
/* (65536 - sizeof(control block)) / sizeof(each message block) */
#define GSP_CTRL_FIFO_QUEUE_ENTRIES             1022

/*
 * following indicates the types of queues
 */
enum queue_type {
    CONTROL_QUEUE,
    RESPONSE_QUEUE,
    EVENT_QUEUE
};

struct nvgpu_gsp_ctrl_fifo_info {
    /*
     * Start Address of control fifo queue
     * fifo_addr_lo ->32 bit starting from LSB
     * fifo_addr_hi ->32 bit from MSB
     */
    u32 fifo_addr_lo;
    u32 fifo_addr_hi;
    /*
     * Size of the control fifo queue
     */
    u32 queue_size;
    /* total number of messages present in the queue */
    u32 queue_entries;
    /*
     * queue type indicates the type of queue it is
     * it can either be a control queue, response queue
     * or event queue
     */
    u32 qtype;
};
#endif/* GSP_CTRL_FIFO_H */