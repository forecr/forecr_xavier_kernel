/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

struct nvgpu_gsp_ctrl_fifo_close {
    u32 qtype;
    bool is_abrupt_close;
};
#endif/* GSP_CTRL_FIFO_H */