/*
 * Copyright (C) 2022 by Y.Ishioka
 */

#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

#include "app_param.h"

char  sbuff[64];

extern  uint16_t  image_buffer[];

void test_logout( char *msg );


int  fat_test_init( void )
{
    DSTATUS  ret;
    int  result = 0;

    ret = disk_initialize( 0 );
    if( ret & STA_NOINIT ) {
        result = -1;
    }

    return  result;
}


int  fat_test_read( char *filename, char *buff, int bsize )
{
    FRESULT  ret;
    FATFS  fs;
    FIL  fil;
    UINT  rdsz ;

    ret = f_mount( &fs, "", 0 );
    if( ret != FR_OK ) {
        return  -1;
    }
    ret = f_open( &fil, filename, FA_READ );
    if( ret != FR_OK ) {
        return  -2;
    }

    ret = f_read( &fil, buff, (UINT)bsize, &rdsz );
    if( ret != FR_OK ) {
        return  -3;
    }

    f_close( &fil );

    return  (int)rdsz;
}


int  fat_test_read_offset( char *filename, char *buff, int bsize, int offset )
{
    FRESULT  ret;
    FATFS  fs;
    FIL  fil;
    UINT  rdsz ;

    ret = f_mount( &fs, "", 0 );
    if( ret != FR_OK ) {
        return  -1;
    }
    ret = f_open( &fil, filename, FA_READ );
    if( ret != FR_OK ) {
        return  -2;
    }

    ret = f_lseek( &fil, (DWORD)offset );
    if( ret != FR_OK ) {
        return  -3;
    }

    ret = f_read( &fil, buff, (UINT)bsize, &rdsz );
    if( ret != FR_OK ) {
        return  -4;
    }

    f_close( &fil );

    return  (int)rdsz;
}


int  fat_test_write( char *filename, char *buff, int size )
{
    FRESULT  ret;
    FATFS  fs;
    FIL  fil;
    UINT  wsize ;

    ret = f_mount( &fs, "", 1 );
    if( ret != FR_OK ) {
        return  -1;
    }
    ret = f_open( &fil, filename, FA_WRITE|FA_CREATE_ALWAYS );
    if( ret != FR_OK ) {
        return  -2;
    }

    ret = f_write( &fil, buff, (UINT)size, &wsize );
    if( ret != FR_OK ) {
        return  -3;
    }

    f_close( &fil );

    return  (int)wsize;
}

