/*
 * Copyright (C) 2022-2025 by Y.Ishioka
 */

#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>

#include "app_param.h"
/***********/
#include "yoji_data.c"
#define  MODE_HEAD           "Yoji:"
#define  INIT_COLOR          0
#define  IMAGE_BUFFER        image_buffer1
extern  unsigned char  IMAGE_BUFFER[];
/***********/

extern  unsigned int disp_color_code[];

void  clear_image_buffer( unsigned char  *buff );
unsigned char  *read_fontx2_a( unsigned int code );
unsigned char  *read_fontx2_k( unsigned int code );

static  int  disp_color;
static  int  disp_color_bak;
static  int  yoji_item_num;
static  int  yoji_item_pos;
static  int  yoji_item_pos_bef1;
static  int  yoji_item_pos_bef2;

static  int  yoji_pos;
static  int  yoji_dir;
static  int  yoji_bef_pos;
static  int  yoji_bef_dir;
static  unsigned int  yoji_code;
static  unsigned char  *src_data;

static  int  yoji_list[ DEF_YOJI_LST_MAX ];
static  unsigned char  yoji_buffer[ 4 ][ DEF_FONT_HIGH ][DEF_FONT_WIDTH ];


static  int  yoji_clear_hor_pat[][DEF_YOJI_MOJI] = {
    {  0,  1,  2,  3 },
    {  4,  5,  6,  7 },
    {  8,  9, 10, 11 },
    { 12, 13, 14, 15 }
};

static  int  yoji_clear_ver_pat[][DEF_YOJI_MOJI] = {
    {  0,  4,  8, 12 },
    {  1,  5,  9, 13 },
    {  2,  6, 10, 14 },
    {  3,  7, 11, 15 }
};


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


static  void  make_yoji_bitmap( unsigned char  *yoji )
{
    unsigned int  code;
    int  i;
    unsigned char  *fontdata;

	memset( yoji_buffer, 0x00, sizeof(yoji_buffer) );

    for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
        code = (unsigned int)(*yoji);
        code = (code<<8) + *(yoji+1);
        fontdata = read_fontx2_k( code );
        set_font( fontdata, (uint8_t *)yoji_buffer[i], DEF_FONT_WIDTH );
        yoji += 2;
    }
}


/*
 * pos  : 0-3
 * dir  : 0:horizontal, 1:vertical
 * color: 0xBBGGRR
 *
 * src-data : yoji_buffer[][DEF_FONT_HIGH][DEF_FONT_WIDTH]
 * dst-data : image_buffer[DEF_IMG_WIDTH*DEF_DISP_HIGH*DEF_IMG_PIXEL]
 *
 * pos=0, dir=0  pos=1, dir=0  pos=2, dir=0  pos=3, dir=0
 *   0 1 2 3       - - - -       - - - -       - - - -
 *   - - - -       4 5 6 7       - - - -       - - - -
 *   - - - -       - - - -       8 9 a b       - - - -
 *   - - - -       - - - -       - - - -       c d e f
 *
 * pos=0, dir=1  pos=1, dir=1  pos=2, dir=1  pos=3, dir=1
 *   0 - - -       - 1 - -       - - 2 -       - - - 3
 *   4 - - -       - 5 - -       - - 6 -       - - - 7
 *   8 - - -       - 9 - -       - - a -       - - - b
 *   c - - -       - d - -       - - e -       - - - f
 *
 */
