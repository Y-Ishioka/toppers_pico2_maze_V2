/*
 * Copyright (C) 2022-2025 by Y.Ishioka
 */

#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "kernel_cfg.h"

/***********/
#define  IMAGE_BUFFER        image_buffer3
extern  unsigned char  IMAGE_BUFFER[];
/***********/

unsigned int  pico_dev_read_adc0( void );

int  pico_gpio_select_btn( void );
int  pico_gpio_restart_btn( void );
int  pico_dev_chk_spi_miso( void );

int  fat_test_init( void );
int  fat_test_read( char *filename, char *buff, int bsize );

void  clear_image_buffer( unsigned char  *buff );
unsigned char  *read_fontx2_a( unsigned int code );
unsigned char  *read_fontx2_k( unsigned int code );

#include "app_param.h"
#include "niji_data.c"

unsigned char  niji_data_buffer[ DEF_TEXT_BUFF+1 ];
unsigned char  niji_kanji_buffer[ 4 ][ DEF_FONT_HIGH ][DEF_FONT_WIDTH ];
unsigned char  ans_buffer[ DEF_FONT_HIGH ][ DEF_FONT_WIDTH ];
unsigned char  sec_buffer[2][ DEF_FONT_HIGH ][ DEF_FONT_WIDTH/2 ];
unsigned char  rem_buffer[2][ DEF_FONT_HIGH ][ DEF_FONT_WIDTH/2 ];
unsigned char  tmp_buffer[ DEF_FONT_HIGH ][ DEF_FONT_WIDTH ];
unsigned char  *niji_data_pnt;

int  char_list_1st[ DEF_NIJI_LST_MAX ];
int  char_list_2nd[ DEF_NIJI_LST_MAX ];

int  x_sec_init;
int  x_sec_remain;
int  x_handle_counter;
int  x_wait_flag;
int  x_item_all_cnt;
int  x_item_1st_cnt;
int  x_item_2nd_cnt;

extern  unsigned int disp_color_code[];

extern  const unsigned char  niji_data[];
char  *niji_data_filnename = "niji.dat";

unsigned int  disp_rct_color_code = 0x3f0303;
unsigned int  disp_arw_color_code = 0x107f00;
unsigned int  disp_que_color_code = 0x00ffff;
unsigned int  disp_ans_color_code = 0xffffff;
unsigned int  disp_sec_color_code = 0x0000ff;
unsigned int  disp_rem_color_code = 0x003f3f;

unsigned char  kigou_quest[2] = { 0x81, 0x48 };
unsigned char  kigou_kotae[2] = { 0x93, 0x9a };

unsigned char  kigou_right[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

unsigned char  kigou_down[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

unsigned char  kigou_center[ DEF_FONT_HIGH ][DEF_FONT_WIDTH ] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

unsigned char  kigou_ank_null[ DEF_FONT_HIGH ][DEF_FONT_WIDTH/2 ] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};


void  clr_img_buf( int x, int y )
{
    int  ver, hor;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *pix_p;

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                pix_p = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + (DEF_IMG_PIXEL) * hor;
                *pix_p     = *pix_p >> 1;
                *(pix_p+1) = *(pix_p+1) >> 1;
                *(pix_p+2) = *(pix_p+2) >> 1;
            }
        }
        tslp_tsk( DEF_TIM_GRAD_DLY );
    }
}


void  set_ank_to_img_buf( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    fnt_pnt = font;
    for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
        img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

        for( hor=0 ; hor<DEF_FONT_WIDTH/2 ; hor++ ) {
            if( *fnt_pnt ) {
                *img_pnt     = pix_r;
                *(img_pnt+1) = pix_g;
                *(img_pnt+2) = pix_b;
            }
            img_pnt += 3;
            fnt_pnt++;
        }
    }
}


void  clr_ank_img_buf( int x, int y )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  pix = 0x00;

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
        img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

        for( hor=0 ; hor<DEF_FONT_WIDTH/2 ; hor++ ) {
            *img_pnt     = pix;
            *(img_pnt+1) = pix;
            *(img_pnt+2) = pix;
            img_pnt += 3;
        }
    }
}


