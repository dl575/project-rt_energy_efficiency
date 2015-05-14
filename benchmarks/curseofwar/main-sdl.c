/******************************************************************************

  Curse of War -- Real Time Strategy Game for Linux.
  Copyright (C) 2013 Alexey Nikolaev.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
******************************************************************************/

#include <stdlib.h>
#include <time.h>
#include "SDL.h"

#ifdef WIN32
#undef main
#endif

#include "grid.h"
#include "common.h"
#include "state.h"
#include "output-sdl.h"
#include "main-common.h"
#include "path.h"

/* no networking in Windows */
#ifndef WIN32
#include <fcntl.h>
#include "messaging.h"
#include "client.h"
#include "server.h"
#endif

/* delay in milliseconds */
#define TIME_DELAY 10

#include "timing.h"

float run_loop_slice(struct state *st, struct ui *ui, SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *uisurf, int tile_variant[MAX_WIDTH][MAX_HEIGHT], int pop_variant[MAX_WIDTH][MAX_HEIGHT], int k)
{
  int loop_counter[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int finished = 0;
  SDL_Event event;
  k++;
  if (k >= 1600)
  {
    loop_counter[0]++;
    k = 0;
  }

  char c = '\0';
  /*
  while ((!finished) && SDL_PollEvent(&event))
  {
    loop_counter[1]++;
    switch (event.type)
    {
      case SDL_KEYDOWN:
        loop_counter[2]++;
        c = '\0';
        switch (event.key.keysym.sym)
      {
        case SDLK_LEFT:
          loop_counter[3]++;
          c = 'h';
          break;

        case SDLK_RIGHT:
          loop_counter[4]++;
          c = 'l';
          break;

        case SDLK_UP:
          loop_counter[5]++;
          c = 'k';
          break;

        case SDLK_DOWN:
          loop_counter[6]++;
          c = 'j';
          break;

        case SDLK_q:
          loop_counter[7]++;
          c = 'q';
          break;

        case SDLK_SPACE:
          loop_counter[8]++;
          c = ' ';
          break;

        default:
          loop_counter[9]++;
          if ((event.key.keysym.unicode & 0xFF80) == 0)
        {
          loop_counter[10]++;
          c = event.key.keysym.unicode & 0x7F;
        }


      }

        finished = singleplayer_process_input(st, ui, c);
        break;

    }

  }
  */

  int slowdown = game_slowdown(st->speed);
  if (((k % slowdown) == 0) && (st->speed != sp_pause))
  {
    loop_counter[11]++;
  }

  if ((k % 5) == 0)
  {
    loop_counter[12]++;
  }

  if (finished)
  {
    loop_counter[13]++;
    {
      goto print_loop_counter;
    }
  }
  else
  {
    {
      goto print_loop_counter;
    }
  }

  {
    print_loop_counter:
#if GET_PREDICT || DEBUG_EN
    ;

    write_array(loop_counter, 14);
    print_loop_counter_end:
#endif
    ;

  }
  {
    predict_exec_time:
    ;
    float exec_time;
#if CORE //big
    #if !CVX_EN //conservative
        exec_time = 1054.000000*loop_counter[0] + -51.000000*loop_counter[1] + 2953.000000*loop_counter[2] + -164.000000*loop_counter[4] + -32.000000*loop_counter[10] + 0.000000;
    #else //cvx
        exec_time = 123.000002*loop_counter[0] + 1467.787853*loop_counter[2] + 382.868691*loop_counter[3] + 122.999998;
    #endif
#else //LITTLE
    #if !CVX_EN //conservative
        exec_time = 802.000000*loop_counter[11] + 34871.000000*loop_counter[12] + 163.000000;
    #else //cvx
        if(CVX_COEFF == 10)
            exec_time = 5577.000000*loop_counter[11] + 29843.000000*loop_counter[12] + 36.000000;
        else if(CVX_COEFF == 50)
            exec_time = 5484.000000*loop_counter[11] + 30296.000000*loop_counter[12] + 56.000000;
        else if(CVX_COEFF == 100)
            exec_time = 5471.000000*loop_counter[11] + 30306.000000*loop_counter[12] + 59.000000;

    #endif
#endif
    return exec_time;
  }
}

int run_loop(struct state *st, struct ui *ui, 
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface,
    SDL_Surface *uisurf, int tile_variant[MAX_WIDTH][MAX_HEIGHT], int
    pop_variant[MAX_WIDTH][MAX_HEIGHT], int k){
  int finished = 0;
  SDL_Event event;
      k++;
      if (k>=1600) k=0;

      char c = '\0';
      while (!finished && SDL_PollEvent(&event)){
        switch (event.type){
          case SDL_KEYDOWN:
            c = '\0';
            switch (event.key.keysym.sym) {
              case SDLK_LEFT: c = 'h'; break;
              case SDLK_RIGHT: c = 'l'; break;
              case SDLK_UP: c = 'k'; break;
              case SDLK_DOWN: c = 'j'; break;
              case SDLK_q: c = 'q'; break;
              case SDLK_SPACE: c = ' '; break;
              default:
                if ( (event.key.keysym.unicode & 0xFF80) == 0 ) {
                  c = event.key.keysym.unicode & 0x7F;
                }
            }
            finished = singleplayer_process_input(st, ui, c);
            break;
        }
      }

      int slowdown = game_slowdown(st->speed);
      if (k % slowdown == 0 && st->speed != sp_pause) { 
        kings_move(st);
        simulate(st);
      }

      if (k % 5 == 0) {
        output_sdl(tileset, typeface, uisurf, screen, st, ui, tile_variant, pop_variant, k);
        SDL_Flip(screen);
      }

  if (finished) {
    return -1;
  } else {
    return k;
  }
}

void run(struct state *st, struct ui *ui, 
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *uisurf){
  /* tile variation */
  int tile_variant[MAX_WIDTH][MAX_HEIGHT];
  int pop_variant[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for (i=0; i<MAX_WIDTH; ++i)
    for (j=0; j<MAX_HEIGHT; ++j) {
      tile_variant[i][j] = rand();
      pop_variant[i][j] = rand();
    }

  int finished = 0;
  SDL_Event event;
  int previous_time = SDL_GetTicks();

  int k = 0;

//---------------------modified by TJSong----------------------//
    int exec_time = 0;
    if(check_define()==ERROR_DEFINE){
        printf("%s", "DEFINE ERROR!!\n");
        //return ERROR_DEFINE;
        return;
    }
    static int cnt = 0;
//---------------------modified by TJSong----------------------//



  //while (!finished){
  while (k >= 0) {

    int time = SDL_GetTicks();

    if (time - previous_time >= TIME_DELAY) {
//---------------------modified by TJSong----------------------//
    fopen_all(); //fopen for frequnecy file
    fprint_deadline(DEADLINE_TIME); //print deadline 
//---------------------modified by TJSong----------------------//


      previous_time = previous_time + TIME_DELAY;


//---------------------modified by TJSong----------------------//
        // Perform slicing and prediction
        float predicted_exec_time = 0.0;
        /*
            CASE 0 = to get prediction equation
            CASE 1 = to get execution deadline
            CASE 2 = to get overhead deadline
            CASE 3 = running on default linux governors
            CASE 4 = running on our prediction
            CASE 5 = running on oracle
            CASE 6 = running on pid
        */
        #if GET_PREDICT /* CASE 0 */
            predicted_exec_time = run_loop_slice(st, ui, screen, tileset, typeface, uisurf, tile_variant, pop_variant, k); //slice        
        #elif GET_DEADLINE /* CASE 1 */
            //nothing
        #elif GET_OVERHEAD /* CASE 2 */
            start_timing();
            predicted_exec_time = run_loop_slice(st, ui, screen, tileset, typeface, uisurf, tile_variant, pop_variant, k); //slice        
            end_timing();
            slice_time = fprint_slice_timing();

            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = fprint_dvfs_timing();
        #elif !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
            //slice_time=0; dvfs_time=0;
            moment_timing_fprint(0); //moment_start
        #elif !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
            moment_timing_fprint(0); //moment_start
            
            start_timing();
            predicted_exec_time = run_loop_slice(st, ui, screen, tileset, typeface, uisurf, tile_variant, pop_variant, k); //slice        
            end_timing();
            end_timing();
            slice_time = fprint_slice_timing();
            
            start_timing();
            #if OVERHEAD_EN //with overhead
                set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #else //without overhead
                set_freq(predicted_exec_time, 0, DEADLINE_TIME, 0); //do dvfs
            #endif
            end_timing();
            dvfs_time = fprint_dvfs_timing();

            moment_timing_fprint(1); //moment_start
        #elif ORACLE_EN /* CASE 5 */
            //slice_time=0;
            static int job_cnt = 0; //job count
            predicted_exec_time  = exec_time_arr[job_cnt];
            moment_timing_fprint(0); //moment_start
            
            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = fprint_dvfs_timing();
            
            moment_timing_fprint(1); //moment_start
            job_cnt++;
        #elif PID_EN /* CASE 6 */
            moment_timing_fprint(0); //moment_start
            
            start_timing();
            predicted_exec_time = pid_controller(exec_time); //pid == slice
            end_timing();
            slice_time = fprint_slice_timing();
            
            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = fprint_dvfs_timing();
            
            moment_timing_fprint(1); //moment_start
        #endif
    
//---------------------modified by TJSong----------------------//



      start_timing();

      k = run_loop(st, ui, screen, tileset, typeface, uisurf, tile_variant, pop_variant, k);

      end_timing();
//---------------------modified by TJSong----------------------//
        exec_time = exec_timing();
        int delay_time = 0;

        #if GET_PREDICT /* CASE 0 */
            fprint_exec_time(exec_time);
        #elif GET_DEADLINE /* CASE 1 */
            fprint_exec_time(exec_time);
        #elif GET_OVERHEAD /* CASE 2 */
            //nothing
        #else /* CASE 3,4,5 and 6 */
            if(DELAY_EN && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0)){
                start_timing();
                usleep(delay_time);
                end_timing();
                delay_time = exec_timing();
            }else
                delay_time = 0;
        moment_timing_fprint(2); //moment_end
        fprint_exec_time(exec_time);
        fprint_total_time(exec_time + slice_time + dvfs_time + delay_time);
        #endif
        fclose_all();//TJSong

        // Write out predicted time & print out frequency used
        fprint_predicted_time(predicted_exec_time);
        fprint_freq(); 

    if(cnt++ > 1000)//TJSong
        break;
//---------------------modified by TJSong----------------------//
 
    }
    else{
      SDL_Delay(TIME_DELAY/2);
    }
  }
}

#ifndef WIN32
/* Client version */
void run_client(struct state *st, struct ui *ui, 
    SDL_Surface *screen, SDL_Surface *tileset, SDL_Surface *typeface, SDL_Surface *uisurf,
    char *s_server_addr, char *s_server_port, char *s_client_port){
  /* tile variation */
  int tile_variant[MAX_WIDTH][MAX_HEIGHT];
  int pop_variant[MAX_WIDTH][MAX_HEIGHT];
  int i, j;
  for (i=0; i<MAX_WIDTH; ++i)
    for (j=0; j<MAX_HEIGHT; ++j) {
      tile_variant[i][j] = rand();
      pop_variant[i][j] = rand();
    }

  /* Init Networking */
  int sfd; /* file descriptor of the socket */
  int ret_code; /* code returned by a function */
  struct addrinfo srv_addr;
  if ((ret_code = client_init_session 
        (&sfd, s_client_port, &srv_addr, s_server_addr, s_server_port)) != 0) {
    perror("Failed to initialize networking");
    return;
  }
  fcntl(sfd, F_SETFL, O_NONBLOCK);
  /* */

  /* Starting the main loop */
  int finished = 0;
  SDL_Event event;
  int previous_time = SDL_GetTicks();

  int k = 0;
  int initialized = 0;
  st->time = 0;
  while (!finished){

    int time = SDL_GetTicks();

    if (time - previous_time >= TIME_DELAY) {

      previous_time = previous_time + TIME_DELAY;
      k++;
      if (k>=1600) k=0;

      char c = '\0';
      while (!finished && SDL_PollEvent(&event)){
        switch (event.type){
          case SDL_KEYDOWN:
            c = '\0';
            switch (event.key.keysym.sym) {
              case SDLK_LEFT: c = 'h'; break;
              case SDLK_RIGHT: c = 'l'; break;
              case SDLK_UP: c = 'k'; break;
              case SDLK_DOWN: c = 'j'; break;
              case SDLK_q: c = 'q'; break;
              case SDLK_SPACE: c = ' '; break;
              default:
                if ( (event.key.keysym.unicode & 0xFF80) == 0 ) {
                  c = event.key.keysym.unicode & 0x7F;
                }
            }
            /* finished = singleplayer_process_input(st, ui, c); */
            finished = client_process_input (st, ui, c, sfd, &srv_addr);
            /* */
            break;
        }
      }
      
      /* Send Keep Alive & receive packets */
      if (k%50 == 0) 
        send_msg_c (sfd, &srv_addr, MSG_C_IS_ALIVE, 0, 0, 0);
      
      int msg = client_receive_msg_s (sfd, st);
      if (msg == MSG_S_STATE && initialized != 1) {
        initialized = 1;
        ui_init(st, ui);
      
        /* reset screen */
        int screen_width = (ui->xlength + 2) * TILE_WIDTH;
        int screen_height = (st->grid.height + 3) * TILE_HEIGHT + TYPE_HEIGHT*5;
        screen_width = MAX (screen_width, TILE_WIDTH + 75*TYPE_WIDTH + TILE_WIDTH);
        SDL_FreeSurface(screen);
        screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_DOUBLEBUF);
        if (screen == NULL) {
          fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
          exit(1);
        }
        /* */
      }
      /* */
      
      /* no simulation in the client */
      /*
      int slowdown = game_slowdown(st->speed);
      if (k % slowdown == 0 && st->speed != sp_pause) { 
        kings_move(st);
        simulate(st);
      }
      */

      if (initialized) {
        if (k % 5 == 0) {
          output_sdl(tileset, typeface, uisurf, screen, st, ui, tile_variant, pop_variant, k);
          SDL_Flip(screen);
        }
      }

    }
    else{
      SDL_Delay(TIME_DELAY/2);
    }
  }
}
#endif

