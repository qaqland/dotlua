#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

// check_unbind_variable
int dotlua_sh_delete(lua_State *L) {
	const char *name = luaL_checkstring(L, 1);
	SHELL_VAR *v = find_variable(name);
	if (!v) {
		return 0;
	}
	if (readonly_p(v)) {
		return luaL_error(L, "%s: cannot unset: readonly %s", name, "variable");
	}
	if (non_unsettable_p(v)) {
		return luaL_error(L, "%s: cannot unset", name);
	}
	unbind_variable(name);
	return 0;
}