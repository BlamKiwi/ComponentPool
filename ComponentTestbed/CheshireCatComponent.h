#pragma once

struct CheshireCatComponent
{
	CheshireCatComponent( );

	CheshireCatComponent( size_t owner, bool smiling );


	void Update( const float dt );

	size_t _owner;

	bool _smile;

};