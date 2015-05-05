#include "stdafx.h"

/*
* Copyright (c) 2015, Missing Box Studio
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ComponentReferenceControlBlock.h"

#include <utility>

ComponentReferenceControlBlock::ComponentReferenceControlBlock( )
	: _next( nullptr )
	, _component( nullptr )
	, _tag( 0 )
	, _flags( 0 )
	
{

}

ComponentReferenceControlBlock::~ComponentReferenceControlBlock( )
{
	// Clear component state
	_component = nullptr;
	_flags = 0;
	_next = nullptr;

	// Increment garbage detection tag
	_tag++;
}

ComponentReferenceControlBlock::ComponentReferenceControlBlock( void* component )
	: _next( nullptr )
	, _component( component )
	, _flags( IS_ACTIVE )
	
	/* Garbage detection tag is only changed on initial contruction and any future destructions. */
{

}

bool ComponentReferenceControlBlock::IsComponentActive() const
{
	return _flags & IS_ACTIVE;
}

void ComponentReferenceControlBlock::SetComponentActive(const bool new_active)
{
	// Get the new bit value
	FlagType f = new_active ? IS_ACTIVE : 0;

	// Clear and set the new bit value
	_flags &= ~IS_ACTIVE;
	_flags |= f;
}

void ComponentReferenceControlBlock::MarkActiveStateChange(const bool new_active)
{
	// Get the new bit value
	FlagType f = new_active ? PENDING_ACTIVE : PENDING_SLEEP;

	// Set the pending flag
	_flags |= f;
}

bool ComponentReferenceControlBlock::IsPendingChanges() const
{
	return (_flags & PENDING_ACTIVE) || (_flags & PENDING_SLEEP) || (_flags & PENDING_DELETE);
}

bool ComponentReferenceControlBlock::IsPendingActiveStateChange() const
{
	return (_flags & PENDING_ACTIVE) || (_flags & PENDING_SLEEP);
}
 
bool ComponentReferenceControlBlock::GetPendingActiveStateChange() const
{
	return _flags & PENDING_ACTIVE;
}

void ComponentReferenceControlBlock::ClearPendingChanges()
{
	_flags &= ~(PENDING_ACTIVE | PENDING_SLEEP | PENDING_DELETE);
}

void ComponentReferenceControlBlock::MarkForDeletion( )
{
	_flags |= PENDING_DELETE;
}

bool ComponentReferenceControlBlock::IsPendingDeletion( ) const
{
	return _flags & PENDING_DELETE;
}

size_t ComponentReferenceControlBlock::GetGarbageTag( ) const
{
	return _tag;
}

void* ComponentReferenceControlBlock::GetComponentPtr( ) const
{
	return _component;
}