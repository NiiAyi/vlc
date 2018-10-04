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

/**
 * Increment the reference count of this picture.
 *
 * \see libvlc_picture_release()
 * \param p_pic A picture object
 */
LIBVLC_API void
libvlc_picture_retain( libvlc_picture_t* p_pic );

/**
 * Decrement the reference count of this picture.
 * When the reference count reaches 0, the picture will be released.
 * The picture must not be accessed after calling this function.
 *
 * \see libvlc_picture_retain
 * \param p_pic A picture object
 */
LIBVLC_API void
libvlc_picture_release( libvlc_picture_t* p_pic );

/**
 * Saves this picture to a file. The image format is the same as the one
 * returned by \link libvlc_picture_type \endlink
 *
 * \param p_pic A picture object
 * \param psz_path The path to the generated file
 * \return 0 in case of success, -1 otherwise
 */
LIBVLC_API int
libvlc_picture_save( const libvlc_picture_t* p_pic, const char* psz_path );

/**
 * Returns the image internal buffer, including potential padding.
 * The libvlc_picture_t owns the returned buffer, which must not be modified nor
 * freed.
 *
 * \param p_pic A picture object
 * \param p_size A pointer to a size_t that will hold the size of the buffer [required]
 * \return A pointer to the internal buffer.
 */
LIBVLC_API const unsigned char*
libvlc_picture_get_buffer( const libvlc_picture_t* p_pic, size_t *p_size );

/**
 * Returns the picture type
 *
 * \param p_pic A picture object
 * \see libvlc_picture_type_t
 */
LIBVLC_API libvlc_picture_type_t
libvlc_picture_type( const libvlc_picture_t* p_pic );

/**
 * Returns the image stride, ie. the number of bytes per line.
 * This can only be called on images of type libvlc_Argb
 *
 * \param p_pic A picture object
 */
LIBVLC_API unsigned int
libvlc_picture_get_stride( const libvlc_picture_t* p_pic );

/**
 * Returns the width of the image in pixels
 *
 * \param p_pic A picture object
 */
LIBVLC_API unsigned int
libvlc_picture_get_width( const libvlc_picture_t* p_pic );

/**
 * Returns the height of the image in pixels
 *
 * \param p_pic A picture object
 */
LIBVLC_API unsigned int
libvlc_picture_get_height( const libvlc_picture_t* p_pic );

# ifdef __cplusplus
}
# endif

#endif // VLC_LIBVLC_PICTURE_H
