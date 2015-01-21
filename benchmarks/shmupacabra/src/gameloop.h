#ifndef GAME2D_GAMELOOP_H
#define GAME2D_GAMELOOP_H

#include "common.h"

void    run_game_slice(lua_State *L);
void    run_game_loop_counters(lua_State *L);
void    run_game(lua_State *L);

#endif  /* GAME2D_GAMELOOP_H */
