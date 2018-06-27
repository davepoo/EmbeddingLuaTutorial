--[[
    - How to use the Lua C API
    - Bind Native C++ Object and use them in Lua
    - Assume you don't know anything about the Lua C API
    - Learning by Doing
    - Code is all in github 
]]
--[[
    - You should know
        - How to script in Lua (although you don't need to be an expert at all)
        - How to code in C++ / C++11
]]

--[[
- Going to use 
    - Lua 5.3.4, 
    - Windows, 
    - CMake & Visual Studio 2015 Community
    - You can use CMake to build in any way you want (VS2017, XCode...)
]]

--[[
== Contents ==
- download lua and use cmake to make a lua application and compile it
- create lua state and execute a lua code inline
- explaination of the types in lua
- explaination of the lua stack
- get a global variable from lua
- set a global variable to lua
- call a lua function
- call a lua function and get the result(s)
- call a lua function with parameters and get the result(s)
- call a c function from lua
- call a c function from lua with a return value
- call a c function from lua with a return value and parameters
- creating your own type for lua, userdata
- Creating C++ objects in lua
- Calling C++ class methods in lua
- using tables, getting settings values from a table in lua
- setting values in a table in lua
- giving data to lua via globals
- metatables
- metatables on userdata
- uservalues
- memory allocation in lua
- the registry
- calling C functions with upvalues
- ?? lightuserdata
- Runtime type information
- Generic Binding of C++/C to Lua
]]--