#include "AutomatedBinding.h"
#include "ArenaAllocator.h"
#include "lua.hpp"
#include <cstdio>
#include <assert.h>
#include <rttr/registration>

/*! \brief The Lua script, you would probably load this data from a .lua file. */
constexpr char* LUA_SCRIPT = R"(
		-- this is a lua script
		Global.HelloWorld()
		Global.HelloWorld2()
		local c = Global.Add( 42, 43 )
		local d = Global.Mul( c, 2 )
		Global.HelloWorld3( d, 99, 111 )
		local spr = Sprite.new()
		spr:Draw()
		)";

int CallGlobalFromLua(lua_State* L)
{
	rttr::method* m = (rttr::method*)lua_touserdata(L, lua_upvalueindex(1));
	rttr::method& methodToInvoke(*m);
	rttr::array_range<rttr::parameter_info> nativeParams = methodToInvoke.get_parameter_infos();
	int numLuaArgs = lua_gettop(L);
	int numNativeArgs = (int)nativeParams.size();
	if (numLuaArgs != numNativeArgs)
	{
		printf("Error calling native function '%s', wrong number of args, expected %d, got %d\n",
			methodToInvoke.get_name().to_string().c_str(), numNativeArgs, numLuaArgs );
		assert(numLuaArgs == numNativeArgs);
	}
	union PassByValue
	{
		int intVal;
		short shortVal;
	};

	std::vector<PassByValue> pbv(numNativeArgs);
	std::vector<rttr::argument> nativeArgs(numNativeArgs);
	auto nativeParamsIt = nativeParams.begin();
	for (int i = 0; i < numLuaArgs; i++, nativeParamsIt++)
	{
		const rttr::type nativeParamType = nativeParamsIt->get_type();
		int luaArgIdx = i + 1;
		int luaType = lua_type(L, luaArgIdx);
		switch (luaType)
		{
		case LUA_TNUMBER:
			if (nativeParamType == rttr::type::get<int>())
			{
				pbv[i].intVal = (int)lua_tonumber(L, luaArgIdx);
				nativeArgs[i] = pbv[i].intVal;
			}
			else if (nativeParamType == rttr::type::get<short>())
			{
				pbv[i].shortVal = (short)lua_tonumber(L, luaArgIdx);
				nativeArgs[i] = pbv[i].shortVal;
			}
			else
			{
				printf("unrecognised parameter type '%s'\n", nativeParamType.get_name().to_string().c_str());
				assert(false);
			}
			break;
		default:
			assert(false); //dont know this type.
			break;
		}
	}
	rttr::variant result = methodToInvoke.invoke_variadic({}, nativeArgs);
	int numberOfReturnValues = 0;
	if (result.is_valid() == false)
	{
		luaL_error(L, "unable to invoke '%s'\n", methodToInvoke.get_name().to_string().c_str());
	}
	else if (result.is_type<void>() == false)
	{
		if (result.is_type<int>())
		{
			lua_pushnumber(L, result.get_value<int>());
			numberOfReturnValues++;
		}
		else if (result.is_type<short>())
		{
			lua_pushnumber(L, result.get_value<short>());
			numberOfReturnValues++;
		}
		else
		{
			luaL_error(L,
				"unhandled return type '%s' from native method '%s'\n",
				result.get_type().get_name().to_string().c_str(),
				methodToInvoke.get_name().to_string().c_str());
		}
	}
	return numberOfReturnValues;
}

/*! \return The meta table name for type t */
std::string MetaTableName( const rttr::type& t )
{
	std::string metaTableName = t.get_name().to_string();
	metaTableName.append("_MT_");
	return metaTableName;
}

int CreateUserDatum(lua_State* L)
{
	const char* typeName = (const char*)lua_tostring(L, lua_upvalueindex(1));
	rttr::type typeToCreate = rttr::type::get_by_name(typeName);

	void* ud = lua_newuserdata(L, sizeof(rttr::variant) );
	new (ud) rttr::variant(typeToCreate.create());
	//rttr::variant& variant = *(rttr::variant*)ud;

	luaL_getmetatable(L, MetaTableName(typeToCreate).c_str());
	lua_setmetatable(L, 1);

	lua_newtable(L);
	lua_setuservalue(L, 1);

	return 1;	//return the userdatum
}

