/*****************************************************************************
 * vlc_media_library.h: SQL-based media library
 *****************************************************************************
 * Copyright (C) 2008-2010 the VideoLAN Team and AUTHORS
 * $Id$
 *
 * Authors: Antoine Lejeune <phytos@videolan.org>
 *          Jean-Philippe André <jpeg@videolan.org>
 *          Rémi Duraffort <ivoire@videolan.org>
 *          Adrien Maglo <magsoft@videolan.org>
 *          Srikanth Raju <srikiraju at gmail dot com>
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

#ifndef VLC_MEDIA_LIBRARY_H
# define VLC_MEDIA_LIBRARY_H

# ifdef __cplusplus
extern "C" {
# endif

typedef enum ml_sorting_criteria
{
    ML_SORT_DEFAULT,
} ml_sorting_criteria;

typedef struct ml_search_filters_t ml_search_filters_t;
struct ml_search_filters_t
{
    const char* psz_pattern;
    ml_sorting_criteria i_sort;
    uint32_t i_nbResults;
    uint32_t i_offset;
    bool b_desc;
};

typedef struct ml_list_filters_t ml_list_filters_t;
struct ml_list_filters_t
{
    ml_sorting_criteria i_sort;
    uint32_t i_nbResults;
    uint32_t i_offset;
    bool b_desc;
};

typedef struct vlc_medialibrary_t vlc_medialibrary_t;
struct vlc_medialibrary_t
{
    struct vlc_common_members obj;
    void* p_sys;

    int (*pf_control)( vlc_medialibrary_t* p_ml, int i_query, ... );
    /**
     * Search for some medialibrary entries.
     *
     * \param p_ml The medialibrary module instance.
     * \param i_query The type search to be performed. \see ml_search enumeration
     * \param p_filter A pointer to a search filter structure. Cannot be NULL.
     *
     * Refer to the individual list of ml_search requests for the additional
     * per-query parameters.
     */
    int (*pf_search)( vlc_medialibrary_t* p_ml, int i_query,
                      const ml_search_filters_t* p_filter, ... );
    /**
     * List some entities from the medialibrary.
     *
     * \param p_ml The medialibrary module instance.
     * \param i_query The type search to be performed. \see ml_list enumeration
     * \param p_filter A pointer to a list filter structure, or NULL for the default
     * filters (alphabetical ascending sort, no pagination)
     *
     * Refer to the individual list of ml_search requests for the additional
     * per-query parameters.
     */
    int (*pf_list)( vlc_medialibrary_t* p_ml, int i_query,
                    const ml_list_filters_t* p_filter, ... );
};

typedef struct ml_entrypoint_t ml_entrypoint_t;
struct ml_entrypoint_t
{
    char* psz_mrl; /**< This entrypoint's MRL. Will be NULL if b_present is false */
    bool b_present; /**< The presence state for this entrypoint. */
    bool b_banned; /**< Will be true if the user required this entrypoint to be excluded */
};

enum ml_control
{
    /* Adds a folder to discover through the medialibrary */
    ML_ADD_FOLDER,              /**< arg1: mrl (const char*)  res: can't fail */
    ML_REMOVE_FOLDER,           /**< arg1: mrl (const char*)  res: can't fail */
    ML_BAN_FOLDER,              /**< arg1: mrl (const char*)  res: can't fail */
    ML_UNBAN_FOLDER,            /**< arg1: mrl (const char*)  res: can't fail */
    ML_LIST_FOLDERS,            /**< arg1: entrypoints (ml_entrypoint_t**); arg2: nb results(size_t*), res: can fail */

    /* Pause/resume background operations, such as media discovery & media analysis */
    ML_PAUSE_BACKGROUND,        /**< no args; can't fail */
    ML_RESUME_BACKGROUND,       /**< no args; can't fail */

    /* Misc operations */
    ML_CLEAR_HISTORY,           /**< no args; can't fail */
};

enum ml_search
{
    /* Search all videos known by the medialibrary */
    ML_SEARCH_VIDEOS,           /**< No args;   res: can fail */
    ML_SEARCH_AUDIOS,           /**< No args;   res: can fail */
    ML_SEARCH_ALBUMS,           /**< No args;   res: can fail */
};

enum ml_list
{
    /* List tracks of an album.             arg1: The album id. res: can fail */
    ML_LIST_ALBUM_TRACKS,
    /* Lit the albums of an artist.         arg1: The artist id. res: can fail */
    ML_LIST_ARTIST_ALBUMS,
};

static inline void vlc_ml_add_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
    p_ml->pf_control( p_ml, ML_ADD_FOLDER, psz_folder );
}

static inline void vlc_ml_remove_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
    p_ml->pf_control( p_ml, ML_REMOVE_FOLDER, psz_folder );
}

static inline void vlc_ml_ban_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
    p_ml->pf_control( p_ml, ML_BAN_FOLDER, psz_folder );
}

static inline void vlc_ml_unban_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
    p_ml->pf_control( p_ml, ML_UNBAN_FOLDER, psz_folder );
}

static inline int vlc_ml_list_folder( vlc_medialibrary_t* p_ml,
                                      ml_entrypoint_t** pp_entrypoints, size_t* p_nb_items )
{
    return p_ml->pf_control( p_ml, ML_LIST_FOLDERS, pp_entrypoints, p_nb_items );
}

