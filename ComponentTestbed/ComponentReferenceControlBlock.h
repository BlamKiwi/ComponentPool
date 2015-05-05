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

#include <cstdint>
#include <type_traits>

/**
*	The control block for component references.
*/
class ComponentReferenceControlBlock final
{
	/**< Allow component pools to get to the next pointer and component pointer. */
	template <typename T>
	friend class ComponentPool;

	/**< Allow the CRCBPools to get at the next pointer. */
	template <size_t MAX_SIZE>
	friend class CRCBPool;

public:
	/**
	*	Creates a component reference control block and sets its initial global state.
	*/
	ComponentReferenceControlBlock( );

	/**
	*	Initializes the control block.
	*
	*	@param component the pointer ot the component
	*/
	ComponentReferenceControlBlock( void * component ) _NOEXCEPT;

	/**
	*	Cleans up a component reference control block.
	*
	*	Garbage tag is incremented.
	*/
	~ComponentReferenceControlBlock( ) _NOEXCEPT;

	/**
	*	Signals the control block that the component we manage has been deleted.
	*/
	void ComponentDeleted( );

	/**
	*	Checks to see if the component we manage is active.
	*
	*	@returns true if the component is active.
	*/
	bool IsComponentActive() const;

	/**
	*	Sets the active state to the given value.
	*
	*	@param new_active the new active state for the component
	*/
	void SetComponentActive(const bool new_active);

	/**
	*	Marks the component to have its active state changed at the end of the update tick. 
	*
	*	@param new_active the new desired active state.
	*/
	void MarkActiveStateChange(const bool new_active);

	/**
	*	Gets if the component has any pending changes. 
	*
	*	@returns true if the component has any pending changes.
	*/
	bool IsPendingChanges() const;

	/**
	*	Gets if the component has a pending active state change.
	*
	*	@returns true if the component has a pending active state change.
	*/
	bool IsPendingActiveStateChange() const;

	/**
	*	Clears any pending changes for the component.
	*/
	void ClearPendingChanges();

	/**
	*	Marks the component to be deleted.
	*/
	void MarkForDeletion( );

	/**
	*	Gets if the component has been marked for deletion.
	*/
	bool IsPendingDeletion( ) const;

	/**
	*	Gets the pending active state change.
	*
	*	@returns true if the component should become active, false if we should put it to sleep
	*/
	bool GetPendingActiveStateChange() const;

	/**
	*	Gets the current garbage tag of the control block.
	*
	*	@returns the current garbage tag
	*/
	size_t GetGarbageTag( ) const;

	/**
	*	Gets the current component pointer that the control block manages.
	*
	*	@returns the current cached component pointer
	*/
	void* GetComponentPtr( ) const;

private:

	/**< The next component control block in the pending changes list or control block free list. */
	ComponentReferenceControlBlock *_next;

	/**< The component. */
	void *_component;

	/**< Control block tag. Gets incremented each time the component we point to is deleted. NOTE THIS STATE IS PERSISTENT ACROSS REUSES TO DETECT GARBAGE. */
	size_t _tag;

	/**< Component state flags. */
	typedef uint8_t FlagType;
	FlagType _flags;
	
	/**< State flag names, */
	enum StateFlags : FlagType { 
		IS_ACTIVE = 1, // Is the component active
		PENDING_ACTIVE = 2, // Should we make the component active
		PENDING_SLEEP = 4, // Should we make the component sleep
		PENDING_DELETE = 8 // Should we delete the component
	};
};