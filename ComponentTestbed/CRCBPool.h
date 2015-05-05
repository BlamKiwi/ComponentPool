#pragma once

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

#include <array>
#include <stdexcept>

#include "ComponentReferenceControlBlock.h"

/**
*	A basic CompoentnReferenceControlBlock pool.
*/
template <size_t POOL_SIZE>
class CRCBPool
{
public:
#pragma region

	typedef ComponentReferenceControlBlock ValueType;
	typedef ValueType* Pointer;
	typedef const ValueType* ConstPointer;
	typedef ValueType& Reference;
	typedef const ValueType& ConstReference;
	typedef std::size_t SizeType;
	typedef std::ptrdiff_t DifferenceType;

#pragma endregion Allocator Typedefs

	/**
	*	Constructs a CompoentnReferenceControlBlock pool.
	*/
	inline explicit CRCBPool( )
	{
		// Setup pool stack
		_pool_head = std::addressof( _pool [ 0 ] );
		for ( SizeType i = 0; i < POOL_SIZE - 1; i++ )
			_pool [ i ]._next = std::addressof( _pool [ i + 1 ] );
	}

	/**
	*	Destroys a CompoentnReferenceControlBlock pool.
	*/
	inline ~CRCBPool( ) { }

	/**
	*	Pool copying is forbidden.
	*
	*	Operation is not well defined.
	*	EG: What happens to already allocated pool members? How do the copies get returned if they are copied?
	*/
	inline explicit CRCBPool( CRCBPool const& ) = delete;

#pragma region 

	/**
	*	Gets the address of the given reference.
	*/
	inline Pointer Address( Reference r ) const { return std::addressof( r ); }

	/**
	*	Gets the address of the given reference.
	*/
	inline ConstPointer Address( ConstReference r ) const { return std::addressof( r ); }

	/**
	*	Gets if the given pointer is valid (belongs to and is of expected alignment) for this allocator.
	*/
	inline bool IsPointerValid( ConstPointer ptr ) const
	{
		// Get the offset of the given pointer into the pool
		const DifferenceType offset = ptr - std::addressof( _pool [ 0 ] );
		
		return !( !ptr || // Null pointers can never be valid
			( offset < 0 || offset >= static_cast< DifferenceType >( POOL_SIZE ) ) || // Ensure the provided pointer actually belongs in this pool
			( std::addressof( _pool [ offset ] ) != ptr ) ); // Ensure provided pointer has proper alignment in the pool
	}

#pragma endregion Address Helpers

#pragma region 

	/**
	*	Tries to allocate an object from the ObjectPool.
	*/
	inline Pointer Allocate( SizeType num_objects )
	{
		// Ensure the amount of requested objects is 1
		if ( num_objects != 1 )
			throw std::runtime_error( "ObjectPools only support allocating one object at a time." );

		// Ensure we have items left in the pool to assign
		if( !_pool_head )
			throw std::runtime_error( "Tried to allocate an object from the ObjectPool but was empty." );

		// Pop an item from the ObjectPool
		auto *item = _pool_head;
		_pool_head = _pool_head->_next;

		// Return the popped item
		return item;
	}

	/**
	*	Returns the given pool item back to the pool.
	*/
	void Deallocate( Pointer pool_item, SizeType num_objects )
	{
		// Ensure the amount of objects returned is only 1
		if ( num_objects != 1 )
			throw std::runtime_error( "ObjectPools only support deallocating one object at a time" );

		// Ensure the given pool item pointer is valid
		if ( !IsPointerValid( pool_item ) )
			throw std::runtime_error( "Provided pointer to deallocate did not appear to be valid for the ObjectPool" );
		
		// Return item to pool
		pool_item->_next = _pool_head;
		_pool_head = pool_item;
	}

#pragma endregion Memory Allocation

#pragma region

	/**
	*	Returns the maximum theoretically possible value of n, for which the call allocate(n) could succeed.
	*
	*	@returns the maximum number of objects we can allocate storage for at once
	*/
	inline SizeType MaxSize( ) const
	{
		return 1;
	}

#pragma endregion Size

#pragma region 

	/**
	*	Constructs the item for the given pointer.
	*/
	template <typename... Args>
	inline void Construct( Pointer p, Args... args ) const { new ( p ) ValueType( std::forward<Args>( args )... ); };

	/**
	*	Destroys the item for the given pointer.
	*/
	inline void Destroy( Pointer p ) const { p->~ComponentReferenceControlBlock( ); }

#pragma endregion Construction/Destruction

#pragma region

	/**
	*	Checks for equality between the given allocators.
	*/
	inline bool operator==( CRCBPool const& a ) const
	{
		// Is this allocator the same instance
		return this == std::addressof( a );
	}

	/**
	*	Checks for inequality between the given allocators.
	*/
	inline bool operator!=( CRCBPool const& a ) const
	{
		return !operator==( a );
	}

#pragma endregion Equality Comparisons

private:

	/**< The ObjectPool pool head. */
	ComponentReferenceControlBlock* _pool_head;

	/**< The ObjectPool pool. */
	std::array<ComponentReferenceControlBlock, POOL_SIZE> _pool;
};