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
#include "medialibrary.h"

class Logger : public medialibrary::ILogger
{
public:
    Logger( vlc_object_t* obj ) : m_obj( obj ) {}

private:
    virtual void Error( const std::string& msg ) override
    {
        msg_Err( m_obj, "%s", msg.c_str() );
    }
    virtual void Warning( const std::string& msg ) override
    {
        msg_Warn( m_obj, "%s", msg.c_str() );
    }
    virtual void Info( const std::string& msg ) override
    {
        msg_Dbg( m_obj, "%s", msg.c_str() );
    }
    virtual void Debug( const std::string& msg ) override
    {
        msg_Dbg( m_obj, "%s", msg.c_str() );
    }

private:
    vlc_object_t* m_obj;
};

void MediaLibrary::onMediaAdded( std::vector<medialibrary::MediaPtr> )
{
}

void MediaLibrary::onMediaUpdated( std::vector<medialibrary::MediaPtr> )
{
}

void MediaLibrary::onMediaDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onArtistsAdded( std::vector<medialibrary::ArtistPtr> )
{
}

void MediaLibrary::onArtistsModified( std::vector<medialibrary::ArtistPtr> )
{
}

void MediaLibrary::onArtistsDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onAlbumsAdded( std::vector<medialibrary::AlbumPtr> )
{
}

void MediaLibrary::onAlbumsModified( std::vector<medialibrary::AlbumPtr> )
{
}

void MediaLibrary::onAlbumsDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onTracksAdded( std::vector<medialibrary::AlbumTrackPtr> )
{
}

void MediaLibrary::onTracksDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onPlaylistsAdded( std::vector<medialibrary::PlaylistPtr> )
{
}

void MediaLibrary::onPlaylistsModified( std::vector<medialibrary::PlaylistPtr> )
{
}

void MediaLibrary::onPlaylistsDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onDiscoveryStarted( const std::string& )
{
}

void MediaLibrary::onDiscoveryProgress( const std::string& )
{
}

void MediaLibrary::onDiscoveryCompleted( const std::string& )
{
}

void MediaLibrary::onReloadStarted( const std::string& )
{
}

void MediaLibrary::onReloadCompleted( const std::string& )
{
}

void MediaLibrary::onEntryPointRemoved( const std::string&, bool )
{
}

void MediaLibrary::onEntryPointBanned( const std::string&, bool )
{
}

void MediaLibrary::onEntryPointUnbanned( const std::string&, bool )
{
}

void MediaLibrary::onParsingStatsUpdated( uint32_t )
{
}

void MediaLibrary::onBackgroundTasksIdleChanged( bool )
{
}

void MediaLibrary::onMediaThumbnailReady( medialibrary::MediaPtr, bool )
{
}

MediaLibrary::MediaLibrary( vlc_object_t* obj )
    : m_logger( new Logger( obj ) )
    , m_ml( NewMediaLibrary() )
{
    m_ml->setVerbosity( medialibrary::LogLevel::Info );
    m_ml->setLogger( m_logger.get() );
    auto userDir = wrapCPtr( config_GetUserDir( VLC_USERDATA_DIR ) );
    std::string mlDir = std::string{ userDir.get() } + "/ml/";

    //tmp:
    unlink( (mlDir + "ml.db").c_str() );

    auto initStatus = m_ml->initialize( mlDir + "ml.db", mlDir + "thumbnails/", this );
    switch ( initStatus )
    {
        case medialibrary::InitializeResult::AlreadyInitialized:
            throw std::runtime_error( "Unexpected double medialibrary intialization" );
        case medialibrary::InitializeResult::Failed:
            throw std::runtime_error( "Medialibrary failed to initialize" );
        case medialibrary::InitializeResult::DbReset:
            msg_Info( obj, "FIXME: Handle database reset" );
            break;
        case medialibrary::InitializeResult::Success:
            msg_Dbg( obj, "MediaLibrary successfully initialized" );
            break;
    }

    m_ml->addParserService( std::make_shared<MetadataExtractor>( obj ) );
    auto res = m_ml->start();
    if ( res == false )
        throw std::runtime_error( "Failed to start medialibrary" );
    m_ml->discover( "file:///home/chouquette/dev/medialibrary/test/samples/samples/playlist/same_folder/" );
    m_ml->reload();
}

static int Open( vlc_object_t* obj )
{
    vlc_medialibrary_t* p_module = reinterpret_cast<vlc_medialibrary_t*>( obj );

    try
    {
        p_module->p_sys = new MediaLibrary( obj );
    }
    catch ( const std::exception& ex )
    {
        msg_Err( obj, "Failed to instantiate/initialize medialibrary: %s", ex.what() );
        return VLC_EGENERIC;
    }
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