int main(int argc, char *argv[]) {

  //srand(time(NULL));
  srand(0);

  /* Read command line arguments */
  struct basic_options op;
  struct multi_options mop;
  if (get_options(argc, argv, &op, &mop) == 1) return 1;

  /* Initialize the state */
  struct state st;
  struct ui ui;

  state_init(&st, &op, &mop);
  ui_init(&st, &ui);

  int screen_width = (ui.xlength + 2) * TILE_WIDTH;
  int screen_height = (st.grid.height + 3) * TILE_HEIGHT + TYPE_HEIGHT*5;
  screen_width = MAX (screen_width, TILE_WIDTH + 75*TYPE_WIDTH + TILE_WIDTH);

  /* Init SDL */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_Surface *screen;
  screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(300, 30);

  /* Load Data */
  char **path;
  path = get_search_paths();
  
  /* Images */
  char *filename;
  
  SDL_Surface *tileset;
  filename = find_file (path, "images/tileset.bmp");
  if ( load_image(filename, &tileset) != 0 ) return 1;
  free(filename); 
  
  SDL_Surface *typeface;
  filename = find_file (path, "images/type.bmp");
  if ( load_image(filename, &typeface) != 0 ) return 1;
  free(filename);
  
  SDL_Surface *uisurf;
  filename = find_file (path, "images/ui.bmp");
  if ( load_image(filename, &uisurf) != 0 ) return 1;
  free(filename);

#ifdef WIN32 
  /* Run the game */
  run(&st, &ui, screen, tileset, typeface, uisurf);
#else
  if (!mop.multiplayer_flag) {
    /* Run the game */
    run(&st, &ui, screen, tileset, typeface, uisurf);
  }
  else {
    if (mop.server_flag)  {
      fprintf(stderr, "\nPlease, run the ncurses executable to start a server.\n\n");
      exit(1);
    }
    else run_client(&st, &ui, screen, tileset, typeface, uisurf, mop.val_server_addr, mop.val_server_port, mop.val_client_port);
  }
#endif

  /* Finalize */
  SDL_FreeSurface(tileset);
  SDL_FreeSurface(typeface);
  SDL_FreeSurface(uisurf);
  
  if (!mop.multiplayer_flag || mop.server_flag)
    printf ("Random seed was %i\n", st.map_seed);

  destroy_search_paths(path);
  free(mop.val_client_port);
  free(mop.val_server_addr);
  free(mop.val_server_port);
  return 0;
}
