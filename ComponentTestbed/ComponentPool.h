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

#include <stdexcept>
#include <type_traits>

#include "CRCBPool.h"
#include "ComponentReference.h"

/**
*	Manages an object pool of components in a cache coherent manner for the update tick.
*
*	Assumes components are freely movable.
*/
template <typename ComponentType>
class ComponentPool
{
	// Assert assumptions over component type
	static_assert( std::is_nothrow_move_assignable<ComponentType>::value, "Component classes must be nothrow move assignable." );
	static_assert( std::is_nothrow_destructible<ComponentType>::value, "Component classes must be nothrow destructible." );

	// Assert assumptions over CRCBs
	static_assert( std::is_nothrow_move_assignable<ComponentReferenceControlBlock>::value, "ComponentReferenceControlBlock must be nothrow move assignable." );
	static_assert( std::is_nothrow_destructible<ComponentReferenceControlBlock>::value, "ComponentReferenceControlBlock must be nothrow destructible." );
	static_assert( std::is_nothrow_constructible<ComponentReferenceControlBlock, void*>::value, "ComponentReferenceControlBlock must be nothrow constructible when taking a component pointer." );

	/**< The maximum amount of components pools should allow for. */
	static const size_t MAX_COMPONENTS = 1000;

public:

	/**
	*	Constructs a component pool.
	*/
	inline ComponentPool( )
		: _pending_changes_head( nullptr )
		, _num_active_components( 0 )
		, _num_sleeping_components( 0 )
	{

	}

	/**
	*	Destroys a component pool.
	*/
	inline ~ComponentPool( )
	{
		// Destroy any remaining components allocated
		const auto count = Count( );
		for ( auto i = 0U; i < count; i++ )
			_components [ i ].~ComponentType( );
	}

	/**
	*	Updates all active components in the component pool.
	*
	*	@param dt the time since the last frame
	*/
	inline void Update( const float dt )
	{
		const auto count = Count( ); // Hoist loop check
		for ( auto i = _num_sleeping_components; i < count; i++ )
			_components [ i ].Update( dt );
	}

	/**
	*	Creates a new active component.
	*
	*	@param args the constructor arguments for the component
	*	@returns the component reference for the component
	*/
	template <typename... Args>
	inline ComponentReference<ComponentType> Create( Args &&... args )
	{
		// Hoist constants
		const auto count = Count( );

		// Can we create any more components 
		if ( count == MAX_COMPONENTS )
			throw std::runtime_error( "Allocation limit reached for component store." );

		// Grab a component reference control block (may throw)
		auto control_block = _control_block_pool.Allocate( 1 );

		// Grab a component
		auto component = std::addressof( _components [ count ] );

		// Construct the new component (may throw)
		try
		{
			new ( component ) ComponentType( std::forward<Args>( args )... );
		}
		catch ( ... )
		{
			// Return the control block
			_control_block_pool.Deallocate( control_block, 1 );

			// Rethrow construct exception
			throw;
		}
		
		// Set the control block data
		_control_block_pool.Construct( control_block, component );

		// Update id table to point to the new control block
		_control_table [ count ] = control_block;

		// We now have a valid component, update count
		++_num_active_components;

		// Return the control_block for the component
		return ComponentReference<ComponentType>( control_block );
	}

	/**
	*	Sets the active state of the component. Changes will be applied at the end of the frame.
	*
	*	@param component the component to change the active state of
	*	@param new_active the desired active state for the component
	*/
	inline void SetActive(const ComponentReference<ComponentType> &component, const bool new_active)
	{
		// Check if the given reference is valid
		if ( !component.IsValid( ) )
			throw std::runtime_error( "Component reference was invalid." );

		// Extract context from reference
		auto context = component._context;

		// Ensure control block actually belongs to this pool
		if ( !_control_block_pool.IsPointerValid( context ) )
			throw std::runtime_error( "Tried to set active state of component that did not belong to this component pool." );

		// If component active state is already in desired state, do nothing
		if ( new_active == context->IsComponentActive( ) )
			return;

		// Add component to the pending changes list (if we haven't done so already)
		if ( !context->IsPendingChanges( ) )
		{
			// Point the new head to the current head
			context->_next = _pending_changes_head;

			// Update the current head to the new head
			_pending_changes_head = context;
		}

		// Mark component for pending changes
		context->MarkActiveStateChange( new_active );
	}

	/**
	*	Sets the component to be deleted. Changes will be applied at the end of the frame.
	*
	*	@param component the component to delete
	*/
	inline void Delete( const ComponentReference<ComponentType> &component )
	{
		// Check if the given reference is valid
		if ( !component.IsValid( ) )
			throw std::runtime_error( "Component reference was invalid." );

		// Extract context from reference
		auto context = component._context;

		// Ensure control block actually belongs to this pool
		if ( !_control_block_pool.IsPointerValid( context ) )
			throw std::runtime_error( "Tried to delete component that did not belong to this component pool." );

		// Add component to the pending changes list (if we haven't done so already)
		if ( !context->IsPendingChanges( ) )
		{
			// Point the new head to the current head
			context->_next = _pending_changes_head;

			// Update the current head to the new head
			_pending_changes_head = context;
		}

		// Mark component for pending changes
		context->MarkForDeletion( );
	}

