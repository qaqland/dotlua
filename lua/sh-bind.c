#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

extern const char *prefix;

int dotlua_sh_bind(lua_State *L) {
	const char *func = luaL_checkstring(L, 1);
	int n = lua_gettop(L); // 1 or 2
	if (n == 1) {
		lua_getglobal(L, func);
	}
	luaL_checktype(L, 2, LUA_TFUNCTION);

	COMMAND *cmd = make_bare_simple_command();
	WORD_LIST *list = xmalloc(sizeof(*list));
	list->next = NULL;
	cmd->value.Simple->words = list;
	const char *tmp[] = {"dotlua", "-f", func, "$@", NULL};
	for (const char **ptr = tmp; *ptr; ptr++) {
		if (list == list->next) {
			list->next = xmalloc(sizeof(*list));
			list = list->next;
		}
		list->word = make_word(*ptr);
		list->next = list;
	}
	list->next = NULL;
	bind_function(func, cmd);

	lua_getglobal(L, prefix); // 3
	lua_getfield(L, 3, "F");  // 4
	lua_insert(L, 1);         // put sh.F at 1
	lua_pop(L, 1);            // pop sh
	lua_settable(L, 1);
	return 0;
}
