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
static lua_State *DotL;
static const char *prefix;

struct config {
	bool source_file; // default
	bool execute_string;
	bool call_function;
	bool raw_call; // without bind
	bool list_lua_binds;
	bool version, help;
	const char *optarg;
	WORD_LIST *list;
};

struct config config;
static const struct config config_init;

static int get_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	SHELL_VAR *svar = find_variable(sname);
	// check if svar is function, array or other unsupported type
	const char *svalue = svar ? value_cell(svar) : NULL;
	lua_pushstring(L, svalue); // Lua would check this internal
	return 1;
}

static int set_shell_var(lua_State *L) {
	const char *sname = luaL_checkstring(L, 1);
	const char *svalue = luaL_checkstring(L, 2);
	// check if svar is function, array or other unsupported type
	// check if svar is readonly
	bind_variable(sname, strdup(svalue), 0); // TODO ? bash manage this value?
	return 0;
}

static int put_shell_var(lua_State *L) {
	return 0;
}

// prepare sh.F table store all binds
static int bind_lua_func(lua_State *L) {
	const char *func = luaL_checkstring(L, 1);
	int n = lua_gettop(L); // 1 or 2
	if (n == 1) {
		if (lua_getglobal(L, func) != LUA_TFUNCTION) {
			builtin_error("%s: unknow lua function", func); // FIX error
		}
	} else {
		luaL_checktype(L, 2, LUA_TFUNCTION);
	}
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
    { "put", put_shell_var},
    { "get", get_shell_var},
    { "set", set_shell_var},
    {"bind", bind_lua_func},
    {"call", call_shell_fn},
    {  NULL,          NULL},
};

///////////////////////////////////////////////////////////////////////////////

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

static int dot_source_file(lua_State *L) {
	// use the first argument as filename
	const char *filename = config.list->word->word;
	if (access(filename, R_OK) != 0) {
		_log("%s: No Lua File\n", filename);
		return EXECUTION_FAILURE;
	}
	// [-0, +1, e]
	if (luaL_loadfile(L, filename) != LUA_OK) {
		_log("%s: Load Error\n", filename);
		goto lua_error;
	}
	WORD_LIST *list = config.list;
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
	if (lua_pcall(L, argc, 0, 0) != LUA_OK) {
		_log("%s: Exection Error\n", filename);
		goto lua_error;
	}
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	lua_settop(L, 0);
	return EXECUTION_FAILURE;
}

static int dot_call_function(lua_State *L) {
	const char *func = config.optarg;
	if (config.raw_call) {
		if (lua_getglobal(L, func) != LUA_TFUNCTION) {
			_log("%s: Not Lua Function(RAW)\n", func);
			goto lua_exit;
		}
	} else {
		lua_getglobal(L, prefix);
		lua_getfield(L, -1, "F");
		if (lua_getfield(L, -1, func) != LUA_TFUNCTION) {
			_log("%s: Not Lua Function(BIND)\n", func);
			goto lua_exit;
		}
	}
	int argc = 0;
	for (WORD_LIST *list = config.list; list; list = list->next) {
		// [-0, +1, e]
		lua_pushstring(L, list->word->word);
		argc++;
	}
	// [-(nargs + 1), +(nresults|1), –]
	if (lua_pcall(L, argc, 0, 0) != LUA_OK) {
		_log("%s: Function Error\n", func);
		goto lua_error;
	}

	// lua return code?
	lua_settop(L, 0);
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
lua_exit:
	lua_settop(L, 0);
	return EXECUTION_FAILURE;
}

static int dot_execute_string(lua_State *L) {
	const char *codestr = config.optarg;
	// [-0, +?, –]
	if (luaL_dostring(L, codestr) != LUA_OK) {
		_log("Code Error\n");
		goto lua_error;
	}
	lua_settop(L, 0);
	return EXECUTION_SUCCESS;

lua_error:
	// [-0, +0, e]
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	lua_settop(L, 0);
	return EXECUTION_FAILURE;
}

static int dot_list_lua_bind(lua_State *L) {
	const char *pattern = config.list ? config.list->word->word : "";
	lua_getglobal(L, prefix); // 1
	lua_getfield(L, -1, "F"); // 2
	lua_pushnil(L);           // 3
	while (lua_next(L, 2) != 0) {
		if (!lua_isstring(L, -2)) {
			continue;
		};
		if (!lua_isfunction(L, -1)) {
			continue;
		};
		const char *func = luaL_checkstring(L, -2);
		if (strstr(func, pattern)) {
			printf("%s\n", func);
		}
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	return EXECUTION_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

int dotlua_builtin(WORD_LIST *list) {
	int opt;
	config = config_init;
	reset_internal_getopt();
	while ((opt = internal_getopt(list, "hf:s:lrv")) != -1) {
		switch (opt) {
		case 'f':
			config.call_function = true;
			config.optarg = list_optarg;
			break;
		case 's':
			config.execute_string = true;
			config.optarg = list_optarg;
			break;
		case 'l':
			config.list_lua_binds = true;
			break;
		case 'r':
			config.raw_call = true;
			break;
		case 'v':
			config.version = true;
			break;
		case 'h':
		case '?':
			config.help = true;
			break;
			// fix --help
			CASE_HELPOPT;
		}
	}
	config.list = loptend;

	if (config.help) {
		builtin_usage();
		return EX_USAGE;
	}
	if (config.call_function) {
		return dot_call_function(DotL);
	}
	if (config.execute_string) {
		return dot_execute_string(DotL);
	}
	if (config.list_lua_binds) {
		return dot_list_lua_bind(DotL);
	}
	if (config.version) {
		printf("dotlua %s\n", VERSION); // define in meson
		return EXECUTION_SUCCESS;
	}
	return dot_source_file(DotL);
}

int dotlua_builtin_load(char *s) {
	(void)s;
	DotL = luaL_newstate();
	luaL_openlibs(DotL);
	luaL_newlib(DotL, dotlua); // +1
	lua_pushstring(DotL, "F");
	lua_newtable(DotL);
	lua_settable(DotL, 1); // -2
	prefix = getenv("DOTLUA_PREFIX");
	prefix = prefix ? prefix : "sh";
	lua_setglobal(DotL, prefix);
	return 1;
}

void dotlua_builtin_unload(char *s) {
	(void)s;
	lua_close(DotL);
}

char *dotlua_long_doc[] = {
    "",
    "dotlua help to run lua code with bash shell",
    "",
    "> dotlua FILE [ARGUMENTS ...]",
    "> dotlua [-r] -f FUNCTION [ARGUMENTS ...]",
    "> dotlua -s \"LUA STRING\"",
    "> dotlua -l [PATTERN]",
    "",
    NULL,
};

struct builtin dotlua_struct = {
    .name = "dotlua",
    .function = dotlua_builtin,
    .flags = BUILTIN_ENABLED,
    .long_doc = dotlua_long_doc,
    .short_doc = "dotlua FILE [ARGUMENTS ...]",
    .handle = 0,
};