int DestroyUserDatum(lua_State* L)
{
	rttr::variant* ud = (rttr::variant*)lua_touserdata(L, -1);
	ud->~variant();
	return 0;
}

int InvokeFuncOnUserDatum(lua_State* L)
{
	rttr::method& m = *(rttr::method*)lua_touserdata(L, lua_upvalueindex(1));
	if (lua_isuserdata(L, 1) == false)
	{
		luaL_error(L, "Expected a userdatum on the lua stack when invoking native method '%s'", m.get_name().to_string().c_str());
	}

	rttr::variant& ud = *(rttr::variant*)lua_touserdata(L, 1);
	luaL_error(L, "TODO: need to pass parameters to native native method '%s'", m.get_name().to_string().c_str());
	rttr::variant result = m.invoke(ud);
	if (result.is_valid() == false)
	{
		luaL_error(L, "Failed to invoke native native method '%s'", m.get_name().to_string().c_str());
	}

	luaL_error(L, "TODO: need to return values from native native method '%s'", m.get_name().to_string().c_str());
	return 0;
}

int IndexUserDatum(lua_State* L)
{
	const char* typeName = (const char*)lua_tostring(L, lua_upvalueindex(1));
	rttr::type typeInfo = rttr::type::get_by_name(typeName);
	if (lua_isuserdata(L, 1) == false)
	{
		luaL_error(L, "Expected a userdatum on the lua stack when indexing native type '%s'", typeName);
	}

	if (lua_isstring(L, -1) == false)
	{
		luaL_error(L, "Expected a name of a native property or method when indexing native type '%s'", typeName);
	}

	//rttr::variant& ud = *(rttr::variant*)lua_touserdata(L, 1);
	const char* fieldName = lua_tostring(L, -1);
	rttr::method m = typeInfo.get_method(fieldName);
	if (m.is_valid())
	{
		void* methodUD = lua_newuserdata(L, sizeof(rttr::method));
		new (methodUD) rttr::method(m);
		lua_pushcclosure(L, InvokeFuncOnUserDatum, 1);
		return 1;
	}

	luaL_error(L, "TODO: need to check to see if '%s' is a property of  '%s'", fieldName, typeName);

	return 0;
}

void AutomatedBindingTutorial()
{
	printf("---- automated binding using run time type info -----\n");

	//create memory pool for Lua
	constexpr int POOL_SIZE = 1024 * 20;
	char memory[POOL_SIZE];
	ArenaAllocator pool(memory, &memory[POOL_SIZE - 1]);

	//open the Lua state using our memory pool
	lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);

	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "Global");

	//binding global methods
	lua_pushvalue(L, -1);											//1
	for (auto& method : rttr::type::get_global_methods() )
	{
		lua_pushstring(L, method.get_name().to_string().c_str());	//2
		lua_pushlightuserdata(L, (void*)&method);
		lua_pushcclosure(L, CallGlobalFromLua, 1);					//3 
		lua_settable(L, -3);										//1[2] = 3
	}

	//binding classes to Lua
	for (auto& classToRegister : rttr::type::get_types())
	{
		if (classToRegister.is_class())
		{
			const std::string s = classToRegister.get_name().to_string();
			const char* typeName = s.c_str();

			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setglobal(L, classToRegister.get_name().to_string().c_str());

			lua_pushvalue(L, -1);
			lua_pushstring(L, typeName);
			lua_pushcclosure(L, CreateUserDatum, 1);
			lua_setfield(L, -2, "new");

			//create the metatable & metamethods for this type
			luaL_newmetatable(L, MetaTableName(classToRegister).c_str());
			lua_pushstring(L, "__gc");
			lua_pushcfunction(L, DestroyUserDatum);
			lua_settable(L, -3);

			lua_pushstring(L, "__index");
			lua_pushstring(L, typeName);
			lua_pushcclosure(L, IndexUserDatum, 1 );
			lua_settable(L, -3);
		}
	}

	//execute the lua script
	int doResult = luaL_dostring(L, LUA_SCRIPT);
	if (doResult != LUA_OK)
	{
		printf("Error: %s\n", lua_tostring(L, -1));
	}

	//close the Lua state
	lua_close(L);
}