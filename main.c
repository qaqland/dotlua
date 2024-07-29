#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// please put <execute_cmd.h> the last
#include <execute_cmd.h>

// keep status like bash's source
// LL -> Long Live
static lua_State *LL;

static int get_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_variable(sname);
	const char *svalue = svar ? value_cell(svar) : NULL;
	lua_pushstring(L, svalue); // Lua would check this internal
	return 1;
}

static int set_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	const char *svalue = luaL_checkstring(L, 2);
	bind_variable(sname, strdup(svalue), 0); // TODO ? bash manage this value?
	return 0;
}

static int bind_lua_func(lua_State *L) {
	const char *func = luaL_checkstring(L, 1);
	if (lua_getglobal(L, func) != LUA_TFUNCTION) {
		builtin_error("%s: unknow lua function", func); // FIX error
	}
	COMMAND *cmd = make_bare_simple_command();
	WORD_LIST *list = xmalloc(sizeof(*list));
	list->next = NULL;
	cmd->value.Simple->words = list;
	const char *prefix[] = {"dotlua", "-f", func, "$@", NULL};
	for (const char **ptr = prefix; *ptr; ptr++) {
		if (list == list->next) {
			list->next = xmalloc(sizeof(*list));
			list = list->next;
		}
		list->word = make_word(*ptr);
		list->next = list;
	}
	list->next = NULL;
	bind_function(func, cmd);
	return 0;
}

static int call_shell_fn(lua_State *L) {
	int n = lua_gettop(L);
	const char *func = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_function(func);
	if (!svar) {
		builtin_error("%s: unknow shell function", func); // FIX error
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
	lua_pushinteger(L, retval);
	return 1;
}

static const struct luaL_Reg dotlua[] = {
	{ "get", get_shell_var},
    { "set", set_shell_var},
    {"bind", bind_lua_func},
	{"call", call_shell_fn},
    {  NULL,          NULL},
};

///////////////////////////////////////////////////////////////////////////////

enum DOTLUA_RUN_TYPE {
	IS_FILE,
	IS_FUNCTION,
	IS_STRING,
};

static const char *_now() {
	static char buff[20];
	time_t now = time(0);
	struct tm *sTm = gmtime(&now); // not localtime
	strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
	return buff;
}

static void _log(const char *fmt, ...) {
	fprintf(stderr, "[dotlua] %s \x1b[31m>>>\x1b[0m ", _now());
	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
}

static int run_file(lua_State *L, WORD_LIST *list) {
	// use the first argument as filename
	const char *filename = list->word->word;
	if (access(filename, R_OK) != 0) {
		_log("%s: No Lua File\n", filename);
		return EXECUTION_FAILURE;
	}
	// [-0, +1, e]
	if (luaL_loadfile(LL, filename) != LUA_OK) {
		_log("%s: Load Error\n", filename);
		goto lua_error;
	}
	// skip first --
	if (list->next && !strcmp(list->next->word->word, "--")) {
		list = list->next;
	}
	int argc = 0;
	for (list = list->next; list; list = list->next) {
		lua_pushstring(L, list->word->word);
		argc++;
	}
	// [-(nargs + 1), +(nresults|1), –]
	if (lua_pcall(LL, argc, 0, 0) != LUA_OK) {
		_log("%s: Exection Error\n", filename);
		goto lua_error;
	}
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(LL, -1));
	lua_settop(LL, 0);
	return EXECUTION_FAILURE;
}

static int run_function(lua_State *L, WORD_LIST *list) {
	const char *func = list->word->word;
	// [-0, +1, e]
	if (lua_getglobal(L, func) != LUA_TFUNCTION) {
		_log("%s: Not Lua Function\n", func);
		goto lua_error;
	}
	int argc = 0;
	for (list = list->next; list; list = list->next) {
		// [-0, +1, e]
		lua_pushstring(L, list->word->word);
		argc++;
	}
	// [-(nargs + 1), +(nresults|1), –]
	if (lua_pcall(L, argc, 1, 0) != LUA_OK) {
		_log("%s: Function Error\n", func);
		goto lua_error;
	}
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(LL, -1));
	lua_settop(LL, 0);
	return EXECUTION_FAILURE;
}

static int run_string(lua_State *L, WORD_LIST *list) {
	const char *codestr = list->word->word;
	// [-0, +?, –]
	if (luaL_dostring(LL, codestr) != LUA_OK) {
		_log("Code Error\n");
		goto lua_error;
	}
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(LL, -1));
	lua_settop(LL, 0);
	return EXECUTION_FAILURE;
}

///////////////////////////////////////////////////////////////////////////////

int dotlua_builtin(WORD_LIST *list) {
	int opt;
	int run_type = IS_FILE;
	reset_internal_getopt();
	while ((opt = internal_getopt(list, "fshv")) != -1) {
		switch (opt) {
		case 'f':
			run_type = IS_FUNCTION;
			break;
		case 's':
			run_type = IS_STRING;
			break;
		case 'v':
			goto exit_version;
		case 'h':
		case '?':
			goto exit_usage;
		}
	}
	list = loptend;

	if (!list) {
		builtin_error("argument required");
		goto exit_usage;
	}
	switch (run_type) {
	case IS_FILE:
		return run_file(LL, list);
	case IS_FUNCTION:
		return run_function(LL, list);
	case IS_STRING:
		return run_string(LL, list);
	}

exit_usage:
	builtin_usage();
	return EX_USAGE;
exit_version:
	printf("dotlua %s\n", VERSION); // define in meson
	return EXECUTION_SUCCESS;
}

int dotlua_builtin_load(char *s) {
	(void)s;

	LL = luaL_newstate();
	luaL_openlibs(LL);
	luaL_newlib(LL, dotlua);
	const char *prefix = getenv("DOTLUA_PREFIX");
	lua_setglobal(LL, prefix ? prefix : "sh");
	return 1;
}

void dotlua_builtin_unload(char *s) {
	(void)s;
	lua_close(LL);
}

char *dotlua_long_doc[] = {
	"DotLua help to run lua code with bash shell",
	"",
	"dotlua xxx.lua [args]",
	"dotlua -s \"lua code string\"",
	"dotlua -f function [args]",
	NULL,
};

struct builtin dotlua_struct = {
	.name = "dotlua",
	.function = dotlua_builtin,
	.flags = BUILTIN_ENABLED,
	.long_doc = dotlua_long_doc,
	.short_doc = "dotlua xxx.lua [args]",
	.handle = 0,
};
