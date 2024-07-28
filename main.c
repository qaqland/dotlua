#include <assert.h>
#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <execute_cmd.h>

// keep status like bash's source
lua_State *L;

static int get_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_variable(sname);
	const char *svalue = svar ? value_cell(svar) : NULL;
	lua_pushstring(L, svalue);
	return 1; // TODO why 1
}

static int set_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	const char *svalue = luaL_checkstring(L, 2);
	bind_variable(sname, strdup(svalue), 0); // TODO ?
	return 0;
}

static int bind_lua_func(lua_State *L) {
	const char *func = luaL_checkstring(L, 1);
	lua_getglobal(L, func);
	if (!lua_isfunction(L, -1)) {
		builtin_error("%s: unknow lua function", func);
	}
	COMMAND *cmd = make_bare_simple_command();
	WORD_LIST *list = xmalloc(sizeof(*list));
	list->next = NULL;
	cmd->value.Simple->words = list;
	const char *prefix[] = {"dotlua", "-f", func, "$@", NULL};
	for (const char **ptr = prefix; *ptr; ptr++) {
		// printf("make cmd: %s\n", *ptr);
		// fflush(stdout);
		if (list == list->next) { // TODO maybe bug here
			list->next = xmalloc(sizeof(*list));
			list = list->next;
		}
		list->word = make_word(*ptr);
		list->next = list;
	}
	list->next = NULL;
	// print_word_list(cmd->value.Simple->words, " ");
	bind_function(func, cmd);
	return 0;
}

static int call_lua_func(lua_State *L, WORD_LIST *list) {
	WORD_LIST *ptr = list;
	const char *func = ptr->word->word;
	lua_getglobal(L, func);
	if (!lua_isfunction(L, -1)) {
		builtin_error("%s: unknow lua function", func);
	}
	int argc = 0;
	for (ptr = ptr->next; ptr; ptr = ptr->next) {
		lua_pushstring(L, ptr->word->word);
		argc++;
	}
	int result = lua_pcall(L, argc, 1, 0);
	if (result != 0) {
		printf("lua failed: %s\n", lua_tostring(L, -1));
		return EXECUTION_FAILURE;
	}
	switch (lua_type(L, -1)) {
	case LUA_TNIL:
		break;
	default:
		// printf("return %s", lua_tostring(L, -1));
		break;
	}
	return EXECUTION_SUCCESS;
}

static int call_shell_fn(lua_State *L) {
	int n = lua_gettop(L);
	const char *func = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_function(func);
	if (!svar) {
		builtin_error("%s: unknow shell function", func);
		return 0;
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
	if (retval) {
		builtin_error("%s: execute_shell_function error %d", func, retval);
	}
	return 1;
}

static const struct luaL_Reg dotlua[] = {
	{"getshvar", get_shell_var},
	{"setshvar", set_shell_var},
	{"callshfn", call_shell_fn},
	{"bindfunc", bind_lua_func},
	{NULL, NULL}};

int dotlua_builtin(WORD_LIST *list) {
	int opt;
	bool is_func_call = false;
	reset_internal_getopt();
	while ((opt = internal_getopt(list, "f")) != -1) {
		switch (opt) {
		case 'f':
			is_func_call = true;
			break;
		}
	}

	list = loptend;

	if (list == NULL) {
		builtin_error("argument required");
		builtin_usage();
		return EX_USAGE;
	}

	if (is_func_call) {
		return call_lua_func(L, list);
	}

	const char *filename = list->word->word;
	// free(fname);

	if (access(filename, R_OK) != 0) {
		builtin_error("%s: file not found or no read permission", filename);
		return EXECUTION_FAILURE;
	}

	// TODO -- pass argument to lua

	int result;
	result = luaL_loadfile(L, filename);
	assert(result == 0);

	result = lua_pcall(L, 0, 0, 0);
	if (result != 0) {
		printf("lua failed: %s\n", lua_tostring(L, -1));
		return EXECUTION_FAILURE;
	}

	return EXECUTION_SUCCESS;
}

int dotlua_builtin_load(char *s) {
	if (current_user.user_name == 0) {
		get_current_user_info();
	}
	printf("dotlua builtin loaded -- %s\n", current_user.user_name);
	fflush(stdout);

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L, dotlua);
	lua_setglobal(L, "dotlua");

	return 1;
}

void dotlua_builtin_unload(char *s) {
	printf("dotlua unload\n");
	fflush(stdout);
	lua_close(L);
}

char *dotlua_long_doc[] = {
	[0] = "dotlua help to run lua with bash shell",
	[1] = "",
	"dotlua xxx.lua",
	"dotlua -f function",
	NULL};

struct builtin dotlua_struct = {
	.name = "dotlua",
	.function = dotlua_builtin,
	.flags = BUILTIN_ENABLED,
	.long_doc = dotlua_long_doc,
	.short_doc = "dotlua xxx.lua",
	.handle = 0};
