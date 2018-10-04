/*****************************************************************************
 * libvlc_picture.h:  libvlc external API
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

#ifndef VLC_LIBVLC_PICTURE_H
#define VLC_LIBVLC_PICTURE_H 1

# ifdef __cplusplus
extern "C" {
# endif

typedef struct libvlc_picture_t libvlc_picture_t;

typedef enum libvlc_picture_type_t
{
    libvlc_Argb,
    libvlc_Png,
    libvlc_Jpg,
} libvlc_picture_type_t;

LIBVLC_API void
libvlc_picture_release( libvlc_picture_t* p_pic );

LIBVLC_API int
libvlc_picture_save( const libvlc_picture_t* p_pic, const char* psz_path );

LIBVLC_API const unsigned char*
libvlc_picture_get_buffer( const libvlc_picture_t* p_pic, size_t *p_size );

LIBVLC_API libvlc_picture_type_t
libvlc_picture_type( const libvlc_picture_t* p_pic );

LIBVLC_API unsigned int
libvlc_picture_get_stride( const libvlc_picture_t* p_pic );

LIBVLC_API unsigned int
libvlc_picture_get_width( const libvlc_picture_t* p_pic );

LIBVLC_API unsigned int
libvlc_picture_get_height( const libvlc_picture_t* p_pic );

# ifdef __cplusplus
}
# endif

#endif // VLC_LIBVLC_PICTURE_H
