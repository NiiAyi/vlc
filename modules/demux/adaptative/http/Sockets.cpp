/*
 * Sockets.cpp
 *****************************************************************************
 * Copyright (C) 2015 - VideoLAN authors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
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
#include "Sockets.hpp"

#include <vlc_network.h>
#include <cerrno>

using namespace adaptative::http;

Socket::Socket()
{
    netfd = -1;
}

Socket::~Socket()
{
    disconnect();
}

bool Socket::connect(vlc_object_t *stream, const std::string &hostname, int port)
{
    netfd = net_ConnectTCP(stream, hostname.c_str(), port);

    if(netfd == -1)
        return false;

    return true;
}

bool Socket::connected() const
{
    return (netfd != -1);
}

void Socket::disconnect()
{
    if (netfd >= 0)
    {
        net_Close(netfd);
        netfd = -1;
    }
}

ssize_t Socket::read(vlc_object_t *stream, void *p_buffer, size_t len)
{
    ssize_t size;
    do
    {
        size = net_Read(stream, netfd, NULL, p_buffer, len, true);
    } while (size < 0 && (errno == EINTR || errno==EAGAIN) );
    return size;
}

std::string Socket::readline(vlc_object_t *stream)
{
    char *line = ::net_Gets(stream, netfd, NULL);
    if(line == NULL)
        return "";
    std::string ret(line);
    ::free(line);
    return ret;
}

bool Socket::send(vlc_object_t *stream, const void *buf, size_t size)
{
    if (netfd == -1)
        return false;

    if (size == 0)
        return true;

    ssize_t ret = net_Write(stream, netfd, NULL, buf, size);
    if (ret <= 0)
        return false;

    if ( (size_t)ret < size )
        send( stream, ((uint8_t*)buf) + ret, size - ret );

    return true;
}

TLSSocket::TLSSocket() : Socket()
{
    creds = NULL;
    tls = NULL;
}

TLSSocket::~TLSSocket()
{
    disconnect();
}

bool TLSSocket::connect(vlc_object_t *stream, const std::string &hostname, int port)
{
    disconnect();
    if(!Socket::connect(stream, hostname, port))
        return false;

    creds = vlc_tls_ClientCreate(stream);
    if(!creds)
    {
        disconnect();
        return false;
    }

    tls = vlc_tls_ClientSessionCreate(creds, netfd, hostname.c_str(), "https", NULL, NULL);
    if(!tls)
    {
        disconnect();
        return false;
    }

    return true;
}

bool TLSSocket::connected() const
{
    return Socket::connected() && tls;
}

ssize_t TLSSocket::read(vlc_object_t *, void *p_buffer, size_t len)
{
    ssize_t size;
    size_t totalread = 0;
    do
    {
        size = tls_Recv(tls, (uint8_t*)p_buffer + totalread, len - totalread); /* only returns partial chunks */
        if(size >= 0)
        {
            totalread += (size_t) size;
        }
        else if(errno != EINTR && errno!=EAGAIN)
        {
            break;
        }
    } while ( totalread < len );
    return totalread;
}

std::string TLSSocket::readline(vlc_object_t *stream)
{
    std::string ret;
    ret.reserve(256);
    char c[2] = {0,0};
    ssize_t size = TLSSocket::read(stream, c, 1);

    while(size > 0)
    {
        ret.append( &c[0] );
        if(c[0] == '\n')
            break;

        size = TLSSocket::read(stream, c, 1);
    }

    return ret;
}

bool TLSSocket::send(vlc_object_t *stream, const void *buf, size_t size)
{
    if (!connected())
        return false;

    if (size == 0)
        return true;

    ssize_t ret = tls_Send(tls, buf, size);
    if (ret <= 0)
        return false;

    if ( (size_t)ret < size )
        send( stream, ((uint8_t*)buf) + ret, size - ret );

    return true;
}

void TLSSocket::disconnect()
{
    if(tls)
        vlc_tls_SessionDelete(tls);
    if(creds)
        vlc_tls_Delete(creds);
    tls = NULL;
    creds = NULL;
    Socket::disconnect();
}
