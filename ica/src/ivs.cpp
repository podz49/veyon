/*
 * ivs.cpp - implementation of IVS, a VNC-server-abstraction for platform-
 *           independent VNC-server-usage
 *
 * Copyright (c) 2006 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
 *  
 * This file is part of iTALC - http://italc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QStringList>


#include "ivs.h"


#ifdef BUILD_LINUX

extern "C" int x11vnc_main( int argc, char * * argv );

#include "rfb/rfb.h"
#include "isd_server.h"


qint64 libvncClientDispatcher( char * _buf, const qint64 _len,
				const socketOpCodes _op_code, void * _user )
{
	rfbClientPtr cl = (rfbClientPtr) _user;
	switch( _op_code )
	{
		case SocketRead:
			return( rfbReadExact( cl, _buf, _len ) == 1 ? _len :
									0 );
		case SocketWrite:
			return( rfbWriteExact( cl, _buf, _len ) == 1 ? _len :
									0 );
		case SocketGetIPBoundTo:
			strncpy( _buf, cl->host, _len );
			break;
	}
	return( 0 );

}


rfbBool isdNewClient( struct _rfbClientRec *, void * * )
{
	return( TRUE );
}


rfbBool isdHandleMessage( struct _rfbClientRec * _client, void * _data,
				const rfbClientToServerMsg * _message )
{
	if( _message->type == rfbItalcServiceRequest )
	{
		return( processItalcClient( libvncClientDispatcher, _client ) );
	}
	return( FALSE );
}


void isdAuthAgainstServer( struct _rfbClientRec * _client )
{
	if( authSecTypeItalc( libvncClientDispatcher, _client, ItalcAuthDSA ) )
	{
		_client->state = rfbClientRec::RFB_INITIALISATION;
	}
}



#elif BUILD_WIN32

extern int WinVNCAppMain( void );

#endif



IVS::IVS( const quint16 _ivs_port, int _argc, char * * _argv ) :
	QThread(),
	m_argc( _argc ),
	m_argv( _argv ),
	m_port( _ivs_port )
{
}




IVS::~IVS()
{
}




void IVS::run( void )
{
#ifdef BUILD_LINUX
	QStringList cmdline;
	// filter some options
	for( int i = 0; i < m_argc; ++i )
	{
		QString option = m_argv[i];
		if( option == "-noshm" || option == "-solid" ||
							option == "-xrandr" )
		{
			cmdline.append( m_argv[i] );
		}
	}
	cmdline/* << "-forever"	// do not quit after 1st conn.
		<< "-shared"	// allow multiple clients
		<< "-nopw"	// do not display warning
		<< "-repeat"	// do not disable autorepeat*/
		<< "-nosel"	// do not exchange clipboard-contents
		<< "-nosetclipboard"	// do not exchange clipboard-contents
/*		<< "-speeds" << ",5000,1"	// FB-rate: 7 MB/s
						// LAN: 5 MB/s
						// latency: 1 ms*/
/*		<< "-wait" << "25"	// time between screen-pools*/
/*		<< "-readtimeout" << "60" // timeout for disconn.*/
/*			<< "-threads"	// enable threaded libvncserver*/
/*		<< "-noremote"	// do not accept remote-cmds
		<< "-nocmds"	// do not run ext. cmds*/
		<< "-rfbport" << QString::number( m_port )
				// set port where the VNC-server should listen
		;
	char * old_av = m_argv[0];
	m_argv = new char *[cmdline.size()+1];
	m_argc = 1;
	m_argv[0] = old_av;

	for( QStringList::iterator it = cmdline.begin();
				it != cmdline.end(); ++it, ++m_argc )
	{
		m_argv[m_argc] = new char[it->length() + 1];
		strcpy( m_argv[m_argc], it->toAscii().constData() );
	}

	// register iTALC-protocol-extension
	rfbProtocolExtension pe;
	pe.newClient = isdNewClient;
	pe.init = NULL;
	pe.enablePseudoEncoding = NULL;
	pe.handleMessage = isdHandleMessage;
	pe.close = NULL;
	pe.usage = NULL;
	pe.processArgument = NULL;
	pe.next = NULL;
	rfbRegisterProtocolExtension( &pe );

	// register handler for iTALC's security-type
	rfbSecurityHandler sh = {
			rfbSecTypeItalc, isdAuthAgainstServer, NULL };
	rfbRegisterSecurityHandler( &sh );
	
	// run x11vnc-server
	x11vnc_main( m_argc, m_argv );
#elif BUILD_WIN32
	// run winvnc-server
	WinVNCAppMain();
#endif
}


