#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <lauxlib.h>
#include <loadables.h>
#include <lua.h>
#include <lualib.h>

// keep status like bash's source
static lua_State *DotL;
const char *prefix;

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

static struct config config;
static const struct config config_init;

extern int dotlua_sh_put(lua_State *L);
extern int dotlua_sh_get(lua_State *L);
extern int dotlua_sh_set(lua_State *L);
extern int dotlua_sh_delete(lua_State *L);
extern int dotlua_sh_call(lua_State *L);
extern int dotlua_sh_bind(lua_State *L);

static const struct luaL_Reg dotlua[] = {
    {   "put",    dotlua_sh_put},
    {   "get",    dotlua_sh_get},
    {   "set",    dotlua_sh_set},
    {"delete", dotlua_sh_delete},
    {  "bind",   dotlua_sh_bind},
    {  "call",   dotlua_sh_call},
    {    NULL,             NULL},
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
	putchar('\n');
}

static int dot_source_file(lua_State *L) {
	// use the first argument as filename
	const char *filename = config.list->word->word;
	if (access(filename, R_OK) != 0) {
		_log("%s: No Lua File", filename);
		return EXECUTION_FAILURE;
	}
	if (luaL_loadfile(L, filename) != LUA_OK) {
		_log("%s: Load Error", filename);
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
	if (lua_pcall(L, argc, 0, 0) != LUA_OK) {
		_log("%s: Exection Error", filename);
		goto lua_error;
	}
	return EXECUTION_SUCCESS;

lua_error:
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	lua_settop(L, 0);
	return EXECUTION_FAILURE;
}

static int dot_call_function(lua_State *L) {
	const char *func = config.optarg;
	if (config.raw_call) {
		if (lua_getglobal(L, func) != LUA_TFUNCTION) {
			_log("%s: Not Lua Function(RAW)", func);
			goto lua_exit;
		}
	} else {
		lua_getglobal(L, prefix);
		lua_getfield(L, -1, "F");
		if (lua_getfield(L, -1, func) != LUA_TFUNCTION) {
			_log("%s: Not Lua Function(BIND)", func);
			goto lua_exit;
		}
	}
	int argc = 0;
	for (WORD_LIST *list = config.list; list; list = list->next) {
		lua_pushstring(L, list->word->word);
		argc++;
	}
	if (lua_pcall(L, argc, 1, 0) != LUA_OK) {
		_log("%s: Function Error", func);
		goto lua_error;
	}

	// return nil means success
	if (!lua_isnil(L, 1)) {
		goto lua_exit;
	}
	lua_settop(L, 0);
	return EXECUTION_SUCCESS;

lua_error:
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
lua_exit:
	lua_settop(L, 0);
	return EXECUTION_FAILURE;
}

static int dot_execute_string(lua_State *L) {
	const char *codestr = config.optarg;
	if (luaL_dostring(L, codestr) != LUA_OK) {
		_log("Code Error");
		goto lua_error;
	}
	lua_settop(L, 0);
	return EXECUTION_SUCCESS;

	// string is usually short, no return nil check

lua_error:
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
	fflush(stdout);
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
		printf("dotlua %s\n", PROJ_VERSION);
		fflush(stdout);
		return EXECUTION_SUCCESS;
	}
	return dot_source_file(DotL);
}

int dotlua_builtin_load(char *s) {
	(void)s;
	void *handle = dlopen(PROJ_LUA_SO_PATH, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		// fprintf(stderr, "error load liblua-5.4.so: %s\n", dlerror());
		builtin_error("error load liblua.so: %s", dlerror());
		return 0;
	}

	DotL = luaL_newstate();
	luaL_openlibs(DotL);

	luaL_newlib(DotL, dotlua); // +1

	lua_pushliteral(DotL, "F");
	lua_newtable(DotL);
	lua_settable(DotL, 1); // -2

	lua_pushliteral(DotL, "version");
	lua_pushliteral(DotL, PROJ_VERSION);
	lua_settable(DotL, 1);

	prefix = getenv("DOTLUA_PREFIX");
	prefix = prefix ? prefix : "sh";
	lua_setglobal(DotL, prefix);
	return 1;
}

void dotlua_builtin_unload(char *s) {
	(void)s;
	// clean functions from sh.F in bash?
	lua_close(DotL);
	dlclose(PROJ_LUA_SO_PATH);
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
