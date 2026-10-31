/*
 * Copyright (C) 2022-2025 by Y.Ishioka
 */

/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 */

#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "kernel_cfg.h"
#include "sample1.h"

#include "app_param.h"


void  pico_gpio_led_set( int on );
int  pico_dev_adc_init( void );
int  pico_dev_read_dip( void );
int  pico_dev_dip_init( void );
unsigned int  pico_dev_rand_read( void );
int  pico_gpio_select_btn( void );
int  pico_gpio_restart_btn( void );
int  pico_dev_btn_init( void );

int hub75_test_init( void );
int hub75_test_loop( void );

void  yoji_main( void );
void  yoji_eki_main( void );
void  niji_main( void );
void  maze_main( void );
void  niji_cyclic( void );


unsigned int disp_color_code[] = {
    0xff0000, 0x00ff00, 0x1010ff, 0xffff00,
    0x00ffff, 0xff00ff, 0xffffff, 0x000000
};


unsigned char  image_buffer1[ DEF_IMAGEBUFF ];
unsigned char  image_buffer2[ DEF_IMAGEBUFF ];
unsigned char  image_buffer3[ DEF_IMAGEBUFF ];
unsigned char  image_buffer4[ DEF_IMAGEBUFF ];

unsigned int  yoji_rand_seed;
int  dip_exec_mode;
int  dip_exec_before;

extern  uint8_t   *img888_1;
extern  uint8_t   *img888_2;
extern  uint8_t   *img888_3;
extern  uint8_t   *img888_4;

void delay( int ms )
{
  tslp_tsk( ms );
}


void  clear_image_buffer( unsigned char  *buff )
{
    memset( buff, 0x00, DEF_IMAGEBUFF );
}


void  check_and_set_dip_mode( void )
{
    dip_exec_mode = pico_dev_read_dip();
    if( dip_exec_mode != dip_exec_before ) {
        switch( dip_exec_mode ) {
        case 0:
            img888_1 = (uint8_t *)image_buffer4;
            img888_2 = (uint8_t *)image_buffer2;
            img888_3 = (uint8_t *)image_buffer1;
            img888_4 = (uint8_t *)image_buffer3;
            break;
        case 1:
            img888_1 = (uint8_t *)image_buffer2;
            img888_2 = (uint8_t *)image_buffer1;
            img888_3 = (uint8_t *)image_buffer3;
            img888_4 = (uint8_t *)image_buffer4;
            break;
        case 2:
            img888_1 = (uint8_t *)image_buffer1;
            img888_2 = (uint8_t *)image_buffer3;
            img888_3 = (uint8_t *)image_buffer4;
            img888_4 = (uint8_t *)image_buffer2;
            break;
        case 3:
        default:
            img888_1 = (uint8_t *)image_buffer3;
            img888_2 = (uint8_t *)image_buffer4;
            img888_3 = (uint8_t *)image_buffer2;
            img888_4 = (uint8_t *)image_buffer1;
            break;
        }
        dip_exec_before = dip_exec_mode;
    }
}


void  cyclic_handler(intptr_t exinf)
{
    check_and_set_dip_mode();

    niji_cyclic();
}


void task1( intptr_t exinf )
{
    yoji_main();
}


void task2( intptr_t exinf )
{
    yoji_eki_main();
}


void task3( intptr_t exinf )
{
    niji_main();
}


void task4( intptr_t exinf )
{
    maze_main();
}


void main_task(intptr_t exinf)
{
    ER  ret;

    (void)hub75_test_init();
    (void)pico_dev_adc_init();
    (void)pico_dev_btn_init();
    (void)pico_dev_dip_init();

    if( pico_gpio_select_btn() != 0 ) {
        syslog(LOG_NOTICE, "# MODE button status : Continuous Mode" );
    } else {
        syslog(LOG_NOTICE, "# Mode button status : Stop Mode" );
    }

    if( pico_gpio_restart_btn() != 0 ) {
        syslog(LOG_NOTICE, "# NEXT button status : ON" );
    } else {
        syslog(LOG_NOTICE, "# Mode button status : OFF" );
    }

    yoji_rand_seed = pico_dev_rand_read();
    syslog(LOG_NOTICE, "# pico_dev_rand_read() : 0x%x", yoji_rand_seed );
    srand( yoji_rand_seed );

    dip_exec_mode = pico_dev_read_dip();
    syslog(LOG_NOTICE, "# MAIN: DIP-SW val : %d", dip_exec_mode );

    dip_exec_before = 9999;
    check_and_set_dip_mode();

    clear_image_buffer( (unsigned char *)image_buffer1 );
    clear_image_buffer( (unsigned char *)image_buffer2 );
    clear_image_buffer( (unsigned char *)image_buffer3 );
    clear_image_buffer( (unsigned char *)image_buffer4 );

    ret = act_tsk(TASK1);
    syslog(LOG_NOTICE, "# act_tsk(TASK1) : %d", ret );

    ret = act_tsk(TASK2);
    syslog(LOG_NOTICE, "# act_tsk(TASK2) : %d", ret );

    ret = act_tsk(TASK3);
    syslog(LOG_NOTICE, "# act_tsk(TASK3) : %d", ret );

    ret = act_tsk(TASK4);
    syslog(LOG_NOTICE, "# act_tsk(TASK4) : %d", ret );

    ret = sta_cyc(CYCHDR1);
    syslog(LOG_NOTICE, "# sta_cyc(CYCHDR1) : %d", ret );

    while( 1 ) {
        (void)hub75_test_loop();
        tslp_tsk( 1 );
    }
}


/*********************************************************/
void tex_routine(TEXPTN texptn, intptr_t exinf)
{
}


#ifdef CPUEXC1
void
cpuexc_handler(void *p_excinf)
{
}
#endif /* CPUEXC1 */
/*********************************************************/

