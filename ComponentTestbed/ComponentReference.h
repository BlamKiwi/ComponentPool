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

class ComponentService;
#include "ComponentReferenceControlBlock.h"

/**
*	Managed references for components.
*/
template <typename ComponentType>
class ComponentReference final
{
	/**< Allow component pools to get at the component reference control block. */
	template <typename T>
	friend class ComponentPool;

public:

	/**
	*	Constructs a new component reference.
	*/
	inline ComponentReference( )
		: _context( nullptr )
		, _control_block_tag( 0 )
	{

	}

	/**
	*	Constructs a new component reference.
	*
	*	@param context the control block for this component
	*/
	inline ComponentReference( ComponentReferenceControlBlock *context )
		: _context( context )
		, _control_block_tag( context->GetGarbageTag( ) ) // Copy the tag at the time of reference instantiation
	{ 

	}

	/**
	*	Constructs a null component reference.
	*/
	inline ComponentReference( std::nullptr_t )
		: ComponentReference( )
	{

	}

	/**
	*	Copy constructor for component references.
	*/
	inline ComponentReference( const ComponentReference & ) = default;

	/**
	*	Move constructor for component references.
	*
	*	@param other the reference to move
	*/
	inline ComponentReference( ComponentReference &&other )
		: _context( other._context )
		, _control_block_tag( other._control_block_tag )
	{
		// Clear other
		other._context = nullptr;
		other._control_block_tag = 0;
	}

	/**
	*	Copy assignment operator for component references.
	*/
	ComponentReference& operator=( const ComponentReference& ) = default;

	/**
	*	Move assignment operator for component references.
	*
	*	@param other the reference to move
	*/
	ComponentReference& operator=( ComponentReference&& other )
	{
		// Check tghat other is a unique object
		if ( this != std::addressof( other ) )
		{
			// Copy values from other
			*this = other;

			// Clear values from other
			other._context = nullptr;
			other._control_block_tag = 0;
		}

		return *this;
	}

	/**
	*	Null assignment operator for component references.
	*/
	ComponentReference& operator=( std::nullptr_t )
	{
		_context = nullptr;
		_control_block_tag = 0;

		return *this;
	}

	/**
	*	Gets a raw pointer to the managed component.
	*/
	inline ComponentType* Get( )
	{
		// Check that the reference is valid before trying to dereference it
		if ( !IsValid( ) )
			throw std::runtime_error( "Tried to dereference component but reference has been invalidated." );

		// Return type discovered component pointer
		return static_cast<ComponentType*>( _context->GetComponentPtr( ) );
	}

	/**
	*	Gets a raw pointer to the managed component.
	*/
	inline const ComponentType* Get( ) const
	{
		// Check that the reference is valid before trying to dereference it
		if ( !IsValid( ) )
			throw std::runtime_error( "Tried to dereference component but reference has been invalidated." );

		// Return type discovered component pointer
		return const_cast< const ComponentType* >( static_cast< ComponentType* >( _context->GetComponentPtr( ) ) );
	}

	/**
	*	Arrow operator.
	*/
	inline ComponentType* operator->( )
	{
		return Get( );
	}

	/**
	*	Dereference operator.
	*/
	inline ComponentType& operator*( )
	{
		return *Get( );
	}

	/**
	*	Arrow operator.
	*/
	inline const ComponentType* operator->( ) const
	{
		return Get( );
	}

	/**
	*	Dereference operator.
	*/
	inline const ComponentType& operator*( ) const
	{
		return *Get( );
	}

	/**
	*	Bool cast operator.
	*/
	inline operator bool( ) const
	{
		return IsValid( );
	}

	/**
	*	Gets if the component reference is still valid.
	*	Component references are valid until they are deallocated to be reused later
	*	and are not null.
	*/
	inline bool IsValid( ) const
	{
		// The reference is valid while we have a context pointer, 
		// the cached tag and control block tag match
		return _context && _control_block_tag == _context->GetGarbageTag( );
	}

private:

	/**< Component control block for this referece. */
	ComponentReferenceControlBlock *_context;

	/**< The control block tag at the time of being given a control block. */
	size_t _control_block_tag;
};

namespace std
{
	/**
	*	Specializes std::hash so that we always hash over the raw pointer for component references.
	*/
	template <class ComponentType>
	struct hash < ComponentReference<ComponentType> >
	{
		size_t operator( )( const ComponentReference<ComponentType> &ref) const
		{
			return std::hash<ComponentType*>( ref.Get( ) );
		}
	};

	/**
	*	Specializes std::swap for Component References.
	*/
	template <class ComponentType>
	void swap( ComponentReference<ComponentType> &lhs, ComponentReference<ComponentType>&rhs )
	{
		ComponentReference<ComponentType> temp( std::move( lhs ) );
		lhs = std::move( rhs );
		rhs = std::move( temp );
	}
}

/**
*	Specializes output stream operator for component references.
*/
template <class ComponentType, class U, class V>
std::basic_ostream<U, V>& operator<<( std::basic_ostream<U, V>& os, const ComponentReference<ComponentType>& ref )
{
	os << ref.Get( );
	return os;
}

/**
*	Equality and Inequality oeprators for component references.
*/

template < class T, class U >
bool operator==( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) == rhs.Get( );
}

template< class T, class U >
bool operator!=( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) != rhs.Get( );
}

template< class T, class U >
bool operator<( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) < rhs.Get( );
}

template< class T, class U >
bool operator>( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) > rhs.Get( );
}

template< class T, class U >
bool operator<=( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) <= rhs.Get( );
}

template< class T, class U >
bool operator>=( const ComponentReference<T>& lhs, const ComponentReference<U>& rhs )
{
	return lhs.Get( ) >= rhs.Get( );
}

template< class T >
bool operator==( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) == rhs;
}

template< class T >
bool operator==( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs == rhs.Get( );
}

template< class T >
bool operator!=( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) == rhs;
}

template< class T >
bool operator!=( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs != rhs.Get( );
}

template< class T >
bool operator<( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) < rhs;
}

template< class T >
bool operator<( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs < rhs.Get( );
}

template< class T >
bool operator<=( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) <= rhs;
}

template< class T >
bool operator<=( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs <= rhs.Get( );
}

template< class T >
bool operator>( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) > rhs
}

template< class T >
bool operator>( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs > rhs.Get( );
}

template< class T >
bool operator>=( const ComponentReference<T>& lhs, std::nullptr_t rhs )
{
	return lhs.Get( ) >= rhs;
}

template< class T >
bool operator>=( std::nullptr_t lhs, const ComponentReference<T>& rhs )
{
	return lhs >= rhs.Get( );
}