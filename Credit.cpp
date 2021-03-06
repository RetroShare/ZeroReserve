/*
    This file is part of the Zero Reserve Plugin for Retroshare.

    Zero Reserve is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zero Reserve is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Zero Reserve.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Credit.h"

#include "zrdb.h"
#include "Currency.h"
#include "ZeroReservePlugin.h"
#include "p3ZeroReserverRS.h"

#include <stdexcept>


void Credit::getCreditList( CreditList & outList, const std::string & id )
{
    ZrDB::Instance()->loadPeer( id, outList );
}



Credit::Credit( const std::string & id, const std::string & currencySym ) :
    m_id( id ),
    m_currency( currencySym )
{
    if( Currency::getCurrencyBySymbol( currencySym) == Currency::INVALID ){
        throw std::runtime_error( "Credit::Credit(): Invalid currency symbol" );
    }
}


ZR::RetVal Credit::allocate( const ZR::ZR_Number & amount)
{
    if( amount > getPeerAvailable() ){
        return ZR::ZR_FAILURE;
    }

    m_allocated += amount;
    ZrDB::Instance()->updatePeerCredit( *this, "allocation", m_allocated );
    return ZR::ZR_SUCCESS;
}


void Credit::deallocate( const ZR::ZR_Number & amount)
{
    m_allocated -= amount;
    try{
        ZrDB::Instance()->updatePeerCredit( *this, "allocation", m_allocated );
    }
    catch( std::exception e ){
        g_ZeroReservePlugin->placeMsg( std::string( "Exception caught: " ) + e.what() + " Cannot deallocate funds" );
    }
}


void Credit::updateCredit()
{
    try{
        ZrDB * db = ZrDB::Instance();
        if( !db->peerExists( *this )){
            db->createPeerRecord( *this );
        }
        db->updatePeerCredit( *this, "credit", m_credit );
    }
    catch( std::exception e ){
        g_ZeroReservePlugin->placeMsg( std::string( "Exception caught: " ) + e.what() + " Cannot update credit" );
    }
}

void Credit::updateOurCredit()
{
    try{
        ZrDB * db = ZrDB::Instance();
        if( !db->peerExists( *this )){
            db->createPeerRecord( *this );
        }
        db->updatePeerCredit( *this, "our_credit", m_our_credit );
    }
    catch( std::exception e ){
        g_ZeroReservePlugin->placeMsg( e.what() );
    }
}


void Credit::loadPeer()
{
        ZrDB::Instance()->loadPeer( *this );
}


void Credit::publish()
{
    p3ZeroReserveRS * p3zr = static_cast< p3ZeroReserveRS* >( g_ZeroReservePlugin->rs_pqi_service() );
    p3zr->sendCredit( this );
}
