#include "seal.h"
#include "lopen.h"

extern void luaopen_lua_extensions(lua_State *L);

#define TRACE_BACK_FUNC_INDEX 1
#define UPDATE_FUNC_INDEX     2
#define DRAW_FUNC_INDEX       3
#define TOP_FUNC_INDEX        3

struct game* GAME = NULL;

int seal_call(lua_State *L, int n, int r) {
    int err = lua_pcall(L, n, r, TRACE_BACK_FUNC_INDEX);
    switch(err) {
        case LUA_OK:
            break;
        case LUA_ERRRUN:
            fprintf(stderr, "!LUA_ERRRUN : %s\n", lua_tostring(L,-1));
            assert(0);
            break;
        case LUA_ERRMEM:
            fprintf(stderr, "!LUA_ERRMEM : %s\n", lua_tostring(L,-1));
             assert(0);
            break;
        case LUA_ERRERR:
            fprintf(stderr, "!LUA_ERRERR : %s\n", lua_tostring(L,-1));
             assert(0);
            break;
        case LUA_ERRGCMM:
            fprintf(stderr, "!LUA_ERRGCMM : %s\n", lua_tostring(L,-1));
             assert(0);
            break;
        default:
            fprintf(stderr, "!Unknown Lua error: %d\n", err);
            assert(0);
            break;
    }
    return err;
}

lua_State* seal_new_lua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    return L;
}

void seal_init() {
    GAME = (struct game*)malloc(sizeof(struct game));
    
    // lua modules
    lua_State* L = seal_new_lua();
    luaopen_lua_extensions(L);
    GAME->lstate = L;
    
    // load the game settings from config.lua
    seal_load_file("scripts/config.lua");
    
    lua_getglobal(L, "APP_NAME");
    lua_getglobal(L, "WINDOW_WIDTH");
    lua_getglobal(L, "WINDOW_HEIGHT");
    lua_pop(L, 3);

    const char* app_name = lua_tostring(L, 1);
    GAME->window_width = lua_tonumber(L, 2);
    GAME->window_height = lua_tonumber(L, 3);
    
    seal_load_file("scripts/startup.lua");
    seal_start_game();
}

void seal_load_string(const char* script_data) {
    if(luaL_dostring(GAME->lstate, script_data)) {
        fprintf(stderr, "run start script Failed.\n");
        abort();
    }
}

void seal_load_file(const char* script_path) {
    if(luaL_dofile(GAME->lstate, script_path)) {
        fprintf(stderr, "run start script Failed.\n");
        abort();
    }
}

static int traceback (lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (msg)
        luaL_traceback(L, L, msg, 1);
    else if (!lua_isnoneornil(L, 1)) {
        if (!luaL_callmeta(L, 1, "__tostring"))
            lua_pushliteral(L, "(no error message)");
    }
    return 1;
}

void seal_start_game() {
    lua_State *L = GAME->lstate;
    assert(lua_gettop(L) == 0);
    lua_pushcfunction(L, traceback);
    lua_getfield(L,LUA_REGISTRYINDEX, GAME_UPDATE);
    lua_getfield(L,LUA_REGISTRYINDEX, GAME_DRAW);
    lua_getfield(L,LUA_REGISTRYINDEX, GAME_PAUSE);
    lua_getfield(L,LUA_REGISTRYINDEX, GAME_RESUME);
    lua_getfield(L,LUA_REGISTRYINDEX, GAME_EVENT);
}

void seal_update(float dt) {
    
    static int frames = 0;
    ++frames;
    if (frames == 60) {
        frames = 0;
    }
    
    lua_State* L = GAME->lstate;

    lua_pushvalue(L, UPDATE_FUNC_INDEX);
    lua_pushnumber(L, dt);

    seal_call(L, 1, 0);
    lua_settop(L, TOP_FUNC_INDEX);
}

void seal_draw() {
    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
}

void seal_destroy() {
    
    lua_close(GAME->lstate);
}