	/**
	*	Applies pending changes to the component pool. 
	*
	*	It is assumed at this stage we cannot invalidate any cached component pointers.
	*	EG: A this pointer in a method call. 
	*/
	inline void LateUpdate()
	{
		// For every element of the pending changes list
		while ( _pending_changes_head)
		{
			// Pop item from the list
			auto const control = _pending_changes_head;

			// Advance the head of the list
			_pending_changes_head = _pending_changes_head->_next;

			/**
			*	Pending changes have an ordering. 
			*
			*	1. Deletion (Dominates other changes)
			*	2. Active State Changes
			*/
			if ( control->IsPendingDeletion( ) )
			{
				// Delete the component
				DeleteInternal( *control );

				// Clean up the control block now that the component has been removed
				_control_block_pool.Destroy( control );

				// Reclaim the control block
				_control_block_pool.Deallocate( control, 1 );
			}
			else if (control->IsPendingActiveStateChange())
			{
				// Apply pending active state change
				SetActiveInternal(*control, control->GetPendingActiveStateChange());

				// Clear any pending changes flags
				control->ClearPendingChanges( );
			}
		}
	}

private:

	/**
	*	Delete the component.
	*/
	inline void DeleteInternal( ComponentReferenceControlBlock &context )
	{
		// Make sure the component is active so we can move it to the end for deletion
		SetActiveInternal( context, true );

		// Get the typed component pointer
		const auto *component = static_cast< ComponentType* >( context._component );

		// Get the index of the component in the awake block
		const auto loc_index = component - std::addressof( _components [ 0 ] );

		// Get the index of the last component in the awake block
		const auto target_index = Count( ) - 1;

		// Move the component to the end of the awake block
		SwapComponents( loc_index, target_index );

		// Call destructor on the component
		_components [ target_index ].~ComponentType( );

		// We now have one less active component
		_num_active_components--;
	}

	/**
	*	Sets the active state of the component.
	*/
	inline void SetActiveInternal(ComponentReferenceControlBlock &context, const bool new_active)
	{
		// If component active state is already in desired state, do nothing
		if (new_active == context.IsComponentActive())
			return;

		// Get the typed component pointer
		const auto *component = static_cast<ComponentType*>(context._component);

		// Are we waking the component up?
		if (new_active)
		{
			// Get the index of the component in the sleeping block
			const auto loc_index = component - std::addressof( _components [ 0 ] );

			// Get the index of the last component in the sleeping block
			const auto target_index = _num_sleeping_components - 1;

			// Move the component to the end of the sleeping block
			SwapComponents( loc_index, target_index );

			// Update the counters
			_num_sleeping_components--;
			_num_active_components++;

		}
		// Are we putting the component to sleep?
		else
		{
			// Get the index of the component in the active block
			const auto loc_index = component - std::addressof( _components [ _num_sleeping_components ] );

			// Get the index of the first component in the active block
			const auto target_index = _num_sleeping_components;

			// Move the component to the beginning of the active block
			SwapComponents( loc_index, target_index );

			// Update the counters
			_num_sleeping_components++;
			_num_active_components--;
		}

		// Set new active flag for component
		context.SetComponentActive(new_active);
	}


	/**
	*	Swaps two components in memory and updates the control table.
	*/
	inline void SwapComponents( size_t loc_index, size_t target_index )
	{
		// Ensure we are actually moving the component
		if ( loc_index != target_index )
		{
			// Swap components and control table entries
			std::swap( _components [ loc_index ], _components [ target_index ] );
			std::swap( _control_table [ loc_index ]->_component, _control_table [ target_index ]->_component );
			std::swap( _control_table [ loc_index ], _control_table [ target_index ] );
		}
	}

	/**
	*	Gets the number of components in the component pool.
	*/
	inline size_t Count( ) const
	{
		return _num_active_components + _num_sleeping_components;
	}

	/**< The control block object pool. */
	CRCBPool<MAX_COMPONENTS> _control_block_pool;

	/**< The component object pool. */
	ComponentType _components [ MAX_COMPONENTS ];

	/**< The control block table that maps component locations to reference control blocks. */
	ComponentReferenceControlBlock* _control_table [ MAX_COMPONENTS ];

	/**< The head of the pending changes list. */
	ComponentReferenceControlBlock *_pending_changes_head;

	/**< The number of active components in the component pool. */
	size_t _num_active_components;

	/**< The number of sleeping components in the component pool. */
	size_t _num_sleeping_components;
};