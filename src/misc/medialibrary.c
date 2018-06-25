/*****************************************************************************
 * medialib.cpp: medialibrary module
 *****************************************************************************
 * Copyright © 2015-2016 VLC authors, VideoLAN and VideoLabs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_media_library.h>
#include <vlc_modules.h>
#include <libvlc.h>

#include <assert.h>

void vlc_ml_entrypoints_release( ml_entrypoint_t* p_list, size_t i_nb_items )
{
    for ( size_t i = 0; i < i_nb_items; ++i )
    {
        free( p_list[i].psz_mrl );
    }
    free( p_list );
}

#undef vlc_ml_create
vlc_medialibrary_t* vlc_ml_create( vlc_object_t* p_obj )
{
    vlc_medialibrary_t *p_ml = vlc_custom_create( p_obj, sizeof( *p_ml ), "medialibrary" );
    if ( unlikely( p_ml == NULL ) )
        return NULL;
    p_ml->p_module = module_need( p_ml, "medialibrary", NULL, false );
    if ( p_ml->p_module == NULL )
    {
        vlc_object_release( p_ml );
        return NULL;
    }
    return p_ml;
}

void vlc_ml_release( vlc_medialibrary_t* p_ml )
{
    assert( p_ml != NULL );
    module_unneed( p_ml, p_ml->p_module );
    vlc_object_release( p_ml );
}

static void vlc_ml_show_release_inner( ml_show_t* p_show )
{
    free( p_show->psz_artwork_mrl );
    free( p_show->psz_name );
    free( p_show->psz_summary );
    free( p_show->psz_tvdb_id );
}

void vlc_ml_show_release( ml_show_t* p_show )
{
    if ( p_show == NULL )
        return;
    vlc_ml_show_release_inner( p_show );
    free( p_show );
}

static void vlc_ml_media_release_inner( ml_media_t* p_media )
{
    vlc_ml_file_list_release( p_media->p_files );
    free( p_media->psz_title );
    free( p_media->psz_artwork_mrl );
    switch( p_media->i_subtype )
    {
        case ML_MEDIA_SUBTYPE_ALBUMTRACK:
            break;
        case ML_MEDIA_SUBTYPE_SHOW_EPISODE:
            free( p_media->show_episode.psz_summary );
            free( p_media->show_episode.psz_tvdb_id );
            break;
        case ML_MEDIA_SUBTYPE_MOVIE:
            free( p_media->movie.psz_summary );
            free( p_media->movie.psz_imdb_id );
            break;
        default:
            vlc_assert_unreachable();
    }
}

static void vlc_ml_artist_release_inner( ml_artist_t* p_artist )
{
    free( p_artist->psz_artwork_mrl );
    free( p_artist->psz_name );
    free( p_artist->psz_shortbio );
    free( p_artist->psz_mb_id );
}

void vlc_ml_artist_release( ml_artist_t* p_artist )
{
    if ( p_artist == NULL )
        return;
    vlc_ml_artist_release_inner( p_artist );
    free( p_artist );
}

static void vlc_ml_album_release_inner( ml_album_t* p_album )
{
    free( p_album->psz_artist );
    free( p_album->psz_artwork_mrl );
    free( p_album->psz_summary );
    free( p_album->psz_title );
    vlc_ml_artist_list_release( p_album->p_featuring );
}

void vlc_ml_album_release( ml_album_t* p_album )
{
    if ( p_album == NULL )
        return;
    vlc_ml_album_release_inner( p_album );
    free( p_album );
}

void vlc_ml_genre_release( ml_genre_t* p_genre )
{
    if ( p_genre == NULL )
        return;
    free( p_genre->psz_name );
    free( p_genre );
}

/* Lists release */

void vlc_ml_media_release( ml_media_t* p_media )
{
    if ( p_media == NULL )
        return;
    vlc_ml_media_release_inner( p_media );
    free( p_media );
}

void vlc_ml_label_list_release( ml_label_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        free( p_list->p_items[i].psz_name );
    free( p_list );
}

void vlc_ml_file_list_release( ml_file_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        free( p_list->p_items[i].psz_mrl );
    free( p_list );
}

void vlc_ml_artist_list_release( ml_artist_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        vlc_ml_artist_release_inner( &p_list->p_items[i] );
    free( p_list );
}


void vlc_ml_media_list_release( ml_media_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        vlc_ml_media_release_inner( &p_list->p_items[i] );
    free( p_list );
}

void vlc_ml_album_list_release( ml_album_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        vlc_ml_album_release_inner( &p_list->p_items[i] );
    free( p_list );
}

void vlc_ml_show_list_release( ml_show_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        vlc_ml_show_release_inner( &p_list->p_items[i] );
    free( p_list );
}

void vlc_ml_genre_list_release( ml_genre_list_t* p_list )
{
    if ( p_list == NULL )
        return;
    for ( size_t i = 0; i < p_list->i_nb_items; ++i )
        free( p_list->p_items[i].psz_name );
    free( p_list );
}
