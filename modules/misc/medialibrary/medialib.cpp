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
#include <vlc_plugin.h>
#include <vlc_media_library.h>

#include <medialibrary/IMediaLibrary.h>

using medialibrary::IMediaLibrary;

static int Open( vlc_object_t* p_obj )
{
    vlc_medialibrary_t* p_module = p_obj;

    std::unique_ptr<IMediaLibrary> ml( NewMediaLibrary() );


    p_module->p_sys = ml.release();
    return VLC_SUCCESS;
}

static void Close( vlc_medialibrary_t* p_module )
{
    MediaLibrary* p_ml = reinterpret_cast<MediaLibrary*>( p_module->p_sys );
    delete p_ml;
}


vlc_module_begin()
    set_shortname(N_("media library"))
    set_description(N_( "Organize your media" ))
    set_category(CAT_ADVANCED)
    set_subcategory(SUBCAT_ADVANCED_MISC)
    set_capability("medialibrary", 100)
    set_callbacks(Open, Close)
vlc_module_end()
