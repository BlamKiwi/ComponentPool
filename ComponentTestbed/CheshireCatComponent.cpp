#include "stdafx.h"

#include <iostream>
#include "CheshireCatComponent.h"


CheshireCatComponent::CheshireCatComponent( ) { }

CheshireCatComponent::CheshireCatComponent( size_t owner, bool smiling )
	: _owner( owner )
	, _smile( smiling )
{

}

void CheshireCatComponent::Update( const float )
{
	std::cout << "CheshireCatComponent: Owner=" << _owner << " ";
	if ( _smile )
		std::cout << "A sinister grin emerges from the trees.";
	else
		std::cout << "Alice senses a presense in the dark.";
	std::cout << std::endl;
}