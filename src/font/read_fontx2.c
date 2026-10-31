/*
 * Copyright (C) 2022 by Y.Ishioka
 */

#include "fx_8x16rk_fnt.c"
#include "jiskan16_fnt.c"

#define DEF_FONT_A16_VAR       fx_8x16rk_fnt
#define DEF_FONT_K16_VAR       jiskan16_fnt


unsigned char  *get_fontx2_a( unsigned char *font, unsigned int code )
{
    unsigned char  *address = (void *)0 ;
    unsigned int  fontbyte ;

    fontbyte = (font[14] + 7) / 8 * font[15] ;
    address = &font[17] + fontbyte * code ;

    return  address ;
}


unsigned char  *get_fontx2_k( unsigned char *font, unsigned int code )
{
    unsigned char  *address = (void *)0 ;
    unsigned char  *tmp ;
    unsigned int  blknum, i, fontnum ;
    unsigned int  bstart, bend ;
    unsigned int  fontbyte ;

    fontbyte = (font[14] + 7) / 8 * font[15] ;
    fontnum = 0 ;

    blknum = (unsigned int)font[17] * 4 ;
    tmp = &font[18] ;
    for( i=0 ; i<blknum ; i+=4 ) {
        bstart = tmp[i]   + ((unsigned int)tmp[i+1] << 8) ;
        bend   = tmp[i+2] + ((unsigned int)tmp[i+3] << 8) ;
        if( code >= bstart && code <= bend ) {
            address = tmp + (fontnum + (code - bstart)) * fontbyte + blknum ;
            break ;
        }

        fontnum += (bend - bstart) + 1 ;
    }

    return  address ;
}


unsigned char  *read_fontx2_a( unsigned int code )
{
  return  get_fontx2_a( (unsigned char *)DEF_FONT_A16_VAR, code );
}


unsigned char  *read_fontx2_k( unsigned int code )
{
  return  get_fontx2_k( (unsigned char *)DEF_FONT_K16_VAR, code );
}