static  void  set_font( uint8_t *font, uint8_t *buff, int width )
{
    int  i, j, k;
    int  row;
    int  w = (width/8);   /* font width byte */
    uint8_t  pat;

    /* row */
    for( i=0 ; i<DEF_FONT_HIGH ; i++ ) {
        //row = DEF_FONT_WIDTH * i;
        row = width * i;
        /* col */
        for( j=0 ; j<w ; j++ ) {
            pat = 0x80;
            for( k=0 ; k<8 ; k++ ) {
                if( (font[ i * w + j ] & pat) != 0 ) {
                    buff[ row + j*8 + k ] = 1;
                }
                pat >>= 1; /* bit shift */
            }
        }
    }
}


void  make_kanji_bitmap( unsigned char *kanji, unsigned char *out )
{
    unsigned int  code;
    unsigned char  *fontdata;

    code = (unsigned int)(*kanji);
    code = (code<<8) + *(kanji+1);

    fontdata = read_fontx2_k( code );
    set_font( fontdata, (uint8_t *)out, DEF_FONT_WIDTH );
}


void  set_font_to_img_buf_quick( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

        fnt_pnt = font;
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( *fnt_pnt ) {
                    *img_pnt     = pix_r;
                    *(img_pnt+1) = pix_g;
                    *(img_pnt+2) = pix_b;
                }
                img_pnt += 3;
                fnt_pnt++;
            }
        }
}


void  set_font_to_img_buf( int x, int y, unsigned char *font, unsigned int color )
{
    int  ver, hor;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *img_pnt;
    unsigned char  *fnt_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * y
               + DEF_IMG_PIXEL * x;

    for( grad=8 ; grad>0 ; grad-- ) {
        fnt_pnt = font;
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( *fnt_pnt ) {
                    *img_pnt     = pix_r>>grad;
                    *(img_pnt+1) = pix_g>>grad;
                    *(img_pnt+2) = pix_b>>grad;
                }
                img_pnt += 3;
                fnt_pnt++;
            }
        }
        tslp_tsk( DEF_TIM_GRAD_DLY );
    }
}


void  make_ascii_bitmap( unsigned char code, unsigned char *out )
{
    unsigned char  *fontdata;

    fontdata = read_fontx2_a( (unsigned int)code );
    set_font( fontdata, (uint8_t *)out, DEF_FONT_WIDTH/2 );
}


void view_anser_rect( unsigned int color )
{
    int  ver, hor;
    unsigned char  *img_base;
    unsigned char  *img_pnt1;
    unsigned char  *img_pnt2;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    img_base = IMAGE_BUFFER 
               + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 23
               + DEF_IMG_PIXEL * 23;

    img_pnt1 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 0;
    img_pnt2 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * 17;
    for( hor=0 ; hor<(16+2) ; hor++ ) {
        *img_pnt1     = pix_r;
        *(img_pnt1+1) = pix_g;
        *(img_pnt1+2) = pix_b;
        *img_pnt2     = pix_r;
        *(img_pnt2+1) = pix_g;
        *(img_pnt2+2) = pix_b;
        img_pnt1 += 3;
        img_pnt2 += 3;
    }

    for( ver=0 ; ver<(16+2) ; ver++ ) {
        img_pnt1 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;
        img_pnt2 = img_base + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + DEF_IMG_PIXEL * 17;
        *img_pnt1     = pix_r;
        *(img_pnt1+1) = pix_g;
        *(img_pnt1+2) = pix_b;
        *img_pnt2     = pix_r;
        *(img_pnt2+1) = pix_g;
        *(img_pnt2+2) = pix_b;
    }
}