static  void  set_yoji_to_image( int pos, int dir, unsigned int color )
{
    int  ver, hor;
    int  moji;
    int inv_dir;
    int  grad;
    unsigned char  *img_base;
    unsigned char  *img_tmp;
    unsigned char  *img_pnt;
    unsigned char  pix_r = (unsigned char)(color & 0xff);
    unsigned char  pix_g = (unsigned char)((color >> 8) & 0xff);
    unsigned char  pix_b = (unsigned char)((color >> 16) & 0xff);

    if( dir == 0 ) {
        inv_dir = 1;
    } else {
        inv_dir = 0;
    }

    img_base = IMAGE_BUFFER 
             + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (pos * inv_dir)
             + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (pos * dir);

    for( moji=0 ; moji<DEF_YOJI_MOJI ; moji++ ) {
        img_tmp = img_base
                  + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (moji * dir)
                  + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (moji * inv_dir);

      for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            img_pnt = img_tmp + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver;

            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                if( yoji_buffer[moji][ver][hor] ) {
                    *img_pnt     = pix_r>>grad;
                    *(img_pnt+1) = pix_g>>grad;
                    *(img_pnt+2) = pix_b>>grad;
                }
                img_pnt += 3;
            }
        }
        tslp_tsk( DEF_TIM_GRAD_DLY );
      }
    }
}


/*
 * area:
 *   0  1  2  3
 *   4  5  6  7
 *   8  9 10 11
 *  12 13 14 15
 */
static  void  clear_image( int area )
{
    int  ver, hor;
    int  grad;
    unsigned char  *area_pnt;
    unsigned char  *pix_p;

    area_pnt = IMAGE_BUFFER 
             + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * DEF_FONT_HIGH * (area>>2)
             + (DEF_FONT_WIDTH * DEF_IMG_PIXEL) * (area & 0x03);

    for( grad=8 ; grad>0 ; grad-- ) {
        for( ver=0 ; ver<DEF_FONT_HIGH ; ver++ ) {
            for( hor=0 ; hor<DEF_FONT_WIDTH ; hor++ ) {
                pix_p = area_pnt + (DEF_IMG_WIDTH * DEF_IMG_PIXEL) * ver + (DEF_IMG_PIXEL) * hor + 0;
                *pix_p     = *pix_p >> 1;
                *(pix_p+1) = *(pix_p+1) >> 1;
                *(pix_p+2) = *(pix_p+2) >> 1;
            }
        }
        tslp_tsk( DEF_TIM_GRAD_DLY );
    }
}


static  int  yoji_item_count( void )
{
    int  count = 0;
    unsigned char  *pnt = (unsigned char *)src_data;

    while( *pnt != 0x00 ) {
        count++;
        pnt += 10;
    }

    return  count;
}


static  int  yoji_comp_list( unsigned int code, int pos )
{
    int  num;
    int  i;
    unsigned char  *pnt = (unsigned char *)src_data;

    for( i=0 ; i<DEF_YOJI_LST_MAX ; i++ ) {
        yoji_list[i] = 0;
    }

    pnt += pos * 2;
    num = 0;
    for( i=0 ; i<yoji_item_num ; i++ ) {
        if( *pnt == ((code >> 8) & 0xff) && *(pnt+1) == (code & 0xff) ) {
            yoji_list[ num++ ] = i;
            if( num >= DEF_YOJI_LST_MAX ) {
                break;
            }
        }
        pnt += 10;
    }

    return  num;
}


