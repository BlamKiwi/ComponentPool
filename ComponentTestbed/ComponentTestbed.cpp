// ComponentTestbed.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "ComponentPool.h"
#include "CheshireCatComponent.h"

int _tmain(int, _TCHAR*[])
{
	ComponentPool <CheshireCatComponent> test_manager;

	auto cat1 = test_manager.Create( 1, true );
	auto cat2 = test_manager.Create( 2, false );
	auto cat3 = test_manager.Create( 3, false );

	test_manager.Update( 0.25f );
	test_manager.LateUpdate( );

	test_manager.SetActive( cat3, false );

	test_manager.Update( 0.25f );
	test_manager.LateUpdate( );

	test_manager.Delete( cat2 );

	test_manager.Update( 0.25f );
	test_manager.LateUpdate( );

	test_manager.SetActive( cat3, true );

	test_manager.Update( 0.25f );
	test_manager.LateUpdate( );

	int x;
	std::cin >> x;

	return 0;
}

