/*****************************************************************************
 * vlc_thumbnailer.h: Thumbnailing API
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

#ifndef VLC_THUMBNAILER_H
#define VLC_THUMBNAILER_H

#include <vlc_common.h>

typedef struct vlc_thumbnailer_t vlc_thumbnailer_t;
typedef struct vlc_thumbnailer_request_t vlc_thumbnailer_request_t;

/**
 * @brief vlc_thumbnailer_cb_t defines a callback invoked on thumbnailing completion.
 *
 * This callback will always be called, provided vlc_thumbnailer_Request returned
 * a non NULL request.
 * In case of failure, p_thumbnail will be NULL.
 * The picture, if any, is owned by the thumbnail, and must be acquired by using
 * \link picture_Hold \endlink to use it passed the callback's scope.
 *
 * @param data Is the opaque pointer passed as vlc_thumbnailer_Create last parameter
 */
typedef void(*vlc_thumbnailer_cb)( void* data, picture_t* p_thumbnail );

/**
 * @brief vlc_thumbnailer_Create Creates an asynchrone thumbnailer object
 * @param p_parent A VLC object
 * @return A thumbnailer object, or NULL in case of failure
 */
VLC_API vlc_thumbnailer_t*
vlc_thumbnailer_Create( vlc_object_t* p_parent )
VLC_USED;

/**
 * @brief vlc_thumbnailer_Request
 * @param p_thumbnailer
 * @param p_input_item
 * @param i_time
 * @param p_cb A callback to be invoked upon completion
 * @param p_data
 * @return
 *
 * This function will immediatly start the thumbnailing process.
 * If this function returns a valid thumbnailer object, vlc_thumbnailer_Stop
 * must be called to release the resources, even when the callback gets
 * invoked with an error status.
 */
VLC_API vlc_thumbnailer_request_t*
vlc_thumbnailer_Request( vlc_thumbnailer_t *p_thumbnailer,
                         input_item_t* p_input_item, vlc_tick_t i_time,
                         vlc_thumbnailer_cb p_cb, void* p_data );

VLC_API void
vlc_thumbnailer_Cancel( vlc_thumbnailer_t* p_thumbnailer, vlc_thumbnailer_request_t* p_request );

/**
 * @brief vlc_thumbnailer_Release releases a thumbnailer and stop all processing
 *        requests, if any.
 * @param p_thumbnailer A thumbnailer object
 */
VLC_API void vlc_thumbnailer_Release( vlc_thumbnailer_t* p_thumbnailer );

#endif // VLC_THUMBNAILER_H
