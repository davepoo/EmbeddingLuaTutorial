#pragma once
#include "lua.hpp"
#include <rttr/registration>

struct ArenaAllocator;

lua_State* CreateScript( ArenaAllocator& pool );
int LoadScript( lua_State* L, const char* script );
int ExecuteScript( lua_State* L );
void CloseScript( lua_State* L );

/*! \brief Takes the result and puts it onto the Lua stack
*	\return the number of values left on the stack. */
int ToLua( lua_State* L, rttr::variant& result );

inline int PutOnLuaStack( lua_State* )
{
	return 0;
}

template< typename T >
inline int PutOnLuaStack( lua_State* L, T& toPutOnStack )
{
	rttr::type typeOfT = rttr::type::get<T>();
	if ( typeOfT.is_class() )
	{
		//pass-by-reference
		rttr::variant v( &toPutOnStack );
		return ToLua( L, v );
	}
	else
	{
		//pass-by-value
		rttr::variant v( toPutOnStack );
		return ToLua( L, v );
	}
}

template< typename T, typename... T2 >
inline int PutOnLuaStack( lua_State* L, T& toPutOnStack, T2&... moreArgs )
{
	return PutOnLuaStack( L, toPutOnStack ) + PutOnLuaStack( L, moreArgs... );
}

template< typename... ARGS >
inline void CallScriptFunction( lua_State* L, const char* funcName, ARGS&... args )
{
	lua_getglobal( L, funcName );
	if ( lua_type( L, -1 ) == LUA_TFUNCTION )
	{
		int numArgs = PutOnLuaStack( L, args... );
		if ( lua_pcall( L, numArgs, 0, 0 ) != 0 )
		{
			printf( "unable to call script function '%s', '%s'\n", funcName, lua_tostring( L, -1 ) );
			luaL_error( L, "unable to call script function '%s', '%s'", funcName, lua_tostring( L, -1 ) );
		}
	}
	else
	{
		printf( "unknown script function '%s'\n", funcName );
		luaL_error( L, "unknown script function '%s'", funcName );
	}
}
