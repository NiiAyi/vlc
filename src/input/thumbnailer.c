/*****************************************************************************
 * thumbnailer.c: Thumbnailing API
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

#include <vlc_thumbnailer.h>
#include <vlc_input.h>
#include <vlc_fs.h>
#include <vlc_block.h>
#include <vlc_atomic.h>
#include "misc/background_worker.h"

struct vlc_thumbnailer_t
{
    vlc_object_t* p_parent;
    struct background_worker* worker;
};

struct vlc_thumbnailer_request_t
{
    vlc_thumbnailer_t *p_thumbnailer;
    input_thread_t *p_input_thread;
    vlc_tick_t i_time;
    input_item_t *p_item;

    vlc_thumbnailer_cb p_cb;
    void* p_user_data;

    vlc_mutex_t lock;
    bool b_done;
    picture_t* p_picture;

    struct vlc_list list_item;
};

static void
on_thumbnailer_input_event( input_thread_t *input,
                            const struct vlc_input_event *event, void *userdata )
{
    VLC_UNUSED(input);
    if ( event->type != INPUT_EVENT_THUMBNAIL_READY &&
         ( event->type != INPUT_EVENT_STATE || event->state != ERROR_S ) )
         return;

    vlc_thumbnailer_request_t* p_request = userdata;
    picture_t *p_pic = NULL;

    if ( event->type == INPUT_EVENT_THUMBNAIL_READY )
    {
        // Stop the input thread ASAP, delegate its release to
        // thumbnailer_request_Release
        input_Stop( p_request->p_input_thread );
        p_pic = event->thumbnail;
    }
    vlc_mutex_lock( &p_request->lock );
    p_request->b_done = true;
    p_request->p_picture = p_pic;
    // If the request has not been cancelled, we can invoke the completion
    // callback. A request that was cancelled early in its processing might
    // send an error state, which we don't want to propagate.
    if ( p_request->p_cb )
        p_request->p_cb( p_request->p_user_data, p_request->p_picture );
    vlc_mutex_unlock( &p_request->lock );
    background_worker_RequestProbe( p_request->p_thumbnailer->worker );
}

static void thumbnailer_request_Hold( void* p_data )
{
    VLC_UNUSED(p_data);
}

static void thumbnailer_request_Release( void* p_data )
{
    vlc_thumbnailer_request_t* p_request = p_data;
    if ( p_request->p_input_thread )
        input_Close( p_request->p_input_thread );

    input_item_Release( p_request->p_item );
    vlc_mutex_destroy( &p_request->lock );
    if ( p_request->p_picture )
        picture_Release( p_request->p_picture );
    free( p_request );
}

static int thumbnailer_request_Start( void* owner, void* entity, void** out )
{
    vlc_thumbnailer_t *p_thumbnailer = owner;
    vlc_thumbnailer_request_t *p_request = entity;
    input_thread_t *p_input = p_request->p_input_thread =
            input_CreateThumbnailer( p_thumbnailer->p_parent,
                                     on_thumbnailer_input_event, p_request,
                                     p_request->p_item );
    if ( unlikely( p_input == NULL ) )
        return VLC_EGENERIC;
    var_SetFloat( p_input, "start-time", secf_from_vlc_tick( p_request->i_time ) );
    if ( input_Start(p_input) != VLC_SUCCESS )
        return VLC_EGENERIC;
    *out = p_request;
    return VLC_SUCCESS;
}

static void thumbnailer_request_Stop( void* owner, void* handle )
{
    VLC_UNUSED(owner);
    vlc_thumbnailer_request_t *p_request = handle;
    assert( p_request->p_input_thread != NULL );
    input_Stop( p_request->p_input_thread );
}

static int thumbnailer_request_Probe( void* owner, void* handle )
{
    VLC_UNUSED(owner);
    vlc_thumbnailer_request_t *p_request = handle;
    vlc_mutex_lock( &p_request->lock );
    int res = p_request->b_done;
    vlc_mutex_unlock( &p_request->lock );
    return res;
}

vlc_thumbnailer_request_t*
vlc_thumbnailer_Request( vlc_thumbnailer_t* p_thumbnailer,
                         input_item_t* p_input_item, vlc_tick_t i_time,
                         vlc_thumbnailer_cb p_cb, void* p_user_data )
{
    vlc_thumbnailer_request_t *p_request = malloc( sizeof( *p_request ) );
    if ( unlikely( p_request == NULL ) )
        return NULL;
    p_request ->p_thumbnailer = p_thumbnailer;
    p_request->p_input_thread = NULL;
    p_request->i_time = i_time;
    p_request->p_item = input_item_Hold( p_input_item );
    p_request->b_done = false;
    p_request->p_picture = NULL;
    p_request->p_cb = p_cb;
    p_request->p_user_data = p_user_data;
    vlc_mutex_init( &p_request->lock );

    if ( background_worker_Push( p_thumbnailer->worker, p_request,
                                 p_request, -1 ) != VLC_SUCCESS )
    {
        thumbnailer_request_Release( p_request );
        return NULL;
    }
    return p_request;
}

void vlc_thumbnailer_Cancel( vlc_thumbnailer_t* p_thumbnailer,
                             vlc_thumbnailer_request_t* p_req )
{
    vlc_mutex_lock( &p_req->lock );
    // Ensure we won't invoke the callback if the input was running.
    p_req->p_cb = NULL;
    vlc_mutex_unlock( &p_req->lock );
    background_worker_Cancel( p_thumbnailer->worker, p_req );
}

vlc_thumbnailer_t *vlc_thumbnailer_Create( vlc_object_t* p_parent)
{
    vlc_thumbnailer_t *p_thumbnailer = malloc( sizeof( *p_thumbnailer ) );
    if ( unlikely( p_thumbnailer == NULL ) )
        return NULL;
    p_thumbnailer->p_parent = p_parent;
    struct background_worker_config cfg = {
        .default_timeout = vlc_tick_from_secf( 5.f ),
        .max_threads = 1,
        .pf_release = thumbnailer_request_Release,
        .pf_hold = thumbnailer_request_Hold,
        .pf_start = thumbnailer_request_Start,
        .pf_probe = thumbnailer_request_Probe,
        .pf_stop = thumbnailer_request_Stop,
    };
    p_thumbnailer->worker = background_worker_New( p_thumbnailer, &cfg );
    if ( unlikely( p_thumbnailer->worker == NULL ) )
    {
        free( p_thumbnailer );
        return NULL;
    }
    return p_thumbnailer;
}

void vlc_thumbnailer_Release( vlc_thumbnailer_t *p_thumbnailer )
{
    background_worker_Delete( p_thumbnailer->worker );
    free( p_thumbnailer );
}