void  yoji_main( void )
{
    unsigned char  *yoji_table;
    int  num;
    int  i;
    int  match;
    int  pos;
    int  loop;
    int  tmp_yoji_item_pos;

    clear_image_buffer( (unsigned char *)IMAGE_BUFFER );

    yoji_pos = 0;
    yoji_dir = 0;
    yoji_bef_pos = 0;
    yoji_bef_dir = 0;
    yoji_code = 0x0000;
    yoji_item_pos_bef1 = 99999;
    yoji_item_pos_bef2 = 99999;

    src_data = (unsigned char *)yoji_data;
    syslog(LOG_NOTICE, "# Select YOJI-JUKUGO." );

    yoji_item_num = yoji_item_count();
    syslog(LOG_NOTICE, "%s yoji_item_num=%d", MODE_HEAD, yoji_item_num );
    yoji_item_pos = rand() % yoji_item_num;
    syslog(LOG_NOTICE, "%s yoji_item_pos=%d", MODE_HEAD, yoji_item_pos );

    disp_color = INIT_COLOR;
    disp_color_bak = 0;
    yoji_table = (unsigned char *)src_data + yoji_item_pos * 10;

    /* set yoji */
    make_yoji_bitmap( yoji_table );
    set_yoji_to_image( yoji_pos, yoji_dir, disp_color_code[disp_color] );

    tslp_tsk( DEF_TIM_YOJI_DLY );

    while( true ) {
        disp_color_bak = disp_color;
        disp_color++;
        if( disp_color_code[disp_color] == 0x0000 ) {
            disp_color = 0;
        }

        yoji_bef_pos = yoji_pos;
        yoji_pos = rand() % DEF_YOJI_MOJI;

        /* search yoji */
        for( match=0 ; match <= 1 ; match++ ) {
            for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                yoji_table = (unsigned char *)src_data + yoji_item_pos * 10 + yoji_pos * 2;
                yoji_code = (*yoji_table << 8) + *(yoji_table + 1);

                /* make list and get list-num */
                num = yoji_comp_list( yoji_code, yoji_bef_pos );

                if( num == 0 ) {
                    syslog(LOG_NOTICE, "%s pos=%d (match=%d  num=%d)  skip search.", MODE_HEAD, yoji_pos, match, num );
                    yoji_pos++;
                    if( yoji_pos >= DEF_YOJI_MOJI ) {
                        yoji_pos = 0;
                    }
                    continue;
                }
                syslog(LOG_NOTICE, "%s pos=%d (match=%d  num=%d)", MODE_HEAD, yoji_pos, match, num );

                /* duplication check for previous data */
                tmp_yoji_item_pos = yoji_item_pos;
                for( loop=0 ; loop<5 ; loop++ ) {
                    pos = rand() % num;
                    yoji_item_pos = yoji_list[pos];
                    //if( (match > 0 && num == 1) || 
                    if( (match > 0) || 
                        (yoji_item_pos != yoji_item_pos_bef1 && yoji_item_pos != yoji_item_pos_bef2) ) {
                        match = 2;
                        break;
                    }
                }
                if( match == 2 ) {
                    break;
                } else {
                    syslog(LOG_NOTICE, "%s duplicated. new=%d  bef1=%d  bef2=%d", 
                           MODE_HEAD, yoji_item_pos, yoji_item_pos_bef1, yoji_item_pos_bef2 );
                    yoji_item_pos = tmp_yoji_item_pos;
                    yoji_pos++;
                    if( yoji_pos >= DEF_YOJI_MOJI ) {
                        yoji_pos = 0;
                    }
                }
            }
        }
        syslog(LOG_NOTICE, "%s pos=%d  yoji_item_pos=%d", MODE_HEAD, yoji_pos, yoji_item_pos );

            yoji_bef_dir = yoji_dir;
            if( yoji_dir == 0 ) {
                yoji_dir = 1;
            } else {
                yoji_dir = 0;
            }

            yoji_table = (unsigned char *)src_data + yoji_item_pos * 10;
            /* set yoji */
            make_yoji_bitmap( yoji_table );
            set_yoji_to_image( yoji_pos, yoji_dir, disp_color_code[disp_color] );
            tslp_tsk( DEF_TIM_SET_DLY );

            if( yoji_bef_dir == 0 ) {
                for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                    if( i != yoji_pos ) {
                        clear_image( yoji_clear_hor_pat[yoji_bef_pos][i] );
                    }
                }
            } else {
                for( i=0 ; i<DEF_YOJI_MOJI ; i++ ) {
                    if( i != yoji_pos ) {
                        clear_image( yoji_clear_ver_pat[yoji_bef_pos][i] );
                    }
                }
            }

            yoji_item_pos_bef2 = yoji_item_pos_bef1;
            yoji_item_pos_bef1 = yoji_item_pos;

        tslp_tsk( DEF_TIM_YOJI_DLY );
    }
}


