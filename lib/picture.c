/*****************************************************************************
 * picture.c:  libvlc API picture management
 *****************************************************************************
 * Copyright (C) 1998-2018 VLC authors and VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc/libvlc.h>
#include <vlc/libvlc_picture.h>

#include <vlc_atomic.h>
#include <vlc_picture.h>
#include <vlc_block.h>
#include <vlc_fs.h>

#include "picture_internal.h"

struct libvlc_picture_t
{
    vlc_atomic_rc_t rc;
    libvlc_picture_type_t i_type;
    block_t* p_converted;
    video_format_t fmt;
};

libvlc_picture_t* libvlc_picture_new( vlc_object_t* p_obj, picture_t* p_input,
                                      libvlc_picture_type_t i_type,
                                      unsigned int i_width, unsigned int i_height )
{
    libvlc_picture_t *p_pic = malloc( sizeof( *p_pic ) );
    if ( unlikely( p_pic == NULL ) )
        return NULL;
    vlc_atomic_rc_init( &p_pic->rc );
    p_pic->i_type = i_type;
    vlc_fourcc_t format;
    switch ( i_type )
    {
        case libvlc_Argb:
            format = VLC_CODEC_ARGB;
            break;
        case libvlc_Jpg:
            format = VLC_CODEC_JPEG;
            break;
        case libvlc_Png:
            format = VLC_CODEC_PNG;
            break;
    }
    if ( picture_Export( p_obj, &p_pic->p_converted, &p_pic->fmt,
                         p_input, format, i_width, i_height ) != VLC_SUCCESS )
    {
        free( p_pic );
        return NULL;
    }

    return p_pic;
}

void libvlc_picture_retain( libvlc_picture_t* p_pic )
{
    vlc_atomic_rc_inc( &p_pic->rc );
}

void libvlc_picture_release( libvlc_picture_t* p_pic )
{
    if ( vlc_atomic_rc_dec( &p_pic->rc ) == false )
        return;
    video_format_Clean( &p_pic->fmt );
    if ( p_pic->p_converted )
        block_Release( p_pic->p_converted );
    free( p_pic );
}

int libvlc_picture_save( const libvlc_picture_t* p_pic, const char* psz_path )
{
    FILE* p_file = vlc_fopen( psz_path, "wb" );
    if ( !p_file )
        return -1;
    size_t res = fwrite( p_pic->p_converted->p_buffer,
                         p_pic->p_converted->i_buffer, 1, p_file );
    fclose( p_file );
    return res == 1 ? 0 : -1;
}

const unsigned char* libvlc_picture_get_buffer( const libvlc_picture_t* p_pic,
                                                size_t *p_size )
{
    assert( p_size != NULL );
    *p_size = p_pic->p_converted->i_buffer;
    return p_pic->p_converted->p_buffer;
}

libvlc_picture_type_t libvlc_picture_type( const libvlc_picture_t* p_pic )
{
    return p_pic->i_type;
}

unsigned int libvlc_picture_get_stride( const libvlc_picture_t *p_pic )
{
    assert( p_pic->i_type == libvlc_Argb );
    return p_pic->fmt.i_width * p_pic->fmt.i_bits_per_pixel / 8;
}

unsigned int libvlc_picture_get_width( const libvlc_picture_t* p_pic )
{
    return p_pic->fmt.i_visible_width;
}

unsigned int libvlc_picture_get_height( const libvlc_picture_t* p_pic )
{
    return p_pic->fmt.i_visible_height;
}