void view_sec_counter( void )
{
    int  num;
    unsigned char  col10, col1;

    num = x_sec_init;

    memset( sec_buffer, 0x00, sizeof(sec_buffer) );
    col10 = (unsigned char)(num / 10);
    col1  = (unsigned char)(num % 10);
    make_ascii_bitmap( col10+'0', (unsigned char *)sec_buffer[0] );
    make_ascii_bitmap( col1+'0',  (unsigned char *)sec_buffer[1] );

    clr_ank_img_buf( DEF_SEC_COL10_X, DEF_SEC_COL10_Y );
    clr_ank_img_buf( DEF_SEC_COL1_X,  DEF_SEC_COL1_Y );
    if( col10 != 0 ) {
        set_ank_to_img_buf( DEF_SEC_COL10_X, DEF_SEC_COL10_Y, (unsigned char *)sec_buffer[0], disp_sec_color_code );
    }
    set_ank_to_img_buf( DEF_SEC_COL1_X, DEF_SEC_COL1_Y, (unsigned char *)sec_buffer[1], disp_sec_color_code );
}


void view_remain_counter( void )
{
    int  num;
    unsigned char  col10, col1;

    num = x_sec_remain;

    memset( rem_buffer, 0x00, sizeof(rem_buffer) );
    col10 = (unsigned char)(num / 10);
    col1  = (unsigned char)(num % 10);
    make_ascii_bitmap( col10+'0', (unsigned char *)rem_buffer[0] );
    make_ascii_bitmap( col1+'0',  (unsigned char *)rem_buffer[1] );

    clr_ank_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y );
    clr_ank_img_buf( DEF_REM_COL1_X,  DEF_REM_COL1_Y );
    if( col10 != 0 ) {
        set_ank_to_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y, (unsigned char *)rem_buffer[0], disp_rem_color_code );
    }
    set_ank_to_img_buf( DEF_REM_COL1_X, DEF_REM_COL1_Y, (unsigned char *)rem_buffer[1], disp_rem_color_code );
}


int  niji_item_count( void )
{
    int  count = 0;
    unsigned char  *pnt = (unsigned char *)niji_data_pnt;

    while( *pnt != 0x00 ) {
        count++;
        pnt += 6; /* k1k1 k2k2 crlf */
    }

    return  count;
}


unsigned char  *get_random_code( void )
{
    unsigned char  *code;
    int  niji_pnt;
    int  sel_char;

    niji_pnt = rand() % x_item_all_cnt;
    sel_char = rand() & 0x1;  /* 0 or 1 */
    code = (unsigned char *)niji_data_pnt + niji_pnt * 6 + sel_char * 2;

    return  code;
}


void  niji_cyclic( void )
{
    unsigned int  vol;
    int  bak_sec;
    int  vol_def_tbl[] = { 1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 80, 99 };

    if( pico_gpio_select_btn() ) {
        x_wait_flag = 0;
    }

    if( x_sec_remain > 0 ) {
        view_remain_counter();
        x_handle_counter--;
        if( x_handle_counter <= 0 ) {
            x_handle_counter = 50;
            x_sec_remain--;
        }
    }

    if( pico_gpio_restart_btn() ) {
        x_sec_remain = 0;
        x_wait_flag = 0;
    }

    bak_sec = x_sec_init;
    vol = pico_dev_read_adc0();
    if(        vol < 0x100 ) {
        x_sec_init = vol_def_tbl[0];
    } else if( vol < 0x200 ) {
        x_sec_init = vol_def_tbl[1];
    } else if( vol < 0x300 ) {
        x_sec_init = vol_def_tbl[2];
    } else if( vol < 0x400 ) {
        x_sec_init = vol_def_tbl[3];
    } else if( vol < 0x500 ) {
        x_sec_init = vol_def_tbl[4];
    } else if( vol < 0x600 ) {
        x_sec_init = vol_def_tbl[5];
    } else if( vol < 0x700 ) {
        x_sec_init = vol_def_tbl[6];
    } else if( vol < 0x800 ) {
        x_sec_init = vol_def_tbl[7];
    } else if( vol < 0x900 ) {
        x_sec_init = vol_def_tbl[8];
    } else if( vol < 0xa00 ) {
        x_sec_init = vol_def_tbl[9];
    } else if( vol < 0xb00 ) {
        x_sec_init = vol_def_tbl[10];
    } else if( vol < 0xc00 ) {
        x_sec_init = vol_def_tbl[11];
    } else if( vol < 0xd00 ) {
        x_sec_init = vol_def_tbl[12];
    } else if( vol < 0xe00 ) {
        x_sec_init = vol_def_tbl[13];
    } else {
        x_sec_init = vol_def_tbl[14];
    }

    if( bak_sec != x_sec_init ) {
        view_sec_counter();
    }
}


