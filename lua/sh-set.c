#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

int dotlua_sh_set(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	const char *svalue = luaL_checkstring(L, 2);
	// check if svar is function, array or other unsupported type
	// check if svar is readonly
	bind_variable(sname, strdup(svalue), 0); // TODO ? bash manage this value?
	return 0;
}