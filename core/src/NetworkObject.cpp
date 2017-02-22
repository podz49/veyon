/*
 * NetworkObject.cpp - data class representing a network object
 *
 * Copyright (c) 2017 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "NetworkObject.h"


const QUuid NetworkObject::networkObjectNamespace( "8a6c479e-243e-4ccb-8e5a-0ddf5cf3c7d0" );


NetworkObject::NetworkObject( const NetworkObject &other ) :
	m_type( other.type() ),
	m_name( other.name() ),
	m_hostAddress( other.hostAddress() ),
	m_macAddress( other.macAddress() ),
	m_directoryAddress( other.directoryAddress() ),
	m_uid( other.uid() )
{
}



NetworkObject::NetworkObject( NetworkObject::Type type,
							  const QString& name,
							  const QString& hostAddress,
							  const QString& macAddress,
							  const QString& directoryAddress ) :
	m_type( type ),
	m_name( name ),
	m_hostAddress( hostAddress ),
	m_macAddress( macAddress ),
	m_directoryAddress( directoryAddress ),
	m_uid( calculateUid() )
{
}



bool NetworkObject::operator ==( const NetworkObject& other ) const
{
	return uid() == other.uid();
}



NetworkObject::Uid NetworkObject::calculateUid() const
{
	// if a directory address is set (e.g. full DN in LDAP) it should be unique and can be
	// used for hashing
	if( directoryAddress().isEmpty() == false )
	{
		return QUuid::createUuidV5( networkObjectNamespace, directoryAddress() );
	}

	return QUuid::createUuidV5( networkObjectNamespace, name() + hostAddress() + macAddress() );
}