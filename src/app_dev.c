/*
 * Copyright (C) 2023 by Y.Ishioka
 */

//#define PICO_NO_HARDWARE 0

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/rand.h"
#include <stdlib.h>

#define PIN_ADC_TIME_SEL 26
#define PIN_BTN_MODE_SEL 14
#define PIN_BTN_RESTART  15

#define PIN_SPI_MISO     16

#define PIN_DIPSW_0      22
#define PIN_DIPSW_1      28


int  pico_dev_chk_spi_miso( void )
{
    gpio_init(PIN_SPI_MISO);
    gpio_set_dir(PIN_SPI_MISO, GPIO_IN);
    gpio_disable_pulls(PIN_SPI_MISO);

  return  (int) gpio_get(PIN_SPI_MISO);
}


unsigned int  pico_dev_read_adc0( void )
{
    adc_select_input(0);

    return  adc_read();
}


int  pico_dev_adc_init( void )
{
    adc_init();

    adc_gpio_init( PIN_ADC_TIME_SEL );
    adc_select_input( 0 );

    return  0;
}


int  pico_dev_read_dip( void )
{
    int  val = 0;

#if 1  /*  Complementary */
    if( gpio_get(PIN_DIPSW_0) == 0 ) {
        val |= 1;
    }
    if( gpio_get(PIN_DIPSW_1) == 0 ) {
        val |= 1<<1;
    }
#else /*  Standard */
    if( gpio_get(PIN_DIPSW_0) != 0 ) {
        val |= 1;
    }
    if( gpio_get(PIN_DIPSW_1) != 0 ) {
        val |= 1<<1;
    }
#endif

    return  val;
}


int  pico_dev_dip_init( void )
{
    gpio_init(PIN_DIPSW_0);
    gpio_set_dir(PIN_DIPSW_0, GPIO_IN);
    gpio_pull_up(PIN_DIPSW_0);

    gpio_init(PIN_DIPSW_1);
    gpio_set_dir(PIN_DIPSW_1, GPIO_IN);
    gpio_pull_up(PIN_DIPSW_1);

    return  0;
}


unsigned int  pico_dev_rand_read( void )
{
    uint32_t  rand32;
    rand32= get_rand_32();

    return  (unsigned int)rand32;
}


int  pico_gpio_select_btn( void )
{
  return  (int) !gpio_get(PIN_BTN_MODE_SEL);
}


int  pico_gpio_restart_btn( void )
{
  return  (int) !gpio_get(PIN_BTN_RESTART);
}


int  pico_dev_btn_init( void )
{
    gpio_init(PIN_BTN_MODE_SEL);
    gpio_set_dir(PIN_BTN_MODE_SEL, GPIO_IN);
    gpio_pull_up(PIN_BTN_MODE_SEL);

    gpio_init(PIN_BTN_RESTART);
    gpio_set_dir(PIN_BTN_RESTART, GPIO_IN);
    gpio_pull_up(PIN_BTN_RESTART);

    return  0;
}


/********* for dummy function *********/
void __unhandled_user_irq( void )
{
}

void  runtime_run_initializers( void )
{
}

unsigned long  __init_array_start[10];
unsigned long  __init_array_end;
unsigned long  __mutex_array_start[10];
unsigned long  __mutex_array_end;
unsigned long __StackLimit;;


