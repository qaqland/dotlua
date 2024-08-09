#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

int dotlua_sh_get(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_variable(sname);
	// check if svar is function, array or other unsupported type
	const char *svalue = svar ? value_cell(svar) : NULL;
	lua_pushstring(L, svalue); // lua check this, nil if NULL
	return 1;
}
