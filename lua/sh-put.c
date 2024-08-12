#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

int dotlua_sh_put(lua_State *L) {
	const char *key = luaL_checkstring(L, 1);
	SHELL_VAR *temp_var = find_variable(key);
	if (lua_gettop(L) == 2) {
		const char *val = luaL_checkstring(L, 2);
		temp_var = bind_variable(key, strdup(val), 0);
	}
	set_auto_export(temp_var);
	return 0;
}