void niji_main( void )
{
    int  color;
    unsigned char  *code_pnt;
    unsigned char  *niji_pnt;
    unsigned char  code1, code2;
    unsigned char  niji_ans[2];
    int  niji_a;
    int  niji_b;
    int  niji_c;
    int  niji_d;
    int  num;
    int  ret;

    syslog(LOG_NOTICE, "# Select NIJI-JUKUGO." );

    ret = pico_dev_chk_spi_miso();
    syslog(LOG_NOTICE, "# pico_dev_chk_spi_miso()=%d", ret );

    if( ret != 0 ) {
        syslog(LOG_NOTICE, "# Memory card interface available." );
    } else {
        syslog(LOG_NOTICE, "# No memory card interface." );
    }

    niji_data_pnt = (unsigned char *)niji_data;

    if( ret != 0 ) {
        ret = fat_test_init();
        syslog(LOG_NOTICE, "# fat_test_init()=%d", ret );
        if( ret == 0 ) {
            syslog(LOG_NOTICE, "# Memory card initialization successful." );
            ret = fat_test_read( niji_data_filnename, (char *)niji_data_buffer, DEF_TEXT_BUFF );
            syslog(LOG_NOTICE, "fat_test_read()=%d", ret );
            if( ret > 0 ) {
                syslog(LOG_NOTICE, "# Memory card file read successful." );
                syslog(LOG_NOTICE, "# File size is %d bytes.", ret );
                niji_data_buffer[ ret ] = 0x00;
                niji_data_pnt = niji_data_buffer;
            } else {
                syslog(LOG_NOTICE, "# Memory card file read failure." );
            }
        } else {
            syslog(LOG_NOTICE, "# Memory card initialization failure." );
        }
    }

    x_sec_init = 99;
    x_sec_remain = 0;
    x_handle_counter = 0;
    x_wait_flag = 0;
    x_item_all_cnt = niji_item_count();
    syslog(LOG_NOTICE, "# Number of items : %d", x_item_all_cnt );

    clear_image_buffer( (unsigned char *)IMAGE_BUFFER );

    /* disp allow */
    set_font_to_img_buf_quick( 16, 24, (unsigned char *)kigou_right, disp_arw_color_code );
    set_font_to_img_buf_quick( 24, 16, (unsigned char *)kigou_down,  disp_arw_color_code );
    set_font_to_img_buf_quick( 24, 40, (unsigned char *)kigou_down,  disp_arw_color_code );
    set_font_to_img_buf_quick( 40, 24, (unsigned char *)kigou_right, disp_arw_color_code );
    view_anser_rect( disp_rct_color_code );

    view_sec_counter();

    color = 0;
    niji_pnt = (unsigned char  *)niji_data_pnt;

    while( true ) {
        do {
            /* get kanji code from niji-zyukugo */
            code_pnt = get_random_code();
            code1 = *code_pnt;
            code2 = *(code_pnt+1);
            syslog(LOG_NOTICE, "create code=0x%x%x", code1, code2 );

            niji_pnt = (unsigned char *)niji_data_pnt;
            x_item_1st_cnt = 0;
            x_item_2nd_cnt = 0;
            for( num=0 ; num<x_item_all_cnt ; num++ ) {
                if( (*niji_pnt == code1) && (*(niji_pnt+1) == code2) ) {
                    char_list_1st[x_item_1st_cnt] = num;
                    x_item_1st_cnt++;
                    if( x_item_1st_cnt >= DEF_NIJI_LST_MAX ) {
                        break;
                    }
                } else if( (*(niji_pnt+2) == code1) && (*(niji_pnt+3) == code2) ) {
                    char_list_2nd[x_item_2nd_cnt] = num;
                    x_item_2nd_cnt++;
                    if( x_item_2nd_cnt >= DEF_NIJI_LST_MAX ) {
                        break;
                    }
                }
                niji_pnt += 6;
            }
        } while ( x_item_1st_cnt < 2 || x_item_2nd_cnt < 2 );

        niji_ans[0] = code1;
        niji_ans[1] = code2;
        syslog(LOG_NOTICE, "# Anser code : 0x%x%x", code1, code2 );
        syslog(LOG_NOTICE, "# 1st items : %d,  2nd items : %d", x_item_1st_cnt, x_item_2nd_cnt );

        do {
            int  tmp1, tmp2;
            tmp1 = rand() % x_item_2nd_cnt;
            tmp2 = rand() % x_item_2nd_cnt;
            niji_a = char_list_2nd[ tmp1 ];
            niji_b = char_list_2nd[ tmp2 ];
        } while( niji_a == niji_b );

        do {
            int  tmp1, tmp2;
            tmp1 = rand() % x_item_1st_cnt;
            tmp2 = rand() % x_item_1st_cnt;
            niji_c = char_list_1st[ tmp1 ];
            niji_d = char_list_1st[ tmp2 ];
        } while( niji_c == niji_d );
        syslog(LOG_NOTICE, "# Question item number : A=%d  B=%d  C=%d  D=%d", niji_a, niji_b, niji_c, niji_d );

        memset( niji_kanji_buffer, 0x00, sizeof(niji_kanji_buffer) );

        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_a*6), (unsigned char  *)niji_kanji_buffer[0] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_b*6), (unsigned char  *)niji_kanji_buffer[1] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_c*6+2), (unsigned char  *)niji_kanji_buffer[2] );
        make_kanji_bitmap( (unsigned char *)(niji_data_pnt + niji_d*6+2), (unsigned char  *)niji_kanji_buffer[3] );

        set_font_to_img_buf(  0, 24, (unsigned char *)niji_kanji_buffer[0], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 24,  0, (unsigned char *)niji_kanji_buffer[1], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 48, 24, (unsigned char *)niji_kanji_buffer[2], disp_color_code[color%6] );
        color++;
        set_font_to_img_buf( 24, 48, (unsigned char *)niji_kanji_buffer[3], disp_color_code[color%6] );
        color++;

        memset( tmp_buffer, 0x00, sizeof(tmp_buffer) );
        make_kanji_bitmap( kigou_quest, (unsigned char  *)tmp_buffer );
        set_font_to_img_buf( 24, 24, (unsigned char *)tmp_buffer, disp_que_color_code );

        x_handle_counter = 50 ; /* cyclic = 20msec */
        x_sec_remain = x_sec_init;
        x_wait_flag = 1;

        /* count down */
        while( x_sec_remain > 0 ) {
            tslp_tsk( DEF_TIM_CHK_DLY );
        }
        view_remain_counter();

        /* wait restart-btn */
        while( x_wait_flag ) {
            tslp_tsk( DEF_TIM_CHK_DLY );
        }

        memset( tmp_buffer, 0x00, sizeof(tmp_buffer) );
        clr_img_buf( 24, 24 );
        make_kanji_bitmap( niji_ans, (unsigned char  *)tmp_buffer );
        set_font_to_img_buf( 24, 24, (unsigned char *)tmp_buffer, disp_ans_color_code );

        tslp_tsk( DEF_TIM_ANS_DISP );

        clr_img_buf(  0, 24 );
        clr_img_buf( 24,  0 );
        clr_img_buf( 48, 24 );
        clr_img_buf( 24, 48 );
        clr_img_buf( 24, 24 );

        clr_ank_img_buf( DEF_REM_COL10_X, DEF_REM_COL10_Y );
        clr_ank_img_buf( DEF_REM_COL1_X,  DEF_REM_COL1_Y );

        tslp_tsk( DEF_TIM_NEXT_DLY );
    }
}

