/*****************************************************************************
 * medialibrary.h: medialibrary module common declarations
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

#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include <medialibrary/IMediaLibrary.h>
#include <medialibrary/parser/IParserService.h>
#include <medialibrary/parser/IItem.h>
#include <medialibrary/parser/Parser.h>

#include <vlc_common.h>
#include <vlc_threads.h>
#include <vlc_input_item.h>
#include <vlc_input.h>

struct vlc_event_t;
struct vlc_object_t;

class Logger;

template <typename T>
inline std::unique_ptr<T, void (*)(void*)> wrapCPtr( T* ptr )
{
    static_assert( std::is_pointer<T>::value == false, "T must be a non pointer type" );
    return std::unique_ptr<T, decltype( &free )>( ptr, &free );
}

class MetadataExtractor : public medialibrary::parser::IParserService
{
private:
    struct ParseContext
    {
        ParseContext( MetadataExtractor* mde, medialibrary::parser::IItem& item )
            : inputItem( nullptr, &input_item_Release )
            , input( nullptr, &input_Close )
            , needsProbing( false )
            , mde( mde )
            , item( item )
        {
            vlc_mutex_init( &m_mutex );
            vlc_cond_init( &m_cond );
        }
        ~ParseContext()
        {
            vlc_cond_destroy( &m_cond );
            vlc_mutex_destroy( &m_mutex );
        }

        std::unique_ptr<input_item_t, decltype(&input_item_Release)> inputItem;
        std::unique_ptr<input_thread_t, decltype(&input_Close)> input;
        vlc_cond_t m_cond;
        vlc_mutex_t m_mutex;
        bool needsProbing;
        MetadataExtractor* mde;
        medialibrary::parser::IItem& item;
    };

public:
    MetadataExtractor( vlc_object_t* parent );
    virtual ~MetadataExtractor() = default;

    // All methods are meant to be accessed through IParserService, not directly
    // hence they are all private
private:
    virtual medialibrary::parser::Status run( medialibrary::parser::IItem& item ) override;
    virtual const char*name() const override;
    virtual uint8_t nbThreads() const override;
    virtual medialibrary::parser::Step targetedStep() const override;
    virtual bool initialize( medialibrary::IMediaLibrary* ml ) override;
    virtual void onFlushing() override;
    virtual void onRestarted() override;

    void onInputEvent( vlc_value_t event, ParseContext& ctx );
    void onSubItemAdded( const vlc_event_t* event, ParseContext& ctx );
    void populateItem( medialibrary::parser::IItem& item, input_item_t* inputItem );

    static int onInputEvent( vlc_object_t*, const char*, vlc_value_t,
                             vlc_value_t cur, void* data );
    static void onSubItemAdded( const vlc_event_t* event, void* data );

private:
    vlc_object_t* m_obj;
};

class MediaLibrary : public medialibrary::IMediaLibraryCb
{
public:
    MediaLibrary( vlc_object_t* obj );

private:
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<medialibrary::IMediaLibrary> m_ml;

    // IMediaLibraryCb interface
public:
    virtual void onMediaAdded(std::vector<medialibrary::MediaPtr> media) override;
    virtual void onMediaUpdated(std::vector<medialibrary::MediaPtr> media) override;
    virtual void onMediaDeleted(std::vector<int64_t> mediaIds) override;
    virtual void onArtistsAdded(std::vector<medialibrary::ArtistPtr> artists) override;
    virtual void onArtistsModified(std::vector<medialibrary::ArtistPtr> artists) override;
    virtual void onArtistsDeleted(std::vector<int64_t> artistsIds) override;
    virtual void onAlbumsAdded(std::vector<medialibrary::AlbumPtr> albums) override;
    virtual void onAlbumsModified(std::vector<medialibrary::AlbumPtr> albums) override;
    virtual void onAlbumsDeleted(std::vector<int64_t> albumsIds) override;
    virtual void onTracksAdded(std::vector<medialibrary::AlbumTrackPtr> tracks) override;
    virtual void onTracksDeleted(std::vector<int64_t> trackIds) override;
    virtual void onPlaylistsAdded(std::vector<medialibrary::PlaylistPtr> playlists) override;
    virtual void onPlaylistsModified(std::vector<medialibrary::PlaylistPtr> playlists) override;
    virtual void onPlaylistsDeleted(std::vector<int64_t> playlistIds) override;
    virtual void onDiscoveryStarted(const std::string& entryPoint) override;
    virtual void onDiscoveryProgress(const std::string& entryPoint) override;
    virtual void onDiscoveryCompleted(const std::string& entryPoint) override;
    virtual void onReloadStarted(const std::string& entryPoint) override;
    virtual void onReloadCompleted(const std::string& entryPoint) override;
    virtual void onEntryPointRemoved(const std::string& entryPoint, bool success) override;
    virtual void onEntryPointBanned(const std::string& entryPoint, bool success) override;
    virtual void onEntryPointUnbanned(const std::string& entryPoint, bool success) override;
    virtual void onParsingStatsUpdated(uint32_t percent) override;
    virtual void onBackgroundTasksIdleChanged(bool isIdle) override;
    virtual void onMediaThumbnailReady(medialibrary::MediaPtr media, bool success) override;
};

#endif // MEDIALIBRARY_H