static inline void vlc_ml_pause_background( vlc_medialibrary_t* p_ml )
{
    p_ml->pf_control( p_ml, ML_PAUSE_BACKGROUND );
}

static inline void vlc_ml_resume_background( vlc_medialibrary_t* p_ml )
{
    p_ml->pf_control( p_ml, ML_RESUME_BACKGROUND );
}

static inline  void vlc_ml_clear_history( vlc_medialibrary_t* p_ml )
{
    p_ml->pf_control( p_ml, ML_CLEAR_HISTORY );
}

VLC_API void vlc_ml_entrypoints_release( ml_entrypoint_t* p_list, size_t i_nb_items );

/*****************************************************************************
 * ML Enums
 *****************************************************************************/

#define ML_PERSON_ARTIST        "Artist"
#define ML_PERSON_ALBUM_ARTIST  "Album Artist"
#define ML_PERSON_ENCODER       "Encoder"
#define ML_PERSON_PUBLISHER     "Publisher"


/** List of Query select types.
 * In a query array or variable argument list, each select type is followed
 * by an argument (X) of variable type (char* or int, @see ml_element_t).
 * These types can be used either in the query list or in the result array.
 * Some types are reserved for the result array:
 */
typedef enum
{
    ML_ALBUM = 1,              /**< Album Title */
    ML_ALBUM_ID,               /**< Album ID */
    ML_ALBUM_COVER,            /**< Album Cover art url */
    /* FIXME: Remove ML_ARTIST */
    ML_ARTIST,                 /**< Artist, interpreted as ML_PEOPLE
                                    && ML_PEOPLE_ROLE = ML_PERSON_ARTIST */
    ML_ARTIST_ID,              /**< Artist ID, interpreted as ML_PEOPLE_ID
                                    && ML_PEOPLE_ROLE = ML_PERSON_ARTIST */
    ML_COMMENT,                /**< Comment about media */
    ML_COUNT_MEDIA,            /**< Number of medias */
    ML_COUNT_ALBUM,            /**< Number of albums */
    ML_COUNT_PEOPLE,           /**< Number of people */
    ML_COVER,                  /**< Cover art url */
    ML_DURATION,               /**< Duration in ms */
    ML_DISC_NUMBER,            /**< Disc number of the track */
    ML_EXTRA,                  /**< Extra/comment (string) on the media */
    ML_FIRST_PLAYED,           /**< First time media was played */
    ML_FILESIZE,               /**< Size of the media file */
    ML_GENRE,                  /**< Genre of the media (if any) */
    ML_ID,                     /**< Media ID */
    ML_IMPORT_TIME,            /**< Date when media was imported */
    ML_LANGUAGE,               /**< Language */
    ML_LAST_PLAYED,            /**< Last play UNIX timestamp */
    ML_LAST_SKIPPED,           /**< Time when media was last skipped */
    ML_ORIGINAL_TITLE,         /**< Media original title (if any) */
    ML_PEOPLE,                 /**< Any People associated with this media */
    ML_PEOPLE_ID,              /**< Id of a person */
    ML_PEOPLE_ROLE,            /**< Person role */
    ML_PLAYED_COUNT,           /**< Media play count */
    ML_PREVIEW,                /**< Url of the video preview */
    ML_SKIPPED_COUNT,          /**< Number of times skipped */
    ML_SCORE,                  /**< Computed media score */
    ML_TITLE,                  /**< Media title */
    ML_TRACK_NUMBER,           /**< Media track number (if any) */
    ML_TYPE,                   /**< Media type. @see ml_type_e */
    ML_URI,                    /**< Media full URI. */
    ML_VOTE,                   /**< Media user vote value */
    ML_YEAR,                   /**< Media publishing year */
    ML_DIRECTORY,              /**< Monitored directory */
    ML_MEDIA,                  /**< Full media descriptor. @see ml_media_t */
    ML_MEDIA_SPARSE,           /**< Sparse media. @see ml_media_t */
    ML_MEDIA_EXTRA,            /**< Sparse + Extra = Full media */

    /* Some special elements */
    ML_LIMIT     = -1,         /**< Limit a query to X results */
    ML_SORT_DESC = -2,         /**< Sort a query descending on argument X */
    ML_SORT_ASC  = -3,         /**< Sort a query ascending on argument X */
    ML_DISTINCT  = -4,         /**< Add DISTINCT to SELECT statements. */
    ML_END       = -42         /**< End of argument list */
} ml_select_e;

/** Media types (audio, video, etc...) */
typedef enum
{
    ML_UNKNOWN   = 0,       /**< Unknown media type */
    ML_AUDIO     = 1 << 0,  /**< Audio only media */
    ML_VIDEO     = 1 << 1,  /**< Video media. May contain audio channels */
    ML_STREAM    = 1 << 2,  /**< Streamed media = not a local file */
    ML_NODE      = 1 << 3,  /**< Nodes like simple nodes, directories, playlists, etc */
    ML_REMOVABLE = 1 << 4,  /**< Removable media: CD/DVD/Card/... */
} ml_type_e;

/** Query result item/list type: integers, strings, medias, timestamps */
typedef enum {
    ML_TYPE_INT,        /**< Object is an int */
    ML_TYPE_PSZ,        /**< A string char* */
    ML_TYPE_TIME,       /**< A timestamp mtime_t */
    ML_TYPE_MEDIA,      /**< A pointer to a media ml_media_t* */
} ml_result_type_e;

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* VLC_MEDIA_LIBRARY_H */
