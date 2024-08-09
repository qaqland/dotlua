#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

// please keep <execute_cmd.h> the last
#include <execute_cmd.h>

int dotlua_sh_call(lua_State *L) {
	int n = lua_gettop(L);
	const char *func = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_function(func);
	if (!svar) {
		return luaL_error(L, "%s: unknow shell function", func);
	}
	WORD_LIST *list = xmalloc(sizeof(*list));
	WORD_LIST *head = list;
	list->word = make_word(func);
	for (int i = 2; i <= n; i++) {
		const char *arg = lua_tostring(L, i);
		list->next = xmalloc(sizeof(*list));
		list = list->next;
		list->word = make_word(arg);
	}
	list->next = NULL;
	int retval = execute_shell_function(svar, head);
	lua_pushinteger(L, retval);
	return 1;
}
