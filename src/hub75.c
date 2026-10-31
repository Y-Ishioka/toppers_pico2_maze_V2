/*
 * Copyright (C) 2022-2026 by Y.Ishioka
 */

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#define PICO_NO_HARDWARE 0

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hub75.pio.h"
#include "app_param.h"

#include "hardware/adc.h"

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define ROWSEL_N_PINS 5
#define CLK_PIN 11
#define STROBE_PIN 12
#define OEN_PIN 13


static inline uint32_t gamma_correct_888_888(uint8_t *pix) {
    return ((uint32_t)*(pix) << 16) | ((uint32_t)*(pix+1) << 8) | ((uint32_t)*(pix+2));
}


extern  unsigned char  image_buffer1[];
extern  unsigned char  image_buffer2[];
extern  unsigned char  image_buffer3[];
extern  unsigned char  image_buffer4[];


uint32_t gc_row[ 2 ][ WIDTH * 4 ];  /* ( upper panel width + lower panel width ) * num_panel */

PIO pio;
uint sm_data;
uint sm_row;
uint data_prog_offs;
uint row_prog_offs;

uint8_t   *img888_1, *img888_2, *img888_3, *img888_4;

#define  IMAGE_BUFFER        image_buffer1

int hub75_test_init() {
    pio = pio0;
    sm_data = 0;
    sm_row = 1;

    data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, DATA_BASE_PIN, CLK_PIN);
    hub75_row_program_init(pio, sm_row, row_prog_offs, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);

    img888_1 = (uint8_t *)image_buffer4;
    img888_2 = (uint8_t *)image_buffer2;
    img888_3 = (uint8_t *)image_buffer1;
    img888_4 = (uint8_t *)image_buffer3;
}

int hub75_test_loop() {
    for (int bit = 0; bit < 8; ++bit) {
        for (int rowsel = 0; rowsel < (1 << ROWSEL_N_PINS); ++rowsel) {
            for (int x = 0; x < WIDTH; ++x) {
                gc_row[0][x] = gamma_correct_888_888(&img888_1[(rowsel * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[1][x] = gamma_correct_888_888(&img888_1[(((1u << ROWSEL_N_PINS) + rowsel) * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[0][x+WIDTH] = gamma_correct_888_888(&img888_2[(rowsel * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[1][x+WIDTH] = gamma_correct_888_888(&img888_2[(((1u << ROWSEL_N_PINS) + rowsel) * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[0][x+WIDTH*2] = gamma_correct_888_888(&img888_3[(rowsel * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[1][x+WIDTH*2] = gamma_correct_888_888(&img888_3[(((1u << ROWSEL_N_PINS) + rowsel) * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[0][x+WIDTH*3] = gamma_correct_888_888(&img888_4[(rowsel * WIDTH + x)*DEF_IMG_PIXEL]);
                gc_row[1][x+WIDTH*3] = gamma_correct_888_888(&img888_4[(((1u << ROWSEL_N_PINS) + rowsel) * WIDTH + x)*DEF_IMG_PIXEL]);
            }

            hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
            for (int x = 0 ; x < WIDTH * 4 ; ++x) {
                pio_sm_put_blocking(pio, sm_data, gc_row[0][x]);
                pio_sm_put_blocking(pio, sm_data, gc_row[1][x]);
            }
            // Dummy pixel per lane
            pio_sm_put_blocking(pio, sm_data, 0);
            pio_sm_put_blocking(pio, sm_data, 0);
            // SM is finished when it stalls on empty TX FIFO
            hub75_wait_tx_stall(pio, sm_data);
            // Also check that previous OEn pulse is finished, else things can get out of sequence
            hub75_wait_tx_stall(pio, sm_row);

            // Latch row data, pulse output enable for new row.
            pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 5));
        }
    }

    return 0;
}
