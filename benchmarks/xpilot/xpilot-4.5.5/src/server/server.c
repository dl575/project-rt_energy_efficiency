/* $Id: server.c,v 5.21 2002/02/13 17:09:18 dik Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <unistd.h>
# ifndef __hpux
#  include <sys/time.h>
# endif
# include <pwd.h>
# include <sys/param.h>
#endif

#ifdef PLOCKSERVER
# if defined(__linux__)
#  include <sys/mman.h>
# else
#  include <sys/lock.h>
# endif
#endif

#ifdef _WINDOWS
# include <io.h>
# include "NT/winServer.h"
# include "NT/winSvrThread.h"
#endif

#define	SERVER
#include "version.h"
#include "config.h"
#include "types.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "socklib.h"
#include "map.h"
#include "bit.h"
#include "sched.h"
#include "netserver.h"
#include "error.h"
#include "portability.h"
#include "server.h"
#include "commonproto.h"

#include "timing.h"
#include "my_common.h"

char server_version[] = VERSION;

#ifndef	lint
char xpilots_versionid[] = "@(#)$" TITLE " $";
#endif

/*
 * Global variables
 */

//---------------------modified by TJSong----------------------//
double exec_time = 0;
int main_job_cnt = 0;
#if HETERO_EN
  int current_core = CORE; //0: little, 1: big
  int is_stable_big = 0; //0: not stable
  int is_stable_little = 0; //0: not stable
  llsp_t *solver_big;
  llsp_t *solver_little;
#elif !HETERO_EN
  llsp_t *solver;
#endif
//---------------------modified by TJSong----------------------//

int			NumPlayers = 0;
int			NumAlliances = 0;
player			**Players;
int			GetInd_1;
int			GetInd[NUM_IDS+1];
server_t		Server;
char			*serverAddr;
int			ShutdownServer = -1;
int			ShutdownDelay = 1000;
char			ShutdownReason[MAX_CHARS];
int 			framesPerSecond = 18;
long			main_loops = 0;		/* needed in events.c */

#ifdef LOG
static bool		Log = true;
#endif
static bool		NoPlayersEnteredYet = true;
int			game_lock = false;
time_t			gameOverTime = 0;
time_t			serverTime = 0;

extern int		login_in_progress;
extern int		NumQueuedPlayers;

static void Check_server_versions(void);
extern void Main_loop(void);
static void Handle_signal(int sig_no);

int main(int argc, char **argv)
{
    int			timer_tick_rate;
    char		*addr;

    /*
     * Make output always linebuffered.  By default pipes
     * and remote shells cause stdout to be fully buffered.
     */
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

    /*
     * --- Output copyright notice ---
     */

    xpprintf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");

    init_error(argv[0]);
    Check_server_versions();

    //seedMT((unsigned)time((time_t *)0) * Get_process_id());
    seedMT(0xdeadbeef);

    if (Parser(argc, argv) == FALSE) {
	exit(1);
    }

    plock_server(pLockServer);           /* Lock the server into memory */
    Make_table();			/* Make trigonometric tables */
    Compute_gravity();
    Find_base_direction();
    Walls_init();

    /* Allocate memory for players, shots and messages */
    Alloc_players(World.NumBases + MAX_PSEUDO_PLAYERS);
    Alloc_shots(MAX_TOTAL_SHOTS);
    Alloc_cells();

    Move_init();

    Robot_init();

    Treasure_init();

    /*
     * Get server's official name.
     */
    if (serverHost) {
	addr = sock_get_addr_by_name(serverHost);
	if (addr == NULL) {
	    errno = 0;
	    error("Failed name lookup on: %s", serverHost);
	    return 1;
	}
	serverAddr = xp_strdup(addr);
	strlcpy(Server.host, serverHost, sizeof(Server.host));
    } else {
	sock_get_local_hostname(Server.host, sizeof Server.host,
				(reportToMetaServer != 0 &&
				 searchDomainForXPilot != 0));
    }

    Get_login_name(Server.owner, sizeof Server.owner);

    /*
     * Log, if enabled.
     */
    Log_game("START");

    if (!Contact_init())
		return(FALSE);

    Meta_init();

    if (Setup_net_server() == -1) {
	End_game();
    }
#ifndef _WINDOWS
    if (NoQuit) {
	signal(SIGHUP, SIG_IGN);
    } else {
	signal(SIGHUP, Handle_signal);
    }
    signal(SIGTERM, Handle_signal);
    signal(SIGINT, Handle_signal);
    signal(SIGPIPE, SIG_IGN);
#ifdef IGNORE_FPE
    signal(SIGFPE, SIG_IGN);
#endif
#endif	/* _WINDOWS */
    /*
     * Set the time the server started
     */
    serverTime = time(NULL);

#ifndef SILENT
    xpprintf("%s Server runs at %d frames per second\n", showtime(), framesPerSecond);
#endif

    if (timerResolution > 0) {
	timer_tick_rate = timerResolution;
    }
    else {
	timer_tick_rate = FPS;
    }
#ifdef _WINDOWS
    /* Windows returns here, we let the worker thread call sched() */
    install_timer_tick(ServerThreadTimerProc, timer_tick_rate);
#else
//---------------------modified by TJSong----------------------//
    if(check_define()==ERROR_DEFINE){
        printf("%s", "DEFINE ERROR!!\n");
        return ERROR_DEFINE;
    }
    fopen_all(); //fopen for frequnecy file
#if HETERO_EN
    solver_big = llsp_new(N_FEATURE + 1);
    solver_little = llsp_new(N_FEATURE + 1);
#elif !HETERO_EN
    solver = llsp_new(N_FEATURE + 1);
#endif
    install_timer_tick(Main_loop, timer_tick_rate);
//---------------------modified by TJSong----------------------//
    sched();
    xpprintf("sched returned!?");
    End_game();
#endif
    llsp_dispose(solver);

    return 1;
}

/*
 * Global variables from frame.c that are needed for inlined functions.
 */
typedef unsigned short shuffle_t;

/*
 * Structure for calculating if a pixel is visible by a player.
 * The following always holds:
 *	(world.x >= realWorld.x && world.y >= realWorld.y)
 */
typedef struct {
    position	world;			/* Lower left hand corner is this */
					/* world coordinate */
    position	realWorld;		/* If the player is on the edge of
					   the screen, these are the world
					   coordinates before adjustment... */
} pixel_visibility_t;

/*
 * Structure with player position info measured in blocks instead of pixels.
 * Used for map state info updating.
 */
typedef struct {
    ipos		world;
    ipos		realWorld;
} block_visibility_t;

typedef struct {
    unsigned char	x, y;
} debris_t;

typedef struct {
    short		x, y, size;
} radar_t;


extern time_t		gameOverTime;
//long			frame_loops = 1;
static long		last_frame_shuffle;
static shuffle_t	*object_shuffle_ptr;
static int		num_object_shuffle;
static int		max_object_shuffle;
static shuffle_t	*player_shuffle_ptr;
static int		num_player_shuffle;
static int		max_player_shuffle;
static radar_t		*radar_ptr;
static int		num_radar, max_radar;

static pixel_visibility_t pv;
static int		view_width,
			view_height,
			horizontal_blocks,
			vertical_blocks,
			debris_x_areas,
			debris_y_areas,
			debris_areas,
			debris_colors,
			spark_rand;
static debris_t		*debris_ptr[DEBRIS_TYPES];
static unsigned		debris_num[DEBRIS_TYPES],
			debris_max[DEBRIS_TYPES];
static debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
static unsigned		fastshot_num[DEBRIS_TYPES * 2],
			fastshot_max[DEBRIS_TYPES * 2];

/*
 * Macro to make room in a given dynamic array for new elements.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * T is the type of the elements.
 * E is the number of new elements to store in the array.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define EXPAND(P,N,M,T,E)						\
    if ((N) + (E) > (M)) {						\
	if ((M) <= 0) {							\
	    M = (E) + 2;						\
	    P = (T *) malloc((M) * sizeof(T));				\
	    N = 0;							\
	} else {							\
	    M = ((M) << 1) + (E);					\
	    P = (T *) realloc(P, (M) * sizeof(T));			\
	}								\
	if (P == NULL) {						\
	    error("No memory");						\
	    N = M = 0;							\
	    return;	/* ! */						\
	}								\
    }

#define inview(x_, y_) 								\
    (   (   ((x_) > pv.world.x && (x_) < pv.world.x + view_width)		\
	 || ((x_) > pv.realWorld.x && (x_) < pv.realWorld.x + view_width))	\
     && (   ((y_) > pv.world.y && (y_) < pv.world.y + view_height)		\
	 || ((y_) > pv.realWorld.y && (y_) < pv.realWorld.y + view_height)))

static int block_inview(block_visibility_t *bv, int x, int y)
{
    return ((x > bv->world.x && x < bv->world.x + horizontal_blocks)
	    || (x > bv->realWorld.x && x < bv->realWorld.x + horizontal_blocks))
	&& ((y > bv->world.y && y < bv->world.y + vertical_blocks)
	    || (y > bv->realWorld.y && y < bv->realWorld.y + vertical_blocks));
}

#include "saudio.h"

/*
 * Slice of Main_loop.
 */
#if !HETERO_EN
struct slice_return Main_loop_slice(llsp_t *restrict solver)
#elif HETERO_EN
struct slice_return Main_loop_slice(llsp_t *restrict solver_big, llsp_t *restrict solver_little)
#endif
{
  int loop_counter[250] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
{}
{}
{}
  main_loops++;
  if ((main_loops & 0x3F) == 0)
  {
    loop_counter[0]++;
{}
  }

  if (ShutdownServer >= 0)
  {
    loop_counter[1]++;
    if (ShutdownServer == 0)
    {
      loop_counter[2]++;
      {
        int return_value;
        player *pl_rename0;
{}
        if (ShutdownServer == 0)
        {
          loop_counter[3]++;
{}
{}
{}
        }
        else
        {
{}
        }

        while (NumPlayers > 0)
        {
          loop_counter[4]++;
          pl_rename0 = Players[NumPlayers - 1];
          if (pl_rename0->conn == NOT_CONNECTED)
          {
            loop_counter[5]++;
{}
          }
          else
          {
{}
          }

        }

{}
{}
{}
{}
{}
{}
{}
{}
{}
        {
          return_value = FALSE;
          goto return0;
        }
        return0:
        ;

      }
    }
    else
    {
      ShutdownServer--;
    }

  }

{}
  if ((NumPlayers > (NumRobots + NumPseudoPlayers)) || RawMode)
  {
    loop_counter[6]++;
    if (NoPlayersEnteredYet)
    {
      loop_counter[7]++;
      if (NumPlayers > (NumRobots + NumPseudoPlayers))
      {
        loop_counter[8]++;
        NoPlayersEnteredYet = false;
        if (gameDuration > 0.0)
        {
          loop_counter[9]++;
{}
          gameOverTime = ((time_t) (gameDuration * 60)) + time((time_t *) NULL);
        }

      }

    }

{}
    if ((main_loops % UPDATES_PR_FRAME) == 0)
    {
      loop_counter[10]++;
      {
        int i_rename1;
        int conn_rename1;
        int ind_rename1;
        player *pl_rename1;
        time_t newTimeLeft_rename1 = 0;
        static time_t oldTimeLeft_rename1;
        static bool game_over_called_rename1 = false;
        if ((++frame_loops) >= LONG_MAX)
        {
          loop_counter[11]++;
          frame_loops = 1;
        }

        {
          if (last_frame_shuffle != frame_loops)
          {
            loop_counter[12]++;
            last_frame_shuffle = frame_loops;
            {
              int i_rename15;
              size_t memsize_rename15;
              num_object_shuffle = MIN(NumObjs, maxVisibleObject);
              if (max_object_shuffle < num_object_shuffle)
              {
                loop_counter[13]++;
                if (object_shuffle_ptr != NULL)
                {
                  loop_counter[14]++;
{}
                }

                max_object_shuffle = num_object_shuffle;
                memsize_rename15 = max_object_shuffle * (sizeof(shuffle_t));
                object_shuffle_ptr = (shuffle_t *) malloc(memsize_rename15);
                if (object_shuffle_ptr == NULL)
                {
                  loop_counter[15]++;
                  max_object_shuffle = 0;
                }

              }

              if (max_object_shuffle < num_object_shuffle)
              {
                loop_counter[16]++;
                num_object_shuffle = max_object_shuffle;
              }

              for (i_rename15 = 0; i_rename15 < num_object_shuffle; i_rename15++)
              {
                loop_counter[17]++;
                object_shuffle_ptr[i_rename15] = i_rename15;
              }

              for (i_rename15 = num_object_shuffle - 1; i_rename15 >= 0; --i_rename15)
              {
                loop_counter[18]++;
                if (object_shuffle_ptr[i_rename15] == i_rename15)
                {
                  loop_counter[19]++;
                  int j_rename15 = (int) (rfrac() * i_rename15);
                  shuffle_t tmp_rename15 = object_shuffle_ptr[j_rename15];
                  object_shuffle_ptr[j_rename15] = object_shuffle_ptr[i_rename15];
                  object_shuffle_ptr[i_rename15] = tmp_rename15;
                }

              }

              return15:
              ;

            }
            {
              int i_rename16;
              size_t memsize_rename16;
              num_player_shuffle = MIN(NumPlayers, 65535);
              if (max_player_shuffle < num_player_shuffle)
              {
                loop_counter[20]++;
                if (player_shuffle_ptr != NULL)
                {
                  loop_counter[21]++;
{}
                }

                max_player_shuffle = num_player_shuffle;
                memsize_rename16 = max_player_shuffle * (sizeof(shuffle_t));
                player_shuffle_ptr = (shuffle_t *) malloc(memsize_rename16);
                if (player_shuffle_ptr == NULL)
                {
                  loop_counter[22]++;
                  max_player_shuffle = 0;
                }

              }

              if (max_player_shuffle < num_player_shuffle)
              {
                loop_counter[23]++;
                num_player_shuffle = max_player_shuffle;
              }

              for (i_rename16 = 0; i_rename16 < num_player_shuffle; i_rename16++)
              {
                loop_counter[24]++;
                player_shuffle_ptr[i_rename16] = i_rename16;
              }

              for (i_rename16 = 0; i_rename16 < num_player_shuffle; i_rename16++)
              {
                loop_counter[25]++;
                int j_rename16 = (int) (rfrac() * num_player_shuffle);
                shuffle_t tmp_rename16 = player_shuffle_ptr[j_rename16];
                player_shuffle_ptr[j_rename16] = player_shuffle_ptr[i_rename16];
                player_shuffle_ptr[i_rename16] = tmp_rename16;
              }

              return16:
              ;

            }
          }

          return4:
          ;

        }
        if (((gameDuration > 0.0) && (game_over_called_rename1 == false)) && (oldTimeLeft_rename1 != (newTimeLeft_rename1 = gameOverTime - time(NULL))))
        {
          loop_counter[26]++;
          if (newTimeLeft_rename1 <= 0)
          {
            loop_counter[27]++;
{}
            ShutdownServer = 30 * FPS;
            game_over_called_rename1 = true;
          }

        }

        for (i_rename1 = 0; i_rename1 < num_player_shuffle; i_rename1++)
        {
          loop_counter[28]++;
          pl_rename1 = Players[i_rename1];
          conn_rename1 = pl_rename1->conn;
          if (conn_rename1 == NOT_CONNECTED)
          {
            loop_counter[29]++;
            continue;
          }

          if ((BIT(pl_rename1->status, PAUSE | GAME_OVER) && (!allowViewing)) && (!pl_rename1->isowner))
          {
            loop_counter[30]++;
            if (BIT(pl_rename1->status, PAUSE))
            {
              loop_counter[31]++;
              if (frame_loops & 0x03)
              {
                loop_counter[32]++;
                continue;
              }

            }
            else
            {
              if (frame_loops & 0x01)
              {
                loop_counter[33]++;
                continue;
              }

            }

          }

          if (pl_rename1->player_count > 0)
          {
            loop_counter[34]++;
            pl_rename1->player_round++;
            if (pl_rename1->player_round >= pl_rename1->player_count)
            {
              loop_counter[35]++;
              pl_rename1->player_round = 0;
              continue;
            }

          }

          if (Send_start_of_frame(conn_rename1) == (-1))
          {
            loop_counter[36]++;
            continue;
          }

          if (newTimeLeft_rename1 != oldTimeLeft_rename1)
          {
            loop_counter[37]++;
{}
          }
          else
            if ((maxRoundTime > 0) && (roundtime >= 0))
          {
            loop_counter[38]++;
{}
          }


          if (BIT(pl_rename1->lock.tagged, LOCK_PLAYER))
          {
            loop_counter[39]++;
            if ((BIT(pl_rename1->status, GAME_OVER | PLAYING) == (GAME_OVER | PLAYING)) || (BIT(pl_rename1->status, PAUSE) && ((((BIT(World.rules->mode, TEAM_PLAY) && (pl_rename1->team != TEAM_NOT_SET)) && (pl_rename1->team == Players[GetInd[pl_rename1->lock.pl_id]]->team)) || pl_rename1->isowner) || allowViewing)))
            {
              loop_counter[40]++;
              ind_rename1 = GetInd[pl_rename1->lock.pl_id];
            }
            else
            {
              ind_rename1 = i_rename1;
            }

          }
          else
          {
            ind_rename1 = i_rename1;
          }

          if (Players[ind_rename1]->damaged > 0)
          {
            loop_counter[41]++;
{}
          }
          else
          {
            {
              int ind_rename5 = ind_rename1;
              int conn_rename5 = conn_rename1;
              player *pl_rename5 = Players[ind_rename5];
              Get_display_parameters(conn_rename5, &view_width, &view_height, &debris_colors, &spark_rand);
              debris_x_areas = (view_width + 255) >> 8;
              debris_y_areas = (view_height + 255) >> 8;
              debris_areas = debris_x_areas * debris_y_areas;
              horizontal_blocks = (view_width + (BLOCK_SZ - 1)) / BLOCK_SZ;
              vertical_blocks = (view_height + (BLOCK_SZ - 1)) / BLOCK_SZ;
              pv.world.x = pl_rename5->pos.x - (view_width / 2);
              pv.world.y = pl_rename5->pos.y - (view_height / 2);
              pv.realWorld = pv.world;
              if (BIT(World.rules->mode, WRAP_PLAY))
              {
                loop_counter[42]++;
                if ((pv.world.x < 0) && ((pv.world.x + view_width) < World.width))
                {
                  loop_counter[43]++;
                  pv.world.x += World.width;
                }
                else
                  if ((pv.world.x > 0) && ((pv.world.x + view_width) >= World.width))
                {
                  loop_counter[44]++;
                  pv.realWorld.x -= World.width;
                }


                if ((pv.world.y < 0) && ((pv.world.y + view_height) < World.height))
                {
                  loop_counter[45]++;
                  pv.world.y += World.height;
                }
                else
                  if ((pv.world.y > 0) && ((pv.world.y + view_height) >= World.height))
                {
                  loop_counter[46]++;
                  pv.realWorld.y -= World.height;
                }


              }

              return5:
              ;

            }
            int Frame_status_result_rename1;
            {
              int return_value;
              int ind_rename6 = ind_rename1;
              int conn_rename6 = conn_rename1;
              static char mods_rename6[MAX_CHARS];
              player *pl_rename6 = Players[ind_rename6];
              int n_rename6;
              int lock_ind_rename6;
              int lock_id_rename6 = NO_ID;
              int lock_dist_rename6 = 0;
              int lock_dir_rename6 = 0;
              int i_rename6;
              int showautopilot_rename6;
{}
              if (BIT(pl_rename6->lock.tagged, LOCK_PLAYER) && BIT(pl_rename6->used, HAS_COMPASS))
              {
                loop_counter[47]++;
                lock_id_rename6 = pl_rename6->lock.pl_id;
                lock_ind_rename6 = GetInd[lock_id_rename6];
                if ((((((!BIT(World.rules->mode, LIMITED_VISIBILITY)) || (pl_rename6->lock.distance <= pl_rename6->sensor_range)) && (((pl_rename6->visibility[lock_ind_rename6].canSee || OWNS_TANK(ind_rename6, lock_ind_rename6)) || TEAM(ind_rename6, lock_ind_rename6)) || ALLIANCE(ind_rename6, lock_ind_rename6))) && (BIT(Players[lock_ind_rename6]->status, PLAYING | GAME_OVER) == PLAYING)) && (playersOnRadar || ((((Players[lock_ind_rename6]->pos.x > pv.world.x) && (Players[lock_ind_rename6]->pos.x < (pv.world.x + view_width))) || ((Players[lock_ind_rename6]->pos.x > pv.realWorld.x) && (Players[lock_ind_rename6]->pos.x < (pv.realWorld.x + view_width)))) && (((Players[lock_ind_rename6]->pos.y > pv.world.y) && (Players[lock_ind_rename6]->pos.y < (pv.world.y + view_height))) || ((Players[lock_ind_rename6]->pos.y > pv.realWorld.y) && (Players[lock_ind_rename6]->pos.y < (pv.realWorld.y + view_height))))))) && (pl_rename6->lock.distance != 0))
                {
                  loop_counter[48]++;
{}
                  lock_dir_rename6 = (int) Wrap_findDir((int) (Players[lock_ind_rename6]->pos.x - pl_rename6->pos.x), (int) (Players[lock_ind_rename6]->pos.y - pl_rename6->pos.y));
                  lock_dist_rename6 = (int) pl_rename6->lock.distance;
                }

              }

              if (BIT(pl_rename6->status, HOVERPAUSE))
              {
                loop_counter[49]++;
                showautopilot_rename6 = (pl_rename6->count <= 0) || ((frame_loops % 8) < 4);
              }
              else
                if (BIT(pl_rename6->used, HAS_AUTOPILOT))
              {
                loop_counter[50]++;
                showautopilot_rename6 = (frame_loops % 8) < 4;
              }
              else
                showautopilot_rename6 = 0;


              i_rename6 = 0;
              if (BIT(pl_rename6->mods.nuclear, FULLNUCLEAR))
              {
                loop_counter[51]++;
                mods_rename6[i_rename6++] = 'F';
              }

              if (BIT(pl_rename6->mods.nuclear, NUCLEAR))
              {
                loop_counter[52]++;
                mods_rename6[i_rename6++] = 'N';
              }

              if (BIT(pl_rename6->mods.warhead, CLUSTER))
              {
                loop_counter[53]++;
                mods_rename6[i_rename6++] = 'C';
              }

              if (BIT(pl_rename6->mods.warhead, IMPLOSION))
              {
                loop_counter[54]++;
                mods_rename6[i_rename6++] = 'I';
              }

              if (pl_rename6->mods.velocity)
              {
                loop_counter[55]++;
                if (i_rename6)
                {
                  loop_counter[56]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'V';
                {
                  int return_value;
                  int i_rename17 = i_rename6;
                  int num_rename17 = pl_rename6->mods.velocity;
                  int digits_rename17;
                  int t_rename17;
                  if (num_rename17 < 0)
                  {
                    loop_counter[57]++;
                    mods_rename6[i_rename17++] = '-';
                    num_rename17 = -num_rename17;
                  }

                  if (num_rename17 < 10)
                  {
                    loop_counter[58]++;
                    mods_rename6[i_rename17++] = '0' + num_rename17;
                    {
                      return_value = i_rename17;
                      goto return17;
                    }
                  }

                  for (t_rename17 = num_rename17, digits_rename17 = 0; t_rename17; t_rename17 /= 10, digits_rename17++)
                    loop_counter[59]++;

                  for (t_rename17 = (i_rename17 + digits_rename17) - 1; t_rename17 >= 0; t_rename17--)
                  {
                    loop_counter[60]++;
                    mods_rename6[t_rename17] = num_rename17 % 10;
                    num_rename17 /= 10;
                  }

                  {
                    return_value = i_rename17 + digits_rename17;
                    goto return17;
                  }
                  return17:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.mini)
              {
                loop_counter[61]++;
                if (i_rename6)
                {
                  loop_counter[62]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'X';
                {
                  int return_value;
                  int i_rename18 = i_rename6;
                  int num_rename18 = pl_rename6->mods.mini + 1;
                  int digits_rename18;
                  int t_rename18;
                  if (num_rename18 < 0)
                  {
                    loop_counter[63]++;
                    mods_rename6[i_rename18++] = '-';
                    num_rename18 = -num_rename18;
                  }

                  if (num_rename18 < 10)
                  {
                    loop_counter[64]++;
                    mods_rename6[i_rename18++] = '0' + num_rename18;
                    {
                      return_value = i_rename18;
                      goto return18;
                    }
                  }

                  for (t_rename18 = num_rename18, digits_rename18 = 0; t_rename18; t_rename18 /= 10, digits_rename18++)
                    loop_counter[65]++;

                  for (t_rename18 = (i_rename18 + digits_rename18) - 1; t_rename18 >= 0; t_rename18--)
                  {
                    loop_counter[66]++;
                    mods_rename6[t_rename18] = num_rename18 % 10;
                    num_rename18 /= 10;
                  }

                  {
                    return_value = i_rename18 + digits_rename18;
                    goto return18;
                  }
                  return18:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.spread)
              {
                loop_counter[67]++;
                if (i_rename6)
                {
                  loop_counter[68]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'Z';
                {
                  int return_value;
                  int i_rename19 = i_rename6;
                  int num_rename19 = pl_rename6->mods.spread;
                  int digits_rename19;
                  int t_rename19;
                  if (num_rename19 < 0)
                  {
                    loop_counter[69]++;
                    mods_rename6[i_rename19++] = '-';
                    num_rename19 = -num_rename19;
                  }

                  if (num_rename19 < 10)
                  {
                    loop_counter[70]++;
                    mods_rename6[i_rename19++] = '0' + num_rename19;
                    {
                      return_value = i_rename19;
                      goto return19;
                    }
                  }

                  for (t_rename19 = num_rename19, digits_rename19 = 0; t_rename19; t_rename19 /= 10, digits_rename19++)
                    loop_counter[71]++;

                  for (t_rename19 = (i_rename19 + digits_rename19) - 1; t_rename19 >= 0; t_rename19--)
                  {
                    loop_counter[72]++;
                    mods_rename6[t_rename19] = num_rename19 % 10;
                    num_rename19 /= 10;
                  }

                  {
                    return_value = i_rename19 + digits_rename19;
                    goto return19;
                  }
                  return19:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.power)
              {
                loop_counter[73]++;
                if (i_rename6)
                {
                  loop_counter[74]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'B';
                {
                  int return_value;
                  int i_rename20 = i_rename6;
                  int num_rename20 = pl_rename6->mods.power;
                  int digits_rename20;
                  int t_rename20;
                  if (num_rename20 < 0)
                  {
                    loop_counter[75]++;
                    mods_rename6[i_rename20++] = '-';
                    num_rename20 = -num_rename20;
                  }

                  if (num_rename20 < 10)
                  {
                    loop_counter[76]++;
                    mods_rename6[i_rename20++] = '0' + num_rename20;
                    {
                      return_value = i_rename20;
                      goto return20;
                    }
                  }

                  for (t_rename20 = num_rename20, digits_rename20 = 0; t_rename20; t_rename20 /= 10, digits_rename20++)
                    loop_counter[77]++;

                  for (t_rename20 = (i_rename20 + digits_rename20) - 1; t_rename20 >= 0; t_rename20--)
                  {
                    loop_counter[78]++;
                    mods_rename6[t_rename20] = num_rename20 % 10;
                    num_rename20 /= 10;
                  }

                  {
                    return_value = i_rename20 + digits_rename20;
                    goto return20;
                  }
                  return20:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.laser)
              {
                loop_counter[79]++;
                if (i_rename6)
                {
                  loop_counter[80]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'L';
                mods_rename6[i_rename6++] = BIT(pl_rename6->mods.laser, STUN) ? 'S' : 'B';
              }

              mods_rename6[i_rename6] = '\0';
              n_rename6 = Send_self(conn_rename6, pl_rename6, lock_id_rename6, lock_dist_rename6, lock_dir_rename6, showautopilot_rename6, Players[GetInd[Get_player_id(conn_rename6)]]->status, mods_rename6);
              if (n_rename6 <= 0)
              {
                loop_counter[81]++;
                {
                  return_value = 0;
                  goto return6;
                }
              }

              if (BIT(pl_rename6->used, HAS_EMERGENCY_THRUST))
              {
                loop_counter[82]++;
{}
              }

              if (BIT(pl_rename6->used, HAS_EMERGENCY_SHIELD))
              {
                loop_counter[83]++;
{}
              }

              if (BIT(pl_rename6->status, SELF_DESTRUCT) && (pl_rename6->count > 0))
              {
                loop_counter[84]++;
{}
              }

              if (BIT(pl_rename6->used, HAS_PHASING_DEVICE))
              {
                loop_counter[85]++;
{}
              }

              if (ShutdownServer != (-1))
              {
                loop_counter[86]++;
{}
              }

              if (round_delay_send > 0)
              {
                loop_counter[87]++;
{}
              }

              {
                return_value = 1;
                goto return6;
              }
              return6:
              ;

              Frame_status_result_rename1 = return_value;
            }
            if (Frame_status_result_rename1 <= 0)
            {
              loop_counter[88]++;
              continue;
            }

            {
              int ind_rename7 = ind_rename1;
              int conn_rename7 = conn_rename1;
              player *pl_rename7 = Players[ind_rename7];
              int i_rename7;
              int k_rename7;
              int x_rename7;
              int y_rename7;
              int conn_bit_rename7 = 1 << conn_rename7;
              block_visibility_t bv_rename7;
              const int fuel_packet_size_rename7 = 5;
              const int cannon_packet_size_rename7 = 5;
              const int target_packet_size_rename7 = 7;
              const int wormhole_packet_size_rename7 = 5;
              int bytes_left_rename7 = 2000;
              int max_packet_rename7;
              int packet_count_rename7;
              x_rename7 = pl_rename7->pos.bx;
              y_rename7 = pl_rename7->pos.by;
              bv_rename7.world.x = x_rename7 - (horizontal_blocks >> 1);
              bv_rename7.world.y = y_rename7 - (vertical_blocks >> 1);
{}
              if (BIT(World.rules->mode, WRAP_PLAY))
              {
                loop_counter[89]++;
                if ((bv_rename7.world.x < 0) && ((bv_rename7.world.x + horizontal_blocks) < World.x))
                {
                  loop_counter[90]++;
                  bv_rename7.world.x += World.x;
                }
                else
                  if ((bv_rename7.world.x > 0) && ((bv_rename7.world.x + horizontal_blocks) > World.x))
                {
                  loop_counter[91]++;
{}
                }


                if ((bv_rename7.world.y < 0) && ((bv_rename7.world.y + vertical_blocks) < World.y))
                {
                  loop_counter[92]++;
                  bv_rename7.world.y += World.y;
                }
                else
                  if ((bv_rename7.world.y > 0) && ((bv_rename7.world.y + vertical_blocks) > World.y))
                {
                  loop_counter[93]++;
{}
                }


              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / target_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_target_update);
              for (k_rename7 = 0; k_rename7 < World.NumTargets; k_rename7++)
              {
                loop_counter[94]++;
                target_t *targ_rename7;
                if ((++i_rename7) >= World.NumTargets)
                {
                  loop_counter[95]++;
                  i_rename7 = 0;
                }

                targ_rename7 = &World.targets[i_rename7];
                if (BIT(targ_rename7->update_mask, conn_bit_rename7) || ((BIT(targ_rename7->conn_mask, conn_bit_rename7) == 0) && block_inview(&bv_rename7, targ_rename7->pos.x, targ_rename7->pos.y)))
                {
                  loop_counter[96]++;
{}
                  pl_rename7->last_target_update = i_rename7;
                  bytes_left_rename7 -= target_packet_size_rename7;
                  if ((++packet_count_rename7) >= max_packet_rename7)
                  {
                    loop_counter[97]++;
                    break;
                  }

                }

              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / cannon_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_cannon_update);
              for (k_rename7 = 0; k_rename7 < World.NumCannons; k_rename7++)
              {
                loop_counter[98]++;
                if ((++i_rename7) >= World.NumCannons)
                {
                  loop_counter[99]++;
                  i_rename7 = 0;
                }

                if (block_inview(&bv_rename7, World.cannon[i_rename7].blk_pos.x, World.cannon[i_rename7].blk_pos.y))
                {
                  loop_counter[100]++;
                  if (BIT(World.cannon[i_rename7].conn_mask, conn_bit_rename7) == 0)
                  {
                    loop_counter[101]++;
{}
                    pl_rename7->last_cannon_update = i_rename7;
                    bytes_left_rename7 -= max_packet_rename7 * cannon_packet_size_rename7;
                    if ((++packet_count_rename7) >= max_packet_rename7)
                    {
                      loop_counter[102]++;
                      break;
                    }

                  }

                }

              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / fuel_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_fuel_update);
              for (k_rename7 = 0; k_rename7 < World.NumFuels; k_rename7++)
              {
                loop_counter[103]++;
                if ((++i_rename7) >= World.NumFuels)
                {
                  loop_counter[104]++;
                  i_rename7 = 0;
                }

                if (BIT(World.fuel[i_rename7].conn_mask, conn_bit_rename7) == 0)
                {
                  loop_counter[105]++;
                  if (World.block[World.fuel[i_rename7].blk_pos.x][World.fuel[i_rename7].blk_pos.y] == FUEL)
                  {
                    loop_counter[106]++;
                    if (block_inview(&bv_rename7, World.fuel[i_rename7].blk_pos.x, World.fuel[i_rename7].blk_pos.y))
                    {
                      loop_counter[107]++;
{}
                      pl_rename7->last_fuel_update = i_rename7;
                      bytes_left_rename7 -= max_packet_rename7 * fuel_packet_size_rename7;
                      if ((++packet_count_rename7) >= max_packet_rename7)
                      {
                        loop_counter[108]++;
                        break;
                      }

                    }

                  }

                }

              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / wormhole_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_wormhole_update);
              for (k_rename7 = 0; k_rename7 < World.NumWormholes; k_rename7++)
              {
                loop_counter[109]++;
                wormhole_t *worm_rename7;
                if ((++i_rename7) >= World.NumWormholes)
                {
                  loop_counter[110]++;
                  i_rename7 = 0;
                }

                worm_rename7 = &World.wormHoles[i_rename7];
                if (((wormholeVisible && worm_rename7->temporary) && ((worm_rename7->type == WORM_IN) || (worm_rename7->type == WORM_NORMAL))) && block_inview(&bv_rename7, worm_rename7->pos.x, worm_rename7->pos.y))
                {
                  loop_counter[111]++;
                  int x_rename7 = (worm_rename7->pos.x * BLOCK_SZ) + (BLOCK_SZ / 2);
                  int y_rename7 = (worm_rename7->pos.y * BLOCK_SZ) + (BLOCK_SZ / 2);
{}
                  pl_rename7->last_wormhole_update = i_rename7;
                  bytes_left_rename7 -= max_packet_rename7 * wormhole_packet_size_rename7;
                  if ((++packet_count_rename7) >= max_packet_rename7)
                  {
                    loop_counter[112]++;
                    break;
                  }

                }

              }

              return7:
              ;

            }
            {
              int ind_rename8 = ind_rename1;
{}
              player *pl_rename8 = Players[ind_rename8];
              player *pl_i_rename8;
              pulse_t *pulse_rename8;
              int i_rename8;
              int j_rename8;
              int k_rename8;
{}
{}
              DFLOAT x_rename8;
              DFLOAT y_rename8;
              for (j_rename8 = 0; j_rename8 < NumPulses; j_rename8++)
              {
                loop_counter[113]++;
                pulse_rename8 = Pulses[j_rename8];
                if (pulse_rename8->len <= 0)
                {
                  loop_counter[114]++;
                  continue;
                }

                x_rename8 = pulse_rename8->pos.x;
                y_rename8 = pulse_rename8->pos.y;
                if (BIT(World.rules->mode, WRAP_PLAY))
                {
                  loop_counter[115]++;
                  if (x_rename8 < 0)
                  {
                    loop_counter[116]++;
                    x_rename8 += World.width;
                  }
                  else
                    if (x_rename8 >= World.width)
                  {
                    loop_counter[117]++;
                    x_rename8 -= World.width;
                  }


                  if (y_rename8 < 0)
                  {
                    loop_counter[118]++;
                    y_rename8 += World.height;
                  }
                  else
                    if (y_rename8 >= World.height)
                  {
                    loop_counter[119]++;
                    y_rename8 -= World.height;
                  }


                }

                if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                {
                  loop_counter[120]++;
{}
                }
                else
                {
                  x_rename8 += tcos(pulse_rename8->dir) * pulse_rename8->len;
                  y_rename8 += tsin(pulse_rename8->dir) * pulse_rename8->len;
                  if (BIT(World.rules->mode, WRAP_PLAY))
                  {
                    loop_counter[121]++;
                    if (x_rename8 < 0)
                    {
                      loop_counter[122]++;
                      x_rename8 += World.width;
                    }
                    else
                      if (x_rename8 >= World.width)
                    {
                      loop_counter[123]++;
                      x_rename8 -= World.width;
                    }


                    if (y_rename8 < 0)
                    {
                      loop_counter[124]++;
                      y_rename8 += World.height;
                    }
                    else
                      if (y_rename8 >= World.height)
                    {
                      loop_counter[125]++;
                      y_rename8 -= World.height;
                    }


                  }

                  if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[126]++;
{}
                  }
                  else
                  {
                    continue;
                  }

                }

                if (Team_immune(pulse_rename8->id, pl_rename8->id))
                {
                  loop_counter[127]++;
{}
                }
                else
                  if ((pulse_rename8->id == pl_rename8->id) && selfImmunity)
                {
                  loop_counter[128]++;
{}
                }
                else
                {
{}
                }


{}
              }

              for (i_rename8 = 0; i_rename8 < NumEcms; i_rename8++)
              {
                loop_counter[129]++;
{}
{}
              }

              for (i_rename8 = 0; i_rename8 < NumTransporters; i_rename8++)
              {
                loop_counter[130]++;
                trans_t *trans_rename8 = Transporters[i_rename8];
{}
                player *pl_rename8 = trans_rename8->id == NO_ID ? NULL : Players[GetInd[trans_rename8->id]];
                DFLOAT x_rename8 = pl_rename8 ? pl_rename8->pos.x : trans_rename8->pos.x;
                DFLOAT y_rename8 = pl_rename8 ? pl_rename8->pos.y : trans_rename8->pos.y;
{}
              }

              for (i_rename8 = 0; i_rename8 < World.NumCannons; i_rename8++)
              {
                loop_counter[131]++;
                cannon_t *cannon_rename8 = World.cannon + i_rename8;
                if (cannon_rename8->tractor_count > 0)
                {
                  loop_counter[132]++;
                  player *t_rename8 = Players[GetInd[cannon_rename8->tractor_target]];
                  if ((((t_rename8->pos.x > pv.world.x) && (t_rename8->pos.x < (pv.world.x + view_width))) || ((t_rename8->pos.x > pv.realWorld.x) && (t_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((t_rename8->pos.y > pv.world.y) && (t_rename8->pos.y < (pv.world.y + view_height))) || ((t_rename8->pos.y > pv.realWorld.y) && (t_rename8->pos.y < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[133]++;
                    int j_rename8;
                    for (j_rename8 = 0; j_rename8 < 3; j_rename8++)
                    {
                      loop_counter[134]++;
{}
                    }

                  }

                }

              }

              for (k_rename8 = 0; k_rename8 < num_player_shuffle; k_rename8++)
              {
                loop_counter[135]++;
                i_rename8 = player_shuffle_ptr[k_rename8];
                pl_i_rename8 = Players[i_rename8];
                if (!BIT(pl_i_rename8->status, PLAYING | PAUSE))
                {
                  loop_counter[136]++;
                  continue;
                }

                if (BIT(pl_i_rename8->status, GAME_OVER))
                {
                  loop_counter[137]++;
                  continue;
                }

                if (!((((pl_i_rename8->pos.x > pv.world.x) && (pl_i_rename8->pos.x < (pv.world.x + view_width))) || ((pl_i_rename8->pos.x > pv.realWorld.x) && (pl_i_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((pl_i_rename8->pos.y > pv.world.y) && (pl_i_rename8->pos.y < (pv.world.y + view_height))) || ((pl_i_rename8->pos.y > pv.realWorld.y) && (pl_i_rename8->pos.y < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[138]++;
                  continue;
                }

                if (BIT(pl_i_rename8->status, PAUSE))
                {
                  loop_counter[139]++;
{}
                  continue;
                }

                if (((pl_rename8->visibility[i_rename8].canSee || (i_rename8 == ind_rename8)) || TEAM(i_rename8, ind_rename8)) || ALLIANCE(i_rename8, ind_rename8))
                {
                  loop_counter[140]++;
{}
                }

                if (BIT(pl_i_rename8->used, HAS_REFUEL))
                {
                  loop_counter[141]++;
                  if ((((World.fuel[pl_i_rename8->fs].pix_pos.x > pv.world.x) && (World.fuel[pl_i_rename8->fs].pix_pos.x < (pv.world.x + view_width))) || ((World.fuel[pl_i_rename8->fs].pix_pos.x > pv.realWorld.x) && (World.fuel[pl_i_rename8->fs].pix_pos.x < (pv.realWorld.x + view_width)))) && (((World.fuel[pl_i_rename8->fs].pix_pos.y > pv.world.y) && (World.fuel[pl_i_rename8->fs].pix_pos.y < (pv.world.y + view_height))) || ((World.fuel[pl_i_rename8->fs].pix_pos.y > pv.realWorld.y) && (World.fuel[pl_i_rename8->fs].pix_pos.y < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[142]++;
{}
                  }

                }

                if (BIT(pl_i_rename8->used, HAS_REPAIR))
                {
                  loop_counter[143]++;
                  DFLOAT x_rename8 = ((DFLOAT) (World.targets[pl_i_rename8->repair_target].pos.x + 0.5)) * BLOCK_SZ;
                  DFLOAT y_rename8 = ((DFLOAT) (World.targets[pl_i_rename8->repair_target].pos.y + 0.5)) * BLOCK_SZ;
                  if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[144]++;
{}
                  }

                }

                if (BIT(pl_i_rename8->used, HAS_TRACTOR_BEAM))
                {
                  loop_counter[145]++;
                  player *t_rename8 = Players[GetInd[pl_i_rename8->lock.pl_id]];
                  if ((((t_rename8->pos.x > pv.world.x) && (t_rename8->pos.x < (pv.world.x + view_width))) || ((t_rename8->pos.x > pv.realWorld.x) && (t_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((t_rename8->pos.y > pv.world.y) && (t_rename8->pos.y < (pv.world.y + view_height))) || ((t_rename8->pos.y > pv.realWorld.y) && (t_rename8->pos.y < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[146]++;
                    int j_rename8;
                    for (j_rename8 = 0; j_rename8 < 3; j_rename8++)
                    {
                      loop_counter[147]++;
{}
                    }

                  }

                }

                if ((pl_i_rename8->ball != NULL) && ((((pl_i_rename8->ball->pos.x > pv.world.x) && (pl_i_rename8->ball->pos.x < (pv.world.x + view_width))) || ((pl_i_rename8->ball->pos.x > pv.realWorld.x) && (pl_i_rename8->ball->pos.x < (pv.realWorld.x + view_width)))) && (((pl_i_rename8->ball->pos.y > pv.world.y) && (pl_i_rename8->ball->pos.y < (pv.world.y + view_height))) || ((pl_i_rename8->ball->pos.y > pv.realWorld.y) && (pl_i_rename8->ball->pos.y < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[148]++;
{}
                }

              }

              return8:
              ;

            }
            {
              int ind_rename9 = ind_rename1;
{}
              player *pl_rename9 = Players[ind_rename9];
              register int x_rename9;
              register int y_rename9;
              int i_rename9;
              int k_rename9;
              int color_rename9;
              int fuzz_rename9 = 0;
              int teamshot_rename9;
{}
              int obj_count_rename9;
              object *shot_rename9;
              object **obj_list_rename9;
              int hori_blocks_rename9;
              int vert_blocks_rename9;
              hori_blocks_rename9 = (view_width + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
              vert_blocks_rename9 = (view_height + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
              Cell_get_objects(OBJ_X_IN_BLOCKS(pl_rename9), OBJ_Y_IN_BLOCKS(pl_rename9), MAX(hori_blocks_rename9, vert_blocks_rename9), num_object_shuffle, &obj_list_rename9, &obj_count_rename9);
              for (k_rename9 = 0; k_rename9 < num_object_shuffle; k_rename9++)
              {
                loop_counter[149]++;
                i_rename9 = object_shuffle_ptr[k_rename9];
                if (i_rename9 >= obj_count_rename9)
                {
                  loop_counter[150]++;
                  continue;
                }

                shot_rename9 = obj_list_rename9[i_rename9];
                x_rename9 = shot_rename9->pos.x;
                y_rename9 = shot_rename9->pos.y;
                if (!((((x_rename9 > pv.world.x) && (x_rename9 < (pv.world.x + view_width))) || ((x_rename9 > pv.realWorld.x) && (x_rename9 < (pv.realWorld.x + view_width)))) && (((y_rename9 > pv.world.y) && (y_rename9 < (pv.world.y + view_height))) || ((y_rename9 > pv.realWorld.y) && (y_rename9 < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[151]++;
                  continue;
                }

                if ((color_rename9 = shot_rename9->color) == BLACK)
                {
                  loop_counter[152]++;
{}
                  color_rename9 = WHITE;
                }

                switch (shot_rename9->type)
                {
                  case OBJ_SPARK:
                    loop_counter[153]++;

                  case OBJ_DEBRIS:
                    loop_counter[154]++;
                    if ((fuzz_rename9 >>= 7) < 0x40)
                  {
                    loop_counter[155]++;
                    fuzz_rename9 = randomMT();
                  }

                    if ((fuzz_rename9 & 0x7F) >= spark_rand)
                  {
                    loop_counter[156]++;
                    break;
                  }

                    if (debris_colors >= 3)
                  {
                    loop_counter[157]++;
                    if (debris_colors > 4)
                    {
                      loop_counter[158]++;
                      if (color_rename9 == BLUE)
                      {
                        loop_counter[159]++;
                        color_rename9 = shot_rename9->life >> 1;
                      }
                      else
                      {
                        color_rename9 = shot_rename9->life >> 2;
                      }

                    }
                    else
                    {
                      if (color_rename9 == BLUE)
                      {
                        loop_counter[160]++;
                        color_rename9 = shot_rename9->life >> 2;
                      }
                      else
                      {
                        color_rename9 = shot_rename9->life >> 3;
                      }

                    }

                    if (color_rename9 >= debris_colors)
                    {
                      loop_counter[161]++;
                      color_rename9 = debris_colors - 1;
                    }

                  }

                  {
                    int color_rename21 = color_rename9;
                    int yf_rename21 = (int) (shot_rename9->pos.y - pv.world.y);
                    int xf_rename21 = (int) (shot_rename9->pos.x - pv.world.x);
                    int i_rename21;
                    if (xf_rename21 < 0)
                    {
                      loop_counter[162]++;
                      xf_rename21 += World.width;
                    }

                    if (yf_rename21 < 0)
                    {
                      loop_counter[163]++;
                      yf_rename21 += World.height;
                    }

                    if ((((unsigned) xf_rename21) >= ((unsigned) view_width)) || (((unsigned) yf_rename21) >= ((unsigned) view_height)))
                    {
                      loop_counter[164]++;
                      {
                        goto return21;
                      }
                    }

                    i_rename21 = ((0 + (color_rename21 * debris_areas)) + (((yf_rename21 >> 8) % debris_y_areas) * debris_x_areas)) + ((xf_rename21 >> 8) % debris_x_areas);
                    if (debris_num[i_rename21] >= 255)
                    {
                      loop_counter[165]++;
                      {
                        goto return21;
                      }
                    }

                    if (debris_num[i_rename21] >= debris_max[i_rename21])
                    {
                      loop_counter[166]++;
                      if (debris_num[i_rename21] == 0)
                      {
                        loop_counter[167]++;
                        debris_ptr[i_rename21] = (debris_t *) malloc((debris_max[i_rename21] = 16) * (sizeof(*debris_ptr[i_rename21])));
                      }
                      else
                      {
                        debris_ptr[i_rename21] = (debris_t *) realloc(debris_ptr[i_rename21], (debris_max[i_rename21] += debris_max[i_rename21]) * (sizeof(*debris_ptr[i_rename21])));
                      }

                      if (debris_ptr[i_rename21] == 0)
                      {
                        loop_counter[168]++;
{}
                        debris_num[i_rename21] = 0;
                        {
                          goto return21;
                        }
                      }

                    }

{}
{}
                    debris_num[i_rename21]++;
                    return21:
                    ;

                  }
                    break;

                  case OBJ_WRECKAGE:
                    loop_counter[169]++;
                    if ((spark_rand != 0) || wreckageCollisionMayKill)
                  {
                    loop_counter[170]++;
{}
{}
                  }

                    break;

                  case OBJ_ASTEROID:
                    loop_counter[171]++;
                  {
{}
{}
                  }
                    break;

                  case OBJ_SHOT:
                    loop_counter[172]++;

                  case OBJ_CANNON_SHOT:
                    loop_counter[173]++;
                    if ((Team_immune(shot_rename9->id, pl_rename9->id) || ((shot_rename9->id != NO_ID) && BIT(Players[GetInd[shot_rename9->id]]->status, PAUSE))) || (((shot_rename9->id == NO_ID) && BIT(World.rules->mode, TEAM_PLAY)) && (shot_rename9->team == pl_rename9->team)))
                  {
                    loop_counter[174]++;
                    color_rename9 = BLUE;
                    teamshot_rename9 = DEBRIS_TYPES;
                  }
                  else
                    if ((shot_rename9->id == pl_rename9->id) && selfImmunity)
                  {
                    loop_counter[175]++;
                    color_rename9 = BLUE;
                    teamshot_rename9 = DEBRIS_TYPES;
                  }
                  else
                    if (shot_rename9->mods.nuclear && (frame_loops & 2))
                  {
                    loop_counter[176]++;
                    color_rename9 = RED;
                    teamshot_rename9 = DEBRIS_TYPES;
                  }
                  else
                  {
                    teamshot_rename9 = 0;
                  }



                  {
                    int offset_rename22 = teamshot_rename9;
                    int color_rename22 = color_rename9;
                    int yf_rename22 = (int) (shot_rename9->pos.y - pv.world.y);
                    int xf_rename22 = (int) (shot_rename9->pos.x - pv.world.x);
                    int i_rename22;
                    if (xf_rename22 < 0)
                    {
                      loop_counter[177]++;
                      xf_rename22 += World.width;
                    }

                    if (yf_rename22 < 0)
                    {
                      loop_counter[178]++;
                      yf_rename22 += World.height;
                    }

                    if ((((unsigned) xf_rename22) >= ((unsigned) view_width)) || (((unsigned) yf_rename22) >= ((unsigned) view_height)))
                    {
                      loop_counter[179]++;
                      {
                        goto return22;
                      }
                    }

                    i_rename22 = ((offset_rename22 + (color_rename22 * debris_areas)) + (((yf_rename22 >> 8) % debris_y_areas) * debris_x_areas)) + ((xf_rename22 >> 8) % debris_x_areas);
                    if (fastshot_num[i_rename22] >= 255)
                    {
                      loop_counter[180]++;
                      {
                        goto return22;
                      }
                    }

                    if (fastshot_num[i_rename22] >= fastshot_max[i_rename22])
                    {
                      loop_counter[181]++;
                      if (fastshot_num[i_rename22] == 0)
                      {
                        loop_counter[182]++;
                        fastshot_ptr[i_rename22] = (debris_t *) malloc((fastshot_max[i_rename22] = 16) * (sizeof(*fastshot_ptr[i_rename22])));
                      }
                      else
                      {
                        fastshot_ptr[i_rename22] = (debris_t *) realloc(fastshot_ptr[i_rename22], (fastshot_max[i_rename22] += fastshot_max[i_rename22]) * (sizeof(*fastshot_ptr[i_rename22])));
                      }

                      if (fastshot_ptr[i_rename22] == 0)
                      {
                        loop_counter[183]++;
{}
                        fastshot_num[i_rename22] = 0;
                        {
                          goto return22;
                        }
                      }

                    }

{}
{}
                    fastshot_num[i_rename22]++;
                    return22:
                    ;

                  }
                    break;

                  case OBJ_TORPEDO:
                    loop_counter[184]++;
{}
{}
                    break;

                  case OBJ_SMART_SHOT:
                    loop_counter[185]++;
{}
{}
                    break;

                  case OBJ_HEAT_SHOT:
                    loop_counter[186]++;
{}
{}
                    break;

                  case OBJ_BALL:
                    loop_counter[187]++;
{}
                    break;

                  case OBJ_MINE:
                    loop_counter[188]++;
                  {
                    int id_rename9 = 0;
{}
                    int confused_rename9 = 0;
                    mineobject *mine_rename9 = MINE_PTR(shot_rename9);
                    if (identifyMines && (Wrap_length(pl_rename9->pos.x - mine_rename9->pos.x, pl_rename9->pos.y - mine_rename9->pos.y) < ((SHIP_SZ + MINE_SENSE_BASE_RANGE) + (pl_rename9->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))))
                    {
                      loop_counter[189]++;
                      id_rename9 = mine_rename9->id;
                      if (id_rename9 == NO_ID)
                      {
                        loop_counter[190]++;
                        id_rename9 = EXPIRED_MINE_ID;
                      }

                      if (BIT(mine_rename9->status, CONFUSED))
                      {
                        loop_counter[191]++;
                        confused_rename9 = 1;
                      }

                    }

                    if ((mine_rename9->id != NO_ID) && BIT(Players[GetInd[mine_rename9->id]]->status, PAUSE))
                    {
                      loop_counter[192]++;
{}
                    }
                    else
                    {
{}
                      if (confused_rename9)
                      {
                        loop_counter[193]++;
                        id_rename9 = 0;
{}
                      }

                    }

{}
                  }
                    break;

                  case OBJ_ITEM:
                    loop_counter[194]++;
                  {
{}
                    if (BIT(shot_rename9->status, RANDOM_ITEM))
                    {
                      loop_counter[195]++;
{}
                    }

{}
                  }
                    break;

                  default:
{}
                    break;

                }

              }

              return9:
              ;

            }
            {
              int ind_rename10 = ind_rename1;
              int conn_rename10 = conn_rename1;
              int i_rename10;
              int k_rename10;
              int mask_rename10;
              int shownuke_rename10;
{}
              player *pl_rename10 = Players[ind_rename10];
              object *shot_rename10;
              DFLOAT x_rename10;
              DFLOAT y_rename10;
              {
                num_radar = 0;
                return23:
                ;

              }
              if (nukesOnRadar)
              {
                loop_counter[196]++;
                mask_rename10 = ((OBJ_SMART_SHOT | OBJ_TORPEDO) | OBJ_HEAT_SHOT) | OBJ_MINE;
              }
              else
              {
                mask_rename10 = missilesOnRadar ? (OBJ_SMART_SHOT | OBJ_TORPEDO) | OBJ_HEAT_SHOT : 0;
                mask_rename10 |= minesOnRadar ? OBJ_MINE : 0;
              }

              if (treasuresOnRadar)
              {
                loop_counter[197]++;
                mask_rename10 |= OBJ_BALL;
              }

              if (asteroidsOnRadar)
              {
                loop_counter[198]++;
                mask_rename10 |= OBJ_ASTEROID;
              }

              if (mask_rename10)
              {
                loop_counter[199]++;
                for (i_rename10 = 0; i_rename10 < NumObjs; i_rename10++)
                {
                  loop_counter[200]++;
                  shot_rename10 = Obj[i_rename10];
                  if (!BIT(shot_rename10->type, mask_rename10))
                  {
                    loop_counter[201]++;
                    continue;
                  }

                  shownuke_rename10 = nukesOnRadar && shot_rename10->mods.nuclear;
                  if (shownuke_rename10 && (frame_loops & 2))
                  {
                    loop_counter[202]++;
{}
                  }
                  else
                  {
{}
                  }

                  if (BIT(shot_rename10->type, OBJ_MINE))
                  {
                    loop_counter[203]++;
                    if ((!minesOnRadar) && (!shownuke_rename10))
                    {
                      loop_counter[204]++;
                      continue;
                    }

                    if ((frame_loops % 8) >= 6)
                    {
                      loop_counter[205]++;
                      continue;
                    }

                  }
                  else
                    if (BIT(shot_rename10->type, OBJ_BALL))
                  {
                    loop_counter[206]++;
{}
                  }
                  else
                    if (BIT(shot_rename10->type, OBJ_ASTEROID))
                  {
                    loop_counter[207]++;
{}
{}
                  }
                  else
                  {
                    if ((!missilesOnRadar) && (!shownuke_rename10))
                    {
                      loop_counter[208]++;
                      continue;
                    }

                    if (frame_loops & 1)
                    {
                      loop_counter[209]++;
                      continue;
                    }

                  }



                  x_rename10 = shot_rename10->pos.x;
                  y_rename10 = shot_rename10->pos.y;
                  if (Wrap_length(pl_rename10->pos.x - x_rename10, pl_rename10->pos.y - y_rename10) <= pl_rename10->sensor_range)
                  {
                    loop_counter[210]++;
                    {
{}
{}
{}
                      radar_t *p_rename24;
                      if ((num_radar + 1) > max_radar)
                      {
                        loop_counter[211]++;
                        if (max_radar <= 0)
                        {
                          loop_counter[212]++;
                          max_radar = 1 + 2;
                          radar_ptr = (radar_t *) malloc(max_radar * (sizeof(radar_t)));
                          num_radar = 0;
                        }
                        else
                        {
                          max_radar = (max_radar << 1) + 1;
                          radar_ptr = (radar_t *) realloc(radar_ptr, max_radar * (sizeof(radar_t)));
                        }

                        if (radar_ptr == NULL)
                        {
                          loop_counter[213]++;
{}
                          num_radar = (max_radar = 0);
                          {
                            goto return24;
                          }
                        }

                      }

                      p_rename24 = &radar_ptr[num_radar++];
{}
{}
{}
                      return24:
                      ;

                    }
                  }

                }

              }

              if (((playersOnRadar || BIT(World.rules->mode, TEAM_PLAY)) || (NumPseudoPlayers > 0)) || (NumAlliances > 0))
              {
                loop_counter[214]++;
                for (k_rename10 = 0; k_rename10 < num_player_shuffle; k_rename10++)
                {
                  loop_counter[215]++;
                  i_rename10 = player_shuffle_ptr[k_rename10];
                  if (((Players[i_rename10]->conn == conn_rename10) || (BIT(Players[i_rename10]->status, (PLAYING | PAUSE) | GAME_OVER) != PLAYING)) || ((((!TEAM(i_rename10, ind_rename10)) && (!ALLIANCE(ind_rename10, i_rename10))) && (!OWNS_TANK(ind_rename10, i_rename10))) && ((!playersOnRadar) || (!pl_rename10->visibility[i_rename10].canSee))))
                  {
                    loop_counter[216]++;
                    continue;
                  }

                  x_rename10 = Players[i_rename10]->pos.x;
                  y_rename10 = Players[i_rename10]->pos.y;
                  if (BIT(World.rules->mode, LIMITED_VISIBILITY) && (Wrap_length(pl_rename10->pos.x - x_rename10, pl_rename10->pos.y - y_rename10) > pl_rename10->sensor_range))
                  {
                    loop_counter[217]++;
                    continue;
                  }

                  if (((BIT(pl_rename10->used, HAS_COMPASS) && BIT(pl_rename10->lock.tagged, LOCK_PLAYER)) && (GetInd[pl_rename10->lock.pl_id] == i_rename10)) && ((frame_loops % 5) >= 3))
                  {
                    loop_counter[218]++;
                    continue;
                  }

{}
                  if ((TEAM(i_rename10, ind_rename10) || ALLIANCE(ind_rename10, i_rename10)) || OWNS_TANK(ind_rename10, i_rename10))
                  {
                    loop_counter[219]++;
{}
                  }

                  {
{}
{}
{}
                    radar_t *p_rename25;
                    if ((num_radar + 1) > max_radar)
                    {
                      loop_counter[220]++;
                      if (max_radar <= 0)
                      {
                        loop_counter[221]++;
                        max_radar = 1 + 2;
                        radar_ptr = (radar_t *) malloc(max_radar * (sizeof(radar_t)));
                        num_radar = 0;
                      }
                      else
                      {
                        max_radar = (max_radar << 1) + 1;
                        radar_ptr = (radar_t *) realloc(radar_ptr, max_radar * (sizeof(radar_t)));
                      }

                      if (radar_ptr == NULL)
                      {
                        loop_counter[222]++;
{}
                        num_radar = (max_radar = 0);
                        {
                          goto return25;
                        }
                      }

                    }

                    p_rename25 = &radar_ptr[num_radar++];
{}
{}
{}
                    return25:
                    ;

                  }
                }

              }

              {
                int conn_rename26 = conn_rename10;
                int i_rename26;
                int dest_rename26;
                int tmp_rename26;
                radar_t *p_rename26;
                const int radar_width_rename26 = 256;
                int radar_height_rename26 = (radar_width_rename26 * World.y) / World.x;
{}
                int radar_y_rename26;
{}
{}
                shuffle_t *radar_shuffle_rename26;
                size_t shuffle_bufsize_rename26;
                if (num_radar > MIN(256, 65535))
                {
                  loop_counter[223]++;
                  num_radar = MIN(256, 65535);
                }

                shuffle_bufsize_rename26 = num_radar * (sizeof(shuffle_t));
                radar_shuffle_rename26 = (shuffle_t *) malloc(shuffle_bufsize_rename26);
                if (radar_shuffle_rename26 == ((shuffle_t *) NULL))
                {
                  loop_counter[224]++;
                  {
                    goto return26;
                  }
                }

                for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                {
                  loop_counter[225]++;
                  radar_shuffle_rename26[i_rename26] = i_rename26;
                }

                for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                {
                  loop_counter[226]++;
                  dest_rename26 = (int) (rfrac() * num_radar);
                  tmp_rename26 = radar_shuffle_rename26[i_rename26];
                  radar_shuffle_rename26[i_rename26] = radar_shuffle_rename26[dest_rename26];
                  radar_shuffle_rename26[dest_rename26] = tmp_rename26;
                }

                if (Get_conn_version(conn_rename26) <= 0x4400)
                {
                  loop_counter[227]++;
                  for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                  {
                    loop_counter[228]++;
                    p_rename26 = &radar_ptr[radar_shuffle_rename26[i_rename26]];
{}
                    radar_y_rename26 = (radar_height_rename26 * p_rename26->y) / World.height;
{}
{}
{}
                  }

                }
                else
                {
{}
{}
                  int fast_count_rename26 = 0;
                  if (num_radar > 256)
                  {
                    loop_counter[229]++;
                    num_radar = 256;
                  }

                  for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                  {
                    loop_counter[230]++;
                    p_rename26 = &radar_ptr[radar_shuffle_rename26[i_rename26]];
{}
                    radar_y_rename26 = (radar_height_rename26 * p_rename26->y) / World.height;
                    if (radar_y_rename26 >= 1024)
                    {
                      loop_counter[231]++;
                      continue;
                    }

{}
{}
{}
                    if (p_rename26->size & 0x80)
                    {
                      loop_counter[232]++;
{}
                    }

{}
{}
                    fast_count_rename26++;
                  }

                  if (fast_count_rename26 > 0)
                  {
                    loop_counter[233]++;
{}
                  }

                }

{}
                return26:
                ;

              }
              return10:
              ;

            }
            {
              int ind_rename11 = i_rename1;
              player *pl_rename11 = Players[ind_rename11];
              if (pl_rename11->lose_item_state != 0)
              {
                loop_counter[234]++;
{}
                if (pl_rename11->lose_item_state == 1)
                {
                  loop_counter[235]++;
                  pl_rename11->lose_item_state = -5;
                }

                if (pl_rename11->lose_item_state < 0)
                {
                  loop_counter[236]++;
                  pl_rename11->lose_item_state++;
                }

              }

              return11:
              ;

            }
            {
{}
              int i_rename12;
              for (i_rename12 = 0; i_rename12 < DEBRIS_TYPES; i_rename12++)
              {
                loop_counter[237]++;
                if (debris_num[i_rename12] != 0)
                {
                  loop_counter[238]++;
{}
                  debris_num[i_rename12] = 0;
                }

              }

              return12:
              ;

            }
            {
{}
              int i_rename13;
              for (i_rename13 = 0; i_rename13 < (DEBRIS_TYPES * 2); i_rename13++)
              {
                loop_counter[239]++;
                if (fastshot_num[i_rename13] != 0)
                {
                  loop_counter[240]++;
{}
                  fastshot_num[i_rename13] = 0;
                }

              }

              return13:
              ;

            }
          }

{}
{}
        }

        oldTimeLeft_rename1 = newTimeLeft_rename1;
        {
{}
          radar_ptr = NULL;
          num_radar = 0;
          max_radar = 0;
          return14:
          ;

        }
        return1:
        ;

      }
    }

  }

  if ((((!NoQuit) && (NumPlayers == (NumRobots + NumPseudoPlayers))) && (!login_in_progress)) && (!NumQueuedPlayers))
  {
    loop_counter[241]++;
    if (!NoPlayersEnteredYet)
    {
      loop_counter[242]++;
      {
        int return_value;
        player *pl_rename2;
{}
        if (ShutdownServer == 0)
        {
          loop_counter[243]++;
{}
{}
{}
        }
        else
        {
{}
        }

        while (NumPlayers > 0)
        {
          loop_counter[244]++;
          pl_rename2 = Players[NumPlayers - 1];
          if (pl_rename2->conn == NOT_CONNECTED)
          {
            loop_counter[245]++;
{}
          }
          else
          {
{}
          }

        }

{}
{}
{}
{}
{}
{}
{}
{}
{}
        {
          return_value = FALSE;
          goto return2;
        }
        return2:
        ;

      }
    }

    if ((serverTime + (5 * 60)) < time(NULL))
    {
      loop_counter[246]++;
{}
{}
      {
        int return_value;
        player *pl_rename3;
{}
        if (ShutdownServer == 0)
        {
          loop_counter[247]++;
{}
{}
{}
        }
        else
        {
{}
        }

        while (NumPlayers > 0)
        {
          loop_counter[248]++;
          pl_rename3 = Players[NumPlayers - 1];
          if (pl_rename3->conn == NOT_CONNECTED)
          {
            loop_counter[249]++;
{}
          }
          else
          {
{}
          }

        }

{}
{}
{}
{}
{}
{}
{}
{}
{}
        {
          return_value = FALSE;
          goto return3;
        }
        return3:
        ;

      }
    }

  }

{}
{}
{}
  {
    print_loop_counter:
    ;
#if GET_PREDICT || DEBUG_EN
    //250
    print_array(loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]));
#endif
 }
  {
    predict_exec_time:
    ;
    struct slice_return exec_time;
    exec_time.big = exec_time.little = 0; //initialize
#if !ONLINE_EN
  #if !CVX_EN //off-line training with conservative
    #if ARCH_ARM
      exec_time.little = 42.658269*loop_counter[6] + 42.951612*loop_counter[7] + 42.951612*loop_counter[8] + 42.658269*loop_counter[10] + 42.658269*loop_counter[12] + 104.188570*loop_counter[13] + -125.202109*loop_counter[14] + 0.242605*loop_counter[17] + 0.242605*loop_counter[18] + -1.277374*loop_counter[19] + 40.765133*loop_counter[20] + -139.934660*loop_counter[21] + 30.135075*loop_counter[24] + 30.135075*loop_counter[25] + 30.135075*loop_counter[28] + 13.599658*loop_counter[29] + 42.951612*loop_counter[36] + -18.774036*loop_counter[42] + 2.367718*loop_counter[45] + 90.570905*loop_counter[46] + -18.774036*loop_counter[89] + 2.367718*loop_counter[92] + -51.041617*loop_counter[93] + -1.985355*loop_counter[98] + -18.774070*loop_counter[99] + -5.358812*loop_counter[100] + 61.268364*loop_counter[101] + -4.897612*loop_counter[103] + -18.774021*loop_counter[104] + 26.812213*loop_counter[105] + 26.812216*loop_counter[106] + -44.835503*loop_counter[107] + -6.638780*loop_counter[109] + -18.774119*loop_counter[110] + -1.985347*loop_counter[131] + 8.080802*loop_counter[135] + -50.600956*loop_counter[136] + 25.662404*loop_counter[138] + 14.956417*loop_counter[140] + 0.242607*loop_counter[149] + 0.624077*loop_counter[150] + 0.899168*loop_counter[151] + -1.749277*loop_counter[153] + 0.894986*loop_counter[154] + -0.821165*loop_counter[155] + 1.528767*loop_counter[156] + 1.208506*loop_counter[157] + 1.377955*loop_counter[161] + -0.778090*loop_counter[163] + -10.447335*loop_counter[166] + 15.161115*loop_counter[167] + 1.452407*loop_counter[169] + 1.452406*loop_counter[170] + 44.874992*loop_counter[172] + -41.216763*loop_counter[173] + -1.859820*loop_counter[178] + -5.437280*loop_counter[181] + -5.437279*loop_counter[182] + 53.440094*loop_counter[194] + -18.774197*loop_counter[196] + -18.774197*loop_counter[199] + 0.242606*loop_counter[200] + 0.242606*loop_counter[201] + -18.774117*loop_counter[214] + 8.080788*loop_counter[215] + -22.701469*loop_counter[216] + 3.349769*loop_counter[220] + 181.032404*loop_counter[221] + 25.662418*loop_counter[225] + 25.662418*loop_counter[226] + 25.662418*loop_counter[230] + -75.594917*loop_counter[231] + 24.423327*loop_counter[232] + 181.032412*loop_counter[233] + -0.493370*loop_counter[237] + -4.969237*loop_counter[238] + -0.293346*loop_counter[239] + -3.611701*loop_counter[240] + 146.000000;
    #elif ARCH_X86
      exec_time.little = -1.749813*loop_counter[0] +
        12.161031*loop_counter[6] + 12.680051*loop_counter[7] +
        12.680051*loop_counter[8] + 12.161031*loop_counter[10] +
        12.161031*loop_counter[12] + 66.276412*loop_counter[13] +
        -62.093513*loop_counter[14] + 0.019267*loop_counter[17] +
        0.019267*loop_counter[18] + 0.294487*loop_counter[19] +
        -43.694222*loop_counter[20] + -9.397756*loop_counter[21] +
        7.387092*loop_counter[24] + 7.387092*loop_counter[25] +
        7.387092*loop_counter[28] + 1.506657*loop_counter[29] +
        12.680051*loop_counter[36] + -0.434345*loop_counter[42] +
        -12.580408*loop_counter[45] + -6.533515*loop_counter[46] +
        -0.434345*loop_counter[89] + -12.580408*loop_counter[92] +
        -21.203745*loop_counter[93] + -0.021717*loop_counter[98] +
        -0.434345*loop_counter[99] + 5.458958*loop_counter[100] +
        -0.072391*loop_counter[103] + -0.434345*loop_counter[104] +
        -1.213000*loop_counter[105] + -1.213000*loop_counter[106] +
        3.362755*loop_counter[107] + -0.108586*loop_counter[109] +
        -0.434345*loop_counter[110] + -0.021717*loop_counter[131] +
        0.917959*loop_counter[135] + -9.735127*loop_counter[136] +
        2.424421*loop_counter[138] + 6.066312*loop_counter[140] +
        0.019267*loop_counter[149] + -0.006763*loop_counter[150] +
        0.172421*loop_counter[151] + 0.129689*loop_counter[153] +
        -0.075057*loop_counter[154] + 0.136155*loop_counter[155] +
        -0.289091*loop_counter[156] + -0.006296*loop_counter[157] +
        0.406778*loop_counter[161] + 0.656212*loop_counter[163] +
        47.131603*loop_counter[166] + -54.238906*loop_counter[167] +
        3.559664*loop_counter[169] + 3.559664*loop_counter[170] +
        -6.134230*loop_counter[172] + 5.854066*loop_counter[173] +
        33.503108*loop_counter[178] + 13.767256*loop_counter[181] +
        13.767256*loop_counter[182] + 2.988774*loop_counter[194] +
        -0.434345*loop_counter[196] + -0.434345*loop_counter[199] +
        0.019267*loop_counter[200] + 0.019267*loop_counter[201] +
        -0.434345*loop_counter[214] + 0.917959*loop_counter[215] +
        -2.811466*loop_counter[216] + 12.435963*loop_counter[220] +
        1.904836*loop_counter[221] + 2.424421*loop_counter[225] +
        2.424421*loop_counter[226] + 2.424421*loop_counter[230] +
        -0.875786*loop_counter[232] + 1.904836*loop_counter[233] +
        -0.003393*loop_counter[237] + -1.262241*loop_counter[238] +
        -0.001697*loop_counter[239] + -0.754526*loop_counter[240] + 82.984818;
    #endif
  #else //off-line training with cvx    
    #if ARCH_ARM
      exec_time.little = 73.021006*loop_counter[6] + 76.173244*loop_counter[7] + 76.173244*loop_counter[8] + 73.021006*loop_counter[10] + 73.021006*loop_counter[12] + -138.004059*loop_counter[13] + 60.978864*loop_counter[14] + 0.307663*loop_counter[17] + 0.307663*loop_counter[18] + -0.078769*loop_counter[19] + -197.645518*loop_counter[20] + -45.645856*loop_counter[21] + 39.764147*loop_counter[24] + 39.764147*loop_counter[25] + 39.764147*loop_counter[28] + -4.699431*loop_counter[29] + 76.173244*loop_counter[36] + -1.748293*loop_counter[42] + -13.894581*loop_counter[45] + -86.667674*loop_counter[46] + -1.748293*loop_counter[89] + -13.894581*loop_counter[92] + 248.361463*loop_counter[93] + -0.087414*loop_counter[98] + -1.748293*loop_counter[99] + -25.084109*loop_counter[100] + 16.888545*loop_counter[101] + -0.291383*loop_counter[103] + -1.748292*loop_counter[104] + 19.252404*loop_counter[105] + 19.252404*loop_counter[106] + -41.083318*loop_counter[107] + -0.437073*loop_counter[109] + -1.748293*loop_counter[110] + -0.087414*loop_counter[131] + -3.027530*loop_counter[135] + -44.396228*loop_counter[136] + -0.141697*loop_counter[138] + -23.730869*loop_counter[140] + 0.307663*loop_counter[149] + 0.074029*loop_counter[150] + -0.438259*loop_counter[151] + 1.629584*loop_counter[153] + 0.383114*loop_counter[154] + -6.451288*loop_counter[155] + 0.995918*loop_counter[156] + 0.504963*loop_counter[157] + 13.127920*loop_counter[161] + -2.466878*loop_counter[163] + -146.691300*loop_counter[166] + 172.918993*loop_counter[167] + -11.604459*loop_counter[169] + -11.604459*loop_counter[170] + -40.002057*loop_counter[172] + 39.661709*loop_counter[173] + -7.923858*loop_counter[178] + -124.573144*loop_counter[181] + -124.573144*loop_counter[182] + 16.825388*loop_counter[194] + -1.748296*loop_counter[196] + -1.748296*loop_counter[199] + 0.307663*loop_counter[200] + 0.307663*loop_counter[201] + -1.748296*loop_counter[214] + -3.027530*loop_counter[215] + -29.863692*loop_counter[216] + 89.143432*loop_counter[220] + 223.361272*loop_counter[221] + -0.141697*loop_counter[225] + -0.141697*loop_counter[226] + -0.141697*loop_counter[230] + -54.742302*loop_counter[231] + 171.969370*loop_counter[232] + 223.361273*loop_counter[233] + -0.013659*loop_counter[237] + 9.656253*loop_counter[238] + -0.006829*loop_counter[239] + -5.689641*loop_counter[240] + 164.825193;
    #elif ARCH_X86
      exec_time.little = 0;
    #endif
  #endif
#elif ONLINE_EN
  #if !HETERO_EN
    #if CORE //on-line training on big core
      exec_time.big    = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #else //on-line training on little core
      exec_time.little = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #endif
  #elif HETERO_EN
    exec_time.big    = get_predicted_time_big(TYPE_PREDICT, solver_big,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    exec_time.little = get_predicted_time_little(TYPE_PREDICT, solver_little,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
  #endif
#endif
    return exec_time;
  }
}


/*
 * Main_loop with loop counts
 */
void Main_loop_loop_counters(void)
{
  int loop_counter[238] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  start_timing();
  main_loops++;
  if ((main_loops & 0x3F) == 0)
  {
    loop_counter[0]++;
    Meta_update(0);
  }

  if (ShutdownServer >= 0)
  {
    loop_counter[1]++;
    if (ShutdownServer == 0)
    {
      loop_counter[2]++;
      {
        int return_value;
        player *pl_rename0;
        char msg_rename0[MSG_LEN];
        if (ShutdownServer == 0)
        {
          loop_counter[3]++;
          errno = 0;
          error("Shutting down...");
          sprintf(msg_rename0, "shutting down: %s", ShutdownReason);
        }
        else
        {
          sprintf(msg_rename0, "server exiting");
        }

        while (NumPlayers > 0)
        {
          loop_counter[4]++;
          pl_rename0 = Players[NumPlayers - 1];
          if (pl_rename0->conn == NOT_CONNECTED)
          {
            loop_counter[5]++;
            Delete_player(NumPlayers - 1);
          }
          else
          {
            Destroy_connection(pl_rename0->conn, msg_rename0);
          }

        }

        Meta_gone();
        Contact_cleanup();
        Free_players();
        Free_shots();
        Free_map();
        Free_cells();
        Free_options();
        Log_game("END");
        exit(0);
        {
          return_value = FALSE;
          goto return0;
        }
        return0:
        ;

      }
    }
    else
    {
      ShutdownServer--;
    }

  }

  Input();
  if ((NumPlayers > (NumRobots + NumPseudoPlayers)) || RawMode)
  {
    loop_counter[6]++;
    if (NoPlayersEnteredYet)
    {
      loop_counter[7]++;
      if (NumPlayers > (NumRobots + NumPseudoPlayers))
      {
        loop_counter[8]++;
        NoPlayersEnteredYet = false;
        if (gameDuration > 0.0)
        {
          loop_counter[9]++;
          xpprintf("%s Server will stop in %g minutes.\n", showtime(), gameDuration);
          gameOverTime = ((time_t) (gameDuration * 60)) + time((time_t *) NULL);
        }

      }

    }

    Update_objects();
    if ((main_loops % UPDATES_PR_FRAME) == 0)
    {
      loop_counter[10]++;
      {
        int i_rename1;
        int conn_rename1;
        int ind_rename1;
        player *pl_rename1;
        time_t newTimeLeft_rename1 = 0;
        static time_t oldTimeLeft_rename1;
        static bool game_over_called_rename1 = false;
        if ((++frame_loops) >= LONG_MAX)
        {
          loop_counter[11]++;
          frame_loops = 1;
        }

        {
          if (last_frame_shuffle != frame_loops)
          {
            loop_counter[12]++;
            last_frame_shuffle = frame_loops;
            {
              int i_rename15;
              size_t memsize_rename15;
              num_object_shuffle = MIN(NumObjs, maxVisibleObject);
              if (max_object_shuffle < num_object_shuffle)
              {
                loop_counter[13]++;
                if (object_shuffle_ptr != NULL)
                {
                  loop_counter[14]++;
                  free(object_shuffle_ptr);
                }

                max_object_shuffle = num_object_shuffle;
                memsize_rename15 = max_object_shuffle * (sizeof(shuffle_t));
                object_shuffle_ptr = (shuffle_t *) malloc(memsize_rename15);
                if (object_shuffle_ptr == NULL)
                {
                  loop_counter[15]++;
                  max_object_shuffle = 0;
                }

              }

              if (max_object_shuffle < num_object_shuffle)
              {
                loop_counter[16]++;
                num_object_shuffle = max_object_shuffle;
              }

              for (i_rename15 = 0; i_rename15 < num_object_shuffle; i_rename15++)
              {
                loop_counter[17]++;
                object_shuffle_ptr[i_rename15] = i_rename15;
              }

              for (i_rename15 = num_object_shuffle - 1; i_rename15 >= 0; --i_rename15)
              {
                loop_counter[18]++;
                if (object_shuffle_ptr[i_rename15] == i_rename15)
                {
                  loop_counter[19]++;
                  int j_rename15 = (int) (rfrac() * i_rename15);
                  shuffle_t tmp_rename15 = object_shuffle_ptr[j_rename15];
                  object_shuffle_ptr[j_rename15] = object_shuffle_ptr[i_rename15];
                  object_shuffle_ptr[i_rename15] = tmp_rename15;
                }

              }

              return15:
              ;

            }
            {
              int i_rename16;
              size_t memsize_rename16;
              num_player_shuffle = MIN(NumPlayers, 65535);
              if (max_player_shuffle < num_player_shuffle)
              {
                loop_counter[20]++;
                if (player_shuffle_ptr != NULL)
                {
                  loop_counter[21]++;
                  free(player_shuffle_ptr);
                }

                max_player_shuffle = num_player_shuffle;
                memsize_rename16 = max_player_shuffle * (sizeof(shuffle_t));
                player_shuffle_ptr = (shuffle_t *) malloc(memsize_rename16);
                if (player_shuffle_ptr == NULL)
                {
                  loop_counter[22]++;
                  max_player_shuffle = 0;
                }

              }

              if (max_player_shuffle < num_player_shuffle)
              {
                loop_counter[23]++;
                num_player_shuffle = max_player_shuffle;
              }

              for (i_rename16 = 0; i_rename16 < num_player_shuffle; i_rename16++)
              {
                loop_counter[24]++;
                player_shuffle_ptr[i_rename16] = i_rename16;
              }

              for (i_rename16 = 0; i_rename16 < num_player_shuffle; i_rename16++)
              {
                loop_counter[25]++;
                int j_rename16 = (int) (rfrac() * num_player_shuffle);
                shuffle_t tmp_rename16 = player_shuffle_ptr[j_rename16];
                player_shuffle_ptr[j_rename16] = player_shuffle_ptr[i_rename16];
                player_shuffle_ptr[i_rename16] = tmp_rename16;
              }

              return16:
              ;

            }
          }

          return4:
          ;

        }
        {
          int time_result0_rename1;
          time_result0_rename1 = time(NULL);
          if (((gameDuration > 0.0) && (game_over_called_rename1 == false)) && (oldTimeLeft_rename1 != (newTimeLeft_rename1 = gameOverTime - time_result0_rename1)))
          {
            loop_counter[26]++;
            if (newTimeLeft_rename1 <= 0)
            {
              loop_counter[27]++;
              Game_Over();
              ShutdownServer = 30 * FPS;
              game_over_called_rename1 = true;
            }

          }

        }
        for (i_rename1 = 0; i_rename1 < num_player_shuffle; i_rename1++)
        {
          loop_counter[28]++;
          pl_rename1 = Players[i_rename1];
          conn_rename1 = pl_rename1->conn;
          if (conn_rename1 == NOT_CONNECTED)
          {
            loop_counter[29]++;
            continue;
          }

          {
            int BIT_result0_rename1;
            BIT_result0_rename1 = BIT(pl_rename1->status, PAUSE | GAME_OVER);
            if ((BIT_result0_rename1 && (!allowViewing)) && (!pl_rename1->isowner))
            {
              loop_counter[30]++;
              {
                int BIT_result0_rename1;
                BIT_result0_rename1 = BIT(pl_rename1->status, PAUSE);
                if (BIT_result0_rename1)
                {
                  loop_counter[31]++;
                  if (frame_loops & 0x03)
                  {
                    loop_counter[32]++;
                    continue;
                  }

                }
                else
                {
                  if (frame_loops & 0x01)
                  {
                    loop_counter[33]++;
                    continue;
                  }

                }

              }
            }

          }
          if (pl_rename1->player_count > 0)
          {
            loop_counter[34]++;
            pl_rename1->player_round++;
            if (pl_rename1->player_round >= pl_rename1->player_count)
            {
              loop_counter[35]++;
              pl_rename1->player_round = 0;
              continue;
            }

          }

          {
            int Send_start_of_frame_result0_rename1;
            Send_start_of_frame_result0_rename1 = Send_start_of_frame(conn_rename1);
            if (Send_start_of_frame_result0_rename1 == (-1))
            {
              loop_counter[36]++;
              continue;
            }

          }
          if (newTimeLeft_rename1 != oldTimeLeft_rename1)
          {
            loop_counter[37]++;
            Send_time_left(conn_rename1, newTimeLeft_rename1);
          }
          else
            if ((maxRoundTime > 0) && (roundtime >= 0))
          {
            loop_counter[38]++;
            Send_time_left(conn_rename1, ((roundtime + FPS) - 1) / FPS);
          }


          {
            int BIT_result0_rename1;
            BIT_result0_rename1 = BIT(pl_rename1->lock.tagged, LOCK_PLAYER);
            if (BIT_result0_rename1)
            {
              loop_counter[39]++;
              {
                int BIT_result0_rename1;
                BIT_result0_rename1 = BIT(pl_rename1->status, GAME_OVER | PLAYING);
                int BIT_result1_rename1;
                BIT_result1_rename1 = BIT(pl_rename1->status, PAUSE);
                int BIT_result2_rename1;
                BIT_result2_rename1 = BIT(World.rules->mode, TEAM_PLAY);
                if ((BIT_result0_rename1 == (GAME_OVER | PLAYING)) || (BIT_result1_rename1 && ((((BIT_result2_rename1 && (pl_rename1->team != TEAM_NOT_SET)) && (pl_rename1->team == Players[GetInd[pl_rename1->lock.pl_id]]->team)) || pl_rename1->isowner) || allowViewing)))
                {
                  loop_counter[40]++;
                  ind_rename1 = GetInd[pl_rename1->lock.pl_id];
                }
                else
                {
                  ind_rename1 = i_rename1;
                }

              }
            }
            else
            {
              ind_rename1 = i_rename1;
            }

          }
          if (Players[ind_rename1]->damaged > 0)
          {
            loop_counter[41]++;
            Send_damaged(conn_rename1, Players[ind_rename1]->damaged);
          }
          else
          {
            {
              int ind_rename5 = ind_rename1;
              int conn_rename5 = conn_rename1;
              player *pl_rename5 = Players[ind_rename5];
              Get_display_parameters(conn_rename5, &view_width, &view_height, &debris_colors, &spark_rand);
              debris_x_areas = (view_width + 255) >> 8;
              debris_y_areas = (view_height + 255) >> 8;
              debris_areas = debris_x_areas * debris_y_areas;
              horizontal_blocks = (view_width + (BLOCK_SZ - 1)) / BLOCK_SZ;
              vertical_blocks = (view_height + (BLOCK_SZ - 1)) / BLOCK_SZ;
              pv.world.x = pl_rename5->pos.x - (view_width / 2);
              pv.world.y = pl_rename5->pos.y - (view_height / 2);
              pv.realWorld = pv.world;
              {
                int BIT_result0_rename5;
                BIT_result0_rename5 = BIT(World.rules->mode, WRAP_PLAY);
                if (BIT_result0_rename5)
                {
                  loop_counter[42]++;
                  if ((pv.world.x < 0) && ((pv.world.x + view_width) < World.width))
                  {
                    loop_counter[43]++;
                    pv.world.x += World.width;
                  }
                  else
                    if ((pv.world.x > 0) && ((pv.world.x + view_width) >= World.width))
                  {
                    loop_counter[44]++;
                    pv.realWorld.x -= World.width;
                  }


                  if ((pv.world.y < 0) && ((pv.world.y + view_height) < World.height))
                  {
                    loop_counter[45]++;
                    pv.world.y += World.height;
                  }
                  else
                    if ((pv.world.y > 0) && ((pv.world.y + view_height) >= World.height))
                  {
                    loop_counter[46]++;
                    pv.realWorld.y -= World.height;
                  }


                }

              }
              return5:
              ;

            }
            int Frame_status_result_rename1;
            {
              int return_value;
              int ind_rename6 = ind_rename1;
              int conn_rename6 = conn_rename1;
              static char mods_rename6[MAX_CHARS];
              player *pl_rename6 = Players[ind_rename6];
              int n_rename6;
              int lock_ind_rename6;
              int lock_id_rename6 = NO_ID;
              int lock_dist_rename6 = 0;
              int lock_dir_rename6 = 0;
              int i_rename6;
              int showautopilot_rename6;
              CLR_BIT(pl_rename6->lock.tagged, LOCK_VISIBLE);
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->lock.tagged, LOCK_PLAYER);
                int BIT_result1_rename6;
                BIT_result1_rename6 = BIT(pl_rename6->used, HAS_COMPASS);
                if (BIT_result0_rename6 && BIT_result1_rename6)
                {
                  loop_counter[47]++;
                  lock_id_rename6 = pl_rename6->lock.pl_id;
                  lock_ind_rename6 = GetInd[lock_id_rename6];
                  {
                    int BIT_result0_rename6;
                    BIT_result0_rename6 = BIT(World.rules->mode, LIMITED_VISIBILITY);
                    int OWNS_TANK_result1_rename6;
                    OWNS_TANK_result1_rename6 = OWNS_TANK(ind_rename6, lock_ind_rename6);
                    int TEAM_result2_rename6;
                    TEAM_result2_rename6 = TEAM(ind_rename6, lock_ind_rename6);
                    int ALLIANCE_result3_rename6;
                    ALLIANCE_result3_rename6 = ALLIANCE(ind_rename6, lock_ind_rename6);
                    int BIT_result4_rename6;
                    BIT_result4_rename6 = BIT(Players[lock_ind_rename6]->status, PLAYING | GAME_OVER);
                    if ((((((!BIT_result0_rename6) || (pl_rename6->lock.distance <= pl_rename6->sensor_range)) && (((pl_rename6->visibility[lock_ind_rename6].canSee || OWNS_TANK_result1_rename6) || TEAM_result2_rename6) || ALLIANCE_result3_rename6)) && (BIT_result4_rename6 == PLAYING)) && (playersOnRadar || ((((Players[lock_ind_rename6]->pos.x > pv.world.x) && (Players[lock_ind_rename6]->pos.x < (pv.world.x + view_width))) || ((Players[lock_ind_rename6]->pos.x > pv.realWorld.x) && (Players[lock_ind_rename6]->pos.x < (pv.realWorld.x + view_width)))) && (((Players[lock_ind_rename6]->pos.y > pv.world.y) && (Players[lock_ind_rename6]->pos.y < (pv.world.y + view_height))) || ((Players[lock_ind_rename6]->pos.y > pv.realWorld.y) && (Players[lock_ind_rename6]->pos.y < (pv.realWorld.y + view_height))))))) && (pl_rename6->lock.distance != 0))
                    {
                      loop_counter[48]++;
                      SET_BIT(pl_rename6->lock.tagged, LOCK_VISIBLE);
                      lock_dir_rename6 = (int) Wrap_findDir((int) (Players[lock_ind_rename6]->pos.x - pl_rename6->pos.x), (int) (Players[lock_ind_rename6]->pos.y - pl_rename6->pos.y));
                      lock_dist_rename6 = (int) pl_rename6->lock.distance;
                    }

                  }
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->status, HOVERPAUSE);
                if (BIT_result0_rename6)
                {
                  loop_counter[49]++;
                  showautopilot_rename6 = (pl_rename6->count <= 0) || ((frame_loops % 8) < 4);
                }
                else
                {
                  int BIT_result0_rename6;
                  BIT_result0_rename6 = BIT(pl_rename6->used, HAS_AUTOPILOT);
                  if (BIT_result0_rename6)
                  {
                    loop_counter[50]++;
                    showautopilot_rename6 = (frame_loops % 8) < 4;
                  }
                  else
                    showautopilot_rename6 = 0;

                }

              }
              i_rename6 = 0;
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->mods.nuclear, FULLNUCLEAR);
                if (BIT_result0_rename6)
                {
                  loop_counter[51]++;
                  mods_rename6[i_rename6++] = 'F';
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->mods.nuclear, NUCLEAR);
                if (BIT_result0_rename6)
                {
                  loop_counter[52]++;
                  mods_rename6[i_rename6++] = 'N';
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->mods.warhead, CLUSTER);
                if (BIT_result0_rename6)
                {
                  loop_counter[53]++;
                  mods_rename6[i_rename6++] = 'C';
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->mods.warhead, IMPLOSION);
                if (BIT_result0_rename6)
                {
                  loop_counter[54]++;
                  mods_rename6[i_rename6++] = 'I';
                }

              }
              if (pl_rename6->mods.velocity)
              {
                loop_counter[55]++;
                if (i_rename6)
                {
                  loop_counter[56]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'V';
                {
                  int return_value;
                  int i_rename17 = i_rename6;
                  int num_rename17 = pl_rename6->mods.velocity;
                  int digits_rename17;
                  int t_rename17;
                  if (num_rename17 < 0)
                  {
                    loop_counter[57]++;
                    mods_rename6[i_rename17++] = '-';
                    num_rename17 = -num_rename17;
                  }

                  if (num_rename17 < 10)
                  {
                    loop_counter[58]++;
                    mods_rename6[i_rename17++] = '0' + num_rename17;
                    {
                      return_value = i_rename17;
                      goto return17;
                    }
                  }

                  for (t_rename17 = num_rename17, digits_rename17 = 0; t_rename17; t_rename17 /= 10, digits_rename17++)
                    loop_counter[59]++;

                  for (t_rename17 = (i_rename17 + digits_rename17) - 1; t_rename17 >= 0; t_rename17--)
                  {
                    loop_counter[60]++;
                    mods_rename6[t_rename17] = num_rename17 % 10;
                    num_rename17 /= 10;
                  }

                  {
                    return_value = i_rename17 + digits_rename17;
                    goto return17;
                  }
                  return17:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.mini)
              {
                loop_counter[61]++;
                if (i_rename6)
                {
                  loop_counter[62]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'X';
                {
                  int return_value;
                  int i_rename18 = i_rename6;
                  int num_rename18 = pl_rename6->mods.mini + 1;
                  int digits_rename18;
                  int t_rename18;
                  if (num_rename18 < 0)
                  {
                    loop_counter[63]++;
                    mods_rename6[i_rename18++] = '-';
                    num_rename18 = -num_rename18;
                  }

                  if (num_rename18 < 10)
                  {
                    loop_counter[64]++;
                    mods_rename6[i_rename18++] = '0' + num_rename18;
                    {
                      return_value = i_rename18;
                      goto return18;
                    }
                  }

                  for (t_rename18 = num_rename18, digits_rename18 = 0; t_rename18; t_rename18 /= 10, digits_rename18++)
                    loop_counter[65]++;

                  for (t_rename18 = (i_rename18 + digits_rename18) - 1; t_rename18 >= 0; t_rename18--)
                  {
                    loop_counter[66]++;
                    mods_rename6[t_rename18] = num_rename18 % 10;
                    num_rename18 /= 10;
                  }

                  {
                    return_value = i_rename18 + digits_rename18;
                    goto return18;
                  }
                  return18:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.spread)
              {
                loop_counter[67]++;
                if (i_rename6)
                {
                  loop_counter[68]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'Z';
                {
                  int return_value;
                  int i_rename19 = i_rename6;
                  int num_rename19 = pl_rename6->mods.spread;
                  int digits_rename19;
                  int t_rename19;
                  if (num_rename19 < 0)
                  {
                    loop_counter[69]++;
                    mods_rename6[i_rename19++] = '-';
                    num_rename19 = -num_rename19;
                  }

                  if (num_rename19 < 10)
                  {
                    loop_counter[70]++;
                    mods_rename6[i_rename19++] = '0' + num_rename19;
                    {
                      return_value = i_rename19;
                      goto return19;
                    }
                  }

                  for (t_rename19 = num_rename19, digits_rename19 = 0; t_rename19; t_rename19 /= 10, digits_rename19++)
                    loop_counter[71]++;

                  for (t_rename19 = (i_rename19 + digits_rename19) - 1; t_rename19 >= 0; t_rename19--)
                  {
                    loop_counter[72]++;
                    mods_rename6[t_rename19] = num_rename19 % 10;
                    num_rename19 /= 10;
                  }

                  {
                    return_value = i_rename19 + digits_rename19;
                    goto return19;
                  }
                  return19:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.power)
              {
                loop_counter[73]++;
                if (i_rename6)
                {
                  loop_counter[74]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'B';
                {
                  int return_value;
                  int i_rename20 = i_rename6;
                  int num_rename20 = pl_rename6->mods.power;
                  int digits_rename20;
                  int t_rename20;
                  if (num_rename20 < 0)
                  {
                    loop_counter[75]++;
                    mods_rename6[i_rename20++] = '-';
                    num_rename20 = -num_rename20;
                  }

                  if (num_rename20 < 10)
                  {
                    loop_counter[76]++;
                    mods_rename6[i_rename20++] = '0' + num_rename20;
                    {
                      return_value = i_rename20;
                      goto return20;
                    }
                  }

                  for (t_rename20 = num_rename20, digits_rename20 = 0; t_rename20; t_rename20 /= 10, digits_rename20++)
                    loop_counter[77]++;

                  for (t_rename20 = (i_rename20 + digits_rename20) - 1; t_rename20 >= 0; t_rename20--)
                  {
                    loop_counter[78]++;
                    mods_rename6[t_rename20] = num_rename20 % 10;
                    num_rename20 /= 10;
                  }

                  {
                    return_value = i_rename20 + digits_rename20;
                    goto return20;
                  }
                  return20:
                  ;

                  i_rename6 = return_value;
                }
              }

              if (pl_rename6->mods.laser)
              {
                loop_counter[79]++;
                if (i_rename6)
                {
                  loop_counter[80]++;
                  mods_rename6[i_rename6++] = ' ';
                }

                mods_rename6[i_rename6++] = 'L';
                mods_rename6[i_rename6++] = BIT(pl_rename6->mods.laser, STUN) ? 'S' : 'B';
              }

              mods_rename6[i_rename6] = '\0';
              n_rename6 = Send_self(conn_rename6, pl_rename6, lock_id_rename6, lock_dist_rename6, lock_dir_rename6, showautopilot_rename6, Players[GetInd[Get_player_id(conn_rename6)]]->status, mods_rename6);
              if (n_rename6 <= 0)
              {
                loop_counter[81]++;
                {
                  return_value = 0;
                  goto return6;
                }
              }

              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->used, HAS_EMERGENCY_THRUST);
                if (BIT_result0_rename6)
                {
                  loop_counter[82]++;
                  Send_thrusttime(conn_rename6, pl_rename6->emergency_thrust_left, pl_rename6->emergency_thrust_max);
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->used, HAS_EMERGENCY_SHIELD);
                if (BIT_result0_rename6)
                {
                  loop_counter[83]++;
                  Send_shieldtime(conn_rename6, pl_rename6->emergency_shield_left, pl_rename6->emergency_shield_max);
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->status, SELF_DESTRUCT);
                if (BIT_result0_rename6 && (pl_rename6->count > 0))
                {
                  loop_counter[84]++;
                  Send_destruct(conn_rename6, pl_rename6->count);
                }

              }
              {
                int BIT_result0_rename6;
                BIT_result0_rename6 = BIT(pl_rename6->used, HAS_PHASING_DEVICE);
                if (BIT_result0_rename6)
                {
                  loop_counter[85]++;
                  Send_phasingtime(conn_rename6, pl_rename6->phasing_left, pl_rename6->phasing_max);
                }

              }
              if (ShutdownServer != (-1))
              {
                loop_counter[86]++;
                Send_shutdown(conn_rename6, ShutdownServer, ShutdownDelay);
              }

              if (round_delay_send > 0)
              {
                loop_counter[87]++;
                Send_rounddelay(conn_rename6, round_delay, roundDelaySeconds * FPS);
              }

              {
                return_value = 1;
                goto return6;
              }
              return6:
              ;

              Frame_status_result_rename1 = return_value;
            }
            if (Frame_status_result_rename1 <= 0)
            {
              loop_counter[88]++;
              continue;
            }

            {
              int ind_rename7 = ind_rename1;
              int conn_rename7 = conn_rename1;
              player *pl_rename7 = Players[ind_rename7];
              int i_rename7;
              int k_rename7;
              int x_rename7;
              int y_rename7;
              int conn_bit_rename7 = 1 << conn_rename7;
              block_visibility_t bv_rename7;
              const int fuel_packet_size_rename7 = 5;
              const int cannon_packet_size_rename7 = 5;
              const int target_packet_size_rename7 = 7;
              const int wormhole_packet_size_rename7 = 5;
              int bytes_left_rename7 = 2000;
              int max_packet_rename7;
              int packet_count_rename7;
              x_rename7 = pl_rename7->pos.bx;
              y_rename7 = pl_rename7->pos.by;
              bv_rename7.world.x = x_rename7 - (horizontal_blocks >> 1);
              bv_rename7.world.y = y_rename7 - (vertical_blocks >> 1);
              bv_rename7.realWorld = bv_rename7.world;
              {
                int BIT_result0_rename7;
                BIT_result0_rename7 = BIT(World.rules->mode, WRAP_PLAY);
                if (BIT_result0_rename7)
                {
                  loop_counter[89]++;
                  if ((bv_rename7.world.x < 0) && ((bv_rename7.world.x + horizontal_blocks) < World.x))
                  {
                    loop_counter[90]++;
                    bv_rename7.world.x += World.x;
                  }
                  else
                    if ((bv_rename7.world.x > 0) && ((bv_rename7.world.x + horizontal_blocks) > World.x))
                  {
                    loop_counter[91]++;
                    bv_rename7.realWorld.x -= World.x;
                  }


                  if ((bv_rename7.world.y < 0) && ((bv_rename7.world.y + vertical_blocks) < World.y))
                  {
                    loop_counter[92]++;
                    bv_rename7.world.y += World.y;
                  }
                  else
                    if ((bv_rename7.world.y > 0) && ((bv_rename7.world.y + vertical_blocks) > World.y))
                  {
                    loop_counter[93]++;
                    bv_rename7.realWorld.y -= World.y;
                  }


                }

              }
              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / target_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_target_update);
              for (k_rename7 = 0; k_rename7 < World.NumTargets; k_rename7++)
              {
                loop_counter[94]++;
                target_t * targ;
                if ((++i_rename7) >= World.NumTargets)
                {
                  loop_counter[95]++;
                  i_rename7 = 0;
                }

                targ = &World.targets[i_rename7];
                {
                  int BIT_result0_rename7;
                  BIT_result0_rename7 = BIT(targ->update_mask, conn_bit_rename7);
                  int BIT_result1_rename7;
                  BIT_result1_rename7 = BIT(targ->conn_mask, conn_bit_rename7);
                  int block_inview_result2_rename7;
                  block_inview_result2_rename7 = block_inview(&bv_rename7, targ->pos.x, targ->pos.y);
                  if (BIT_result0_rename7 || ((BIT_result1_rename7 == 0) && block_inview_result2_rename7))
                  {
                    loop_counter[96]++;
                    Send_target(conn_rename7, i_rename7, targ->dead_time, targ->damage);
                    pl_rename7->last_target_update = i_rename7;
                    bytes_left_rename7 -= target_packet_size_rename7;
                    if ((++packet_count_rename7) >= max_packet_rename7)
                    {
                      loop_counter[97]++;
                      break;
                    }

                  }

                }
              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / cannon_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_cannon_update);
              for (k_rename7 = 0; k_rename7 < World.NumCannons; k_rename7++)
              {
                loop_counter[98]++;
                if ((++i_rename7) >= World.NumCannons)
                {
                  loop_counter[99]++;
                  i_rename7 = 0;
                }

                {
                  int block_inview_result0_rename7;
                  block_inview_result0_rename7 = block_inview(&bv_rename7, World.cannon[i_rename7].blk_pos.x, World.cannon[i_rename7].blk_pos.y);
                  if (block_inview_result0_rename7)
                  {
                    loop_counter[100]++;
                    {
                      int BIT_result0_rename7;
                      BIT_result0_rename7 = BIT(World.cannon[i_rename7].conn_mask, conn_bit_rename7);
                      if (BIT_result0_rename7 == 0)
                      {
                        loop_counter[101]++;
                        Send_cannon(conn_rename7, i_rename7, World.cannon[i_rename7].dead_time);
                        pl_rename7->last_cannon_update = i_rename7;
                        bytes_left_rename7 -= max_packet_rename7 * cannon_packet_size_rename7;
                        if ((++packet_count_rename7) >= max_packet_rename7)
                        {
                          loop_counter[102]++;
                          break;
                        }

                      }

                    }
                  }

                }
              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / fuel_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_fuel_update);
              for (k_rename7 = 0; k_rename7 < World.NumFuels; k_rename7++)
              {
                loop_counter[103]++;
                if ((++i_rename7) >= World.NumFuels)
                {
                  loop_counter[104]++;
                  i_rename7 = 0;
                }

                {
                  int BIT_result0_rename7;
                  BIT_result0_rename7 = BIT(World.fuel[i_rename7].conn_mask, conn_bit_rename7);
                  if (BIT_result0_rename7 == 0)
                  {
                    loop_counter[105]++;
                    if (World.block[World.fuel[i_rename7].blk_pos.x][World.fuel[i_rename7].blk_pos.y] == FUEL)
                    {
                      loop_counter[106]++;
                      {
                        int block_inview_result0_rename7;
                        block_inview_result0_rename7 = block_inview(&bv_rename7, World.fuel[i_rename7].blk_pos.x, World.fuel[i_rename7].blk_pos.y);
                        if (block_inview_result0_rename7)
                        {
                          loop_counter[107]++;
                          Send_fuel(conn_rename7, i_rename7, (int) World.fuel[i_rename7].fuel);
                          pl_rename7->last_fuel_update = i_rename7;
                          bytes_left_rename7 -= max_packet_rename7 * fuel_packet_size_rename7;
                          if ((++packet_count_rename7) >= max_packet_rename7)
                          {
                            loop_counter[108]++;
                            break;
                          }

                        }

                      }
                    }

                  }

                }
              }

              packet_count_rename7 = 0;
              max_packet_rename7 = MAX(5, bytes_left_rename7 / wormhole_packet_size_rename7);
              i_rename7 = MAX(0, pl_rename7->last_wormhole_update);
              for (k_rename7 = 0; k_rename7 < World.NumWormholes; k_rename7++)
              {
                loop_counter[109]++;
                wormhole_t * worm;
                if ((++i_rename7) >= World.NumWormholes)
                {
                  loop_counter[110]++;
                  i_rename7 = 0;
                }

                worm = &World.wormHoles[i_rename7];
                {
                  int block_inview_result0_rename7;
                  block_inview_result0_rename7 = block_inview(&bv_rename7, worm->pos.x, worm->pos.y);
                  if (((wormholeVisible && worm->temporary) && ((worm->type == WORM_IN) || (worm->type == WORM_NORMAL))) && block_inview_result0_rename7)
                  {
                    loop_counter[111]++;
                    int x_rename7 = (worm->pos.x * BLOCK_SZ) + (BLOCK_SZ / 2);
                    int y_rename7 = (worm->pos.y * BLOCK_SZ) + (BLOCK_SZ / 2);
                    Send_wormhole(conn_rename7, x_rename7, y_rename7);
                    pl_rename7->last_wormhole_update = i_rename7;
                    bytes_left_rename7 -= max_packet_rename7 * wormhole_packet_size_rename7;
                    if ((++packet_count_rename7) >= max_packet_rename7)
                    {
                      loop_counter[112]++;
                      break;
                    }

                  }

                }
              }

              return7:
              ;

            }
            {
              int ind_rename8 = ind_rename1;
              int conn_rename8 = conn_rename1;
              player *pl_rename8 = Players[ind_rename8];
              player *pl_i_rename8;
              pulse_t * pulse;
              int i_rename8;
              int j_rename8;
              int k_rename8;
              int color_rename8;
              int dir_rename8;
              DFLOAT x_rename8;
              DFLOAT y_rename8;
              for (j_rename8 = 0; j_rename8 < NumPulses; j_rename8++)
              {
                loop_counter[113]++;
                pulse = Pulses[j_rename8];
                if (pulse->len <= 0)
                {
                  loop_counter[114]++;
                  continue;
                }

                x_rename8 = pulse->pos.x;
                y_rename8 = pulse->pos.y;
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(World.rules->mode, WRAP_PLAY);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[115]++;
                    if (x_rename8 < 0)
                    {
                      loop_counter[116]++;
                      x_rename8 += World.width;
                    }
                    else
                      if (x_rename8 >= World.width)
                    {
                      loop_counter[117]++;
                      x_rename8 -= World.width;
                    }


                    if (y_rename8 < 0)
                    {
                      loop_counter[118]++;
                      y_rename8 += World.height;
                    }
                    else
                      if (y_rename8 >= World.height)
                    {
                      loop_counter[119]++;
                      y_rename8 -= World.height;
                    }


                  }

                }
                if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                {
                  loop_counter[120]++;
                  dir_rename8 = pulse->dir;
                }
                else
                {
                  x_rename8 += tcos(pulse->dir) * pulse->len;
                  y_rename8 += tsin(pulse->dir) * pulse->len;
                  {
                    int BIT_result0_rename8;
                    BIT_result0_rename8 = BIT(World.rules->mode, WRAP_PLAY);
                    if (BIT_result0_rename8)
                    {
                      loop_counter[121]++;
                      if (x_rename8 < 0)
                      {
                        loop_counter[122]++;
                        x_rename8 += World.width;
                      }
                      else
                        if (x_rename8 >= World.width)
                      {
                        loop_counter[123]++;
                        x_rename8 -= World.width;
                      }


                      if (y_rename8 < 0)
                      {
                        loop_counter[124]++;
                        y_rename8 += World.height;
                      }
                      else
                        if (y_rename8 >= World.height)
                      {
                        loop_counter[125]++;
                        y_rename8 -= World.height;
                      }


                    }

                  }
                  if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[126]++;
                    dir_rename8 = MOD2(pulse->dir + (RES / 2), RES);
                  }
                  else
                  {
                    continue;
                  }

                }

                {
                  int Team_immune_result0_rename8;
                  Team_immune_result0_rename8 = Team_immune(pulse->id, pl_rename8->id);
                  if (Team_immune_result0_rename8)
                  {
                    loop_counter[127]++;
                    color_rename8 = BLUE;
                  }
                  else
                    if ((pulse->id == pl_rename8->id) && selfImmunity)
                  {
                    loop_counter[128]++;
                    color_rename8 = BLUE;
                  }
                  else
                  {
                    color_rename8 = RED;
                  }


                }
                Send_laser(conn_rename8, color_rename8, (int) x_rename8, (int) y_rename8, pulse->len, dir_rename8);
              }

              for (i_rename8 = 0; i_rename8 < NumEcms; i_rename8++)
              {
                loop_counter[129]++;
                ecm_t *ecm_rename8 = Ecms[i_rename8];
                Send_ecm(conn_rename8, (int) ecm_rename8->pos.x, (int) ecm_rename8->pos.y, ecm_rename8->size);
              }

              for (i_rename8 = 0; i_rename8 < NumTransporters; i_rename8++)
              {
                loop_counter[130]++;
                trans_t *trans_rename8 = Transporters[i_rename8];
                player *victim_rename8 = Players[GetInd[trans_rename8->target]];
                player *pl_rename8 = trans_rename8->id == NO_ID ? NULL : Players[GetInd[trans_rename8->id]];
                DFLOAT x_rename8 = pl_rename8 ? pl_rename8->pos.x : trans_rename8->pos.x;
                DFLOAT y_rename8 = pl_rename8 ? pl_rename8->pos.y : trans_rename8->pos.y;
                Send_trans(conn_rename8, victim_rename8->pos.x, victim_rename8->pos.y, (int) x_rename8, (int) y_rename8);
              }

              for (i_rename8 = 0; i_rename8 < World.NumCannons; i_rename8++)
              {
                loop_counter[131]++;
                cannon_t *cannon_rename8 = World.cannon + i_rename8;
                if (cannon_rename8->tractor_count > 0)
                {
                  loop_counter[132]++;
                  player *t_rename8 = Players[GetInd[cannon_rename8->tractor_target]];
                  if ((((t_rename8->pos.x > pv.world.x) && (t_rename8->pos.x < (pv.world.x + view_width))) || ((t_rename8->pos.x > pv.realWorld.x) && (t_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((t_rename8->pos.y > pv.world.y) && (t_rename8->pos.y < (pv.world.y + view_height))) || ((t_rename8->pos.y > pv.realWorld.y) && (t_rename8->pos.y < (pv.realWorld.y + view_height)))))
                  {
                    loop_counter[133]++;
                    int j_rename8;
                    for (j_rename8 = 0; j_rename8 < 3; j_rename8++)
                    {
                      loop_counter[134]++;
                      Send_connector(conn_rename8, (int) (t_rename8->pos.x + t_rename8->ship->pts[j_rename8][t_rename8->dir].x), (int) (t_rename8->pos.y + t_rename8->ship->pts[j_rename8][t_rename8->dir].y), (int) cannon_rename8->pix_pos.x, (int) cannon_rename8->pix_pos.y, 1);
                    }

                  }

                }

              }

              for (k_rename8 = 0; k_rename8 < num_player_shuffle; k_rename8++)
              {
                loop_counter[135]++;
                i_rename8 = player_shuffle_ptr[k_rename8];
                pl_i_rename8 = Players[i_rename8];
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->status, PLAYING | PAUSE);
                  if (!BIT_result0_rename8)
                  {
                    loop_counter[136]++;
                    continue;
                  }

                }
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->status, GAME_OVER);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[137]++;
                    continue;
                  }

                }
                if (!((((pl_i_rename8->pos.x > pv.world.x) && (pl_i_rename8->pos.x < (pv.world.x + view_width))) || ((pl_i_rename8->pos.x > pv.realWorld.x) && (pl_i_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((pl_i_rename8->pos.y > pv.world.y) && (pl_i_rename8->pos.y < (pv.world.y + view_height))) || ((pl_i_rename8->pos.y > pv.realWorld.y) && (pl_i_rename8->pos.y < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[138]++;
                  continue;
                }

                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->status, PAUSE);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[139]++;
                    Send_paused(conn_rename8, pl_i_rename8->pos.x, pl_i_rename8->pos.y, pl_i_rename8->count);
                    continue;
                  }

                }
                {
                  int TEAM_result0_rename8;
                  TEAM_result0_rename8 = TEAM(i_rename8, ind_rename8);
                  int ALLIANCE_result1_rename8;
                  ALLIANCE_result1_rename8 = ALLIANCE(i_rename8, ind_rename8);
                  if (((pl_rename8->visibility[i_rename8].canSee || (i_rename8 == ind_rename8)) || TEAM_result0_rename8) || ALLIANCE_result1_rename8)
                  {
                    loop_counter[140]++;
                    Send_ship(conn_rename8, pl_i_rename8->pos.x, pl_i_rename8->pos.y, pl_i_rename8->id, pl_i_rename8->dir, BIT(pl_i_rename8->used, HAS_SHIELD) != 0, BIT(pl_i_rename8->used, HAS_CLOAKING_DEVICE) != 0, BIT(pl_i_rename8->used, HAS_EMERGENCY_SHIELD) != 0, BIT(pl_i_rename8->used, HAS_PHASING_DEVICE) != 0, BIT(pl_i_rename8->used, HAS_DEFLECTOR) != 0);
                  }

                }
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->used, HAS_REFUEL);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[141]++;
                    if ((((World.fuel[pl_i_rename8->fs].pix_pos.x > pv.world.x) && (World.fuel[pl_i_rename8->fs].pix_pos.x < (pv.world.x + view_width))) || ((World.fuel[pl_i_rename8->fs].pix_pos.x > pv.realWorld.x) && (World.fuel[pl_i_rename8->fs].pix_pos.x < (pv.realWorld.x + view_width)))) && (((World.fuel[pl_i_rename8->fs].pix_pos.y > pv.world.y) && (World.fuel[pl_i_rename8->fs].pix_pos.y < (pv.world.y + view_height))) || ((World.fuel[pl_i_rename8->fs].pix_pos.y > pv.realWorld.y) && (World.fuel[pl_i_rename8->fs].pix_pos.y < (pv.realWorld.y + view_height)))))
                    {
                      loop_counter[142]++;
                      Send_refuel(conn_rename8, (int) World.fuel[pl_i_rename8->fs].pix_pos.x, (int) World.fuel[pl_i_rename8->fs].pix_pos.y, pl_i_rename8->pos.x, pl_i_rename8->pos.y);
                    }

                  }

                }
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->used, HAS_REPAIR);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[143]++;
                    DFLOAT x_rename8 = ((DFLOAT) (World.targets[pl_i_rename8->repair_target].pos.x + 0.5)) * BLOCK_SZ;
                    DFLOAT y_rename8 = ((DFLOAT) (World.targets[pl_i_rename8->repair_target].pos.y + 0.5)) * BLOCK_SZ;
                    if ((((x_rename8 > pv.world.x) && (x_rename8 < (pv.world.x + view_width))) || ((x_rename8 > pv.realWorld.x) && (x_rename8 < (pv.realWorld.x + view_width)))) && (((y_rename8 > pv.world.y) && (y_rename8 < (pv.world.y + view_height))) || ((y_rename8 > pv.realWorld.y) && (y_rename8 < (pv.realWorld.y + view_height)))))
                    {
                      loop_counter[144]++;
                      Send_refuel(conn_rename8, pl_i_rename8->pos.x, pl_i_rename8->pos.y, (int) x_rename8, (int) y_rename8);
                    }

                  }

                }
                {
                  int BIT_result0_rename8;
                  BIT_result0_rename8 = BIT(pl_i_rename8->used, HAS_TRACTOR_BEAM);
                  if (BIT_result0_rename8)
                  {
                    loop_counter[145]++;
                    player *t_rename8 = Players[GetInd[pl_i_rename8->lock.pl_id]];
                    if ((((t_rename8->pos.x > pv.world.x) && (t_rename8->pos.x < (pv.world.x + view_width))) || ((t_rename8->pos.x > pv.realWorld.x) && (t_rename8->pos.x < (pv.realWorld.x + view_width)))) && (((t_rename8->pos.y > pv.world.y) && (t_rename8->pos.y < (pv.world.y + view_height))) || ((t_rename8->pos.y > pv.realWorld.y) && (t_rename8->pos.y < (pv.realWorld.y + view_height)))))
                    {
                      loop_counter[146]++;
                      int j_rename8;
                      for (j_rename8 = 0; j_rename8 < 3; j_rename8++)
                      {
                        loop_counter[147]++;
                        Send_connector(conn_rename8, (int) (t_rename8->pos.x + t_rename8->ship->pts[j_rename8][t_rename8->dir].x), (int) (t_rename8->pos.y + t_rename8->ship->pts[j_rename8][t_rename8->dir].y), pl_i_rename8->pos.x, pl_i_rename8->pos.y, 1);
                      }

                    }

                  }

                }
                if ((pl_i_rename8->ball != NULL) && ((((pl_i_rename8->ball->pos.x > pv.world.x) && (pl_i_rename8->ball->pos.x < (pv.world.x + view_width))) || ((pl_i_rename8->ball->pos.x > pv.realWorld.x) && (pl_i_rename8->ball->pos.x < (pv.realWorld.x + view_width)))) && (((pl_i_rename8->ball->pos.y > pv.world.y) && (pl_i_rename8->ball->pos.y < (pv.world.y + view_height))) || ((pl_i_rename8->ball->pos.y > pv.realWorld.y) && (pl_i_rename8->ball->pos.y < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[148]++;
                  Send_connector(conn_rename8, pl_i_rename8->ball->pos.x, pl_i_rename8->ball->pos.y, pl_i_rename8->pos.x, pl_i_rename8->pos.y, 0);
                }

              }

              return8:
              ;

            }
            {
              int ind_rename9 = ind_rename1;
              int conn_rename9 = conn_rename1;
              player *pl_rename9 = Players[ind_rename9];
              register int x_rename9;
              register int y_rename9;
              int i_rename9;
              int k_rename9;
              int color_rename9;
              int fuzz_rename9 = 0;
              int teamshot_rename9;
              int len_rename9;
              int obj_count_rename9;
              object * shot;
              object * (*obj_list);
              int hori_blocks_rename9;
              int vert_blocks_rename9;
              hori_blocks_rename9 = (view_width + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
              vert_blocks_rename9 = (view_height + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
              Cell_get_objects(OBJ_X_IN_BLOCKS(pl_rename9), OBJ_Y_IN_BLOCKS(pl_rename9), MAX(hori_blocks_rename9, vert_blocks_rename9), num_object_shuffle, &obj_list, &obj_count_rename9);
              for (k_rename9 = 0; k_rename9 < num_object_shuffle; k_rename9++)
              {
                loop_counter[149]++;
                i_rename9 = object_shuffle_ptr[k_rename9];
                if (i_rename9 >= obj_count_rename9)
                {
                  loop_counter[150]++;
                  continue;
                }

                shot = obj_list[i_rename9];
                x_rename9 = shot->pos.x;
                y_rename9 = shot->pos.y;
                if (!((((x_rename9 > pv.world.x) && (x_rename9 < (pv.world.x + view_width))) || ((x_rename9 > pv.realWorld.x) && (x_rename9 < (pv.realWorld.x + view_width)))) && (((y_rename9 > pv.world.y) && (y_rename9 < (pv.world.y + view_height))) || ((y_rename9 > pv.realWorld.y) && (y_rename9 < (pv.realWorld.y + view_height))))))
                {
                  loop_counter[151]++;
                  continue;
                }

                if ((color_rename9 = shot->color) == BLACK)
                {
                  loop_counter[152]++;
                  xpprintf("black %d,%d\n", shot->type, shot->id);
                  color_rename9 = WHITE;
                }

                switch (shot->type)
                {
                  case OBJ_SPARK:

                  case OBJ_DEBRIS:
                    if ((fuzz_rename9 >>= 7) < 0x40)
                  {
                    loop_counter[153]++;
                    fuzz_rename9 = randomMT();
                  }

                    if ((fuzz_rename9 & 0x7F) >= spark_rand)
                  {
                    loop_counter[154]++;
                    break;
                  }

                    if (debris_colors >= 3)
                  {
                    loop_counter[155]++;
                    if (debris_colors > 4)
                    {
                      loop_counter[156]++;
                      if (color_rename9 == BLUE)
                      {
                        loop_counter[157]++;
                        color_rename9 = shot->life >> 1;
                      }
                      else
                      {
                        color_rename9 = shot->life >> 2;
                      }

                    }
                    else
                    {
                      if (color_rename9 == BLUE)
                      {
                        loop_counter[158]++;
                        color_rename9 = shot->life >> 2;
                      }
                      else
                      {
                        color_rename9 = shot->life >> 3;
                      }

                    }

                    if (color_rename9 >= debris_colors)
                    {
                      loop_counter[159]++;
                      color_rename9 = debris_colors - 1;
                    }

                  }

                  {
                    int color_rename21 = color_rename9;
                    int yf_rename21 = (int) (shot->pos.y - pv.world.y);
                    int xf_rename21 = (int) (shot->pos.x - pv.world.x);
                    int i_rename21;
                    if (xf_rename21 < 0)
                    {
                      loop_counter[160]++;
                      xf_rename21 += World.width;
                    }

                    if (yf_rename21 < 0)
                    {
                      loop_counter[161]++;
                      yf_rename21 += World.height;
                    }

                    if ((((unsigned) xf_rename21) >= ((unsigned) view_width)) || (((unsigned) yf_rename21) >= ((unsigned) view_height)))
                    {
                      loop_counter[162]++;
                      {
                        goto return21;
                      }
                    }

                    i_rename21 = ((0 + (color_rename21 * debris_areas)) + (((yf_rename21 >> 8) % debris_y_areas) * debris_x_areas)) + ((xf_rename21 >> 8) % debris_x_areas);
                    if (debris_num[i_rename21] >= 255)
                    {
                      loop_counter[163]++;
                      {
                        goto return21;
                      }
                    }

                    if (debris_num[i_rename21] >= debris_max[i_rename21])
                    {
                      loop_counter[164]++;
                      if (debris_num[i_rename21] == 0)
                      {
                        loop_counter[165]++;
                        debris_ptr[i_rename21] = (debris_t *) malloc((debris_max[i_rename21] = 16) * (sizeof(*debris_ptr[i_rename21])));
                      }
                      else
                      {
                        debris_ptr[i_rename21] = (debris_t *) realloc(debris_ptr[i_rename21], (debris_max[i_rename21] += debris_max[i_rename21]) * (sizeof(*debris_ptr[i_rename21])));
                      }

                      if (debris_ptr[i_rename21] == 0)
                      {
                        loop_counter[166]++;
                        error("No memory for debris");
                        debris_num[i_rename21] = 0;
                        {
                          goto return21;
                        }
                      }

                    }

                    debris_ptr[i_rename21][debris_num[i_rename21]].x = (unsigned char) xf_rename21;
                    debris_ptr[i_rename21][debris_num[i_rename21]].y = (unsigned char) yf_rename21;
                    debris_num[i_rename21]++;
                    ;
                    return21:
                    ;

                  }
                    break;

                  case OBJ_WRECKAGE:
                    if ((spark_rand != 0) || wreckageCollisionMayKill)
                  {
                    loop_counter[167]++;
                    wireobject *wreck_rename9 = WIRE_PTR(shot);
                    Send_wreckage(conn_rename9, x_rename9, y_rename9, (u_byte) wreck_rename9->info, wreck_rename9->size, wreck_rename9->rotation);
                  }

                    break;

                  case OBJ_ASTEROID:
                  {
                    wireobject *ast_rename9 = WIRE_PTR(shot);
                    Send_asteroid(conn_rename9, x_rename9, y_rename9, (u_byte) ast_rename9->info, ast_rename9->size, ast_rename9->rotation);
                  }
                    break;

                  case OBJ_SHOT:

                  case OBJ_CANNON_SHOT:
                  {
                    int Team_immune_result0_rename9;
                    Team_immune_result0_rename9 = Team_immune(shot->id, pl_rename9->id);
                    int BIT_result1_rename9;
                    BIT_result1_rename9 = BIT(Players[GetInd[shot->id]]->status, PAUSE);
                    int BIT_result2_rename9;
                    BIT_result2_rename9 = BIT(World.rules->mode, TEAM_PLAY);
                    if ((Team_immune_result0_rename9 || ((shot->id != NO_ID) && BIT_result1_rename9)) || (((shot->id == NO_ID) && BIT_result2_rename9) && (shot->team == pl_rename9->team)))
                    {
                      loop_counter[168]++;
                      color_rename9 = BLUE;
                      teamshot_rename9 = DEBRIS_TYPES;
                    }
                    else
                      if ((shot->id == pl_rename9->id) && selfImmunity)
                    {
                      loop_counter[169]++;
                      color_rename9 = BLUE;
                      teamshot_rename9 = DEBRIS_TYPES;
                    }
                    else
                      if (shot->mods.nuclear && (frame_loops & 2))
                    {
                      loop_counter[170]++;
                      color_rename9 = RED;
                      teamshot_rename9 = DEBRIS_TYPES;
                    }
                    else
                    {
                      teamshot_rename9 = 0;
                    }



                  }
                  {
                    int offset_rename22 = teamshot_rename9;
                    int color_rename22 = color_rename9;
                    int yf_rename22 = (int) (shot->pos.y - pv.world.y);
                    int xf_rename22 = (int) (shot->pos.x - pv.world.x);
                    int i_rename22;
                    if (xf_rename22 < 0)
                    {
                      loop_counter[171]++;
                      xf_rename22 += World.width;
                    }

                    if (yf_rename22 < 0)
                    {
                      loop_counter[172]++;
                      yf_rename22 += World.height;
                    }

                    if ((((unsigned) xf_rename22) >= ((unsigned) view_width)) || (((unsigned) yf_rename22) >= ((unsigned) view_height)))
                    {
                      loop_counter[173]++;
                      {
                        goto return22;
                      }
                    }

                    i_rename22 = ((offset_rename22 + (color_rename22 * debris_areas)) + (((yf_rename22 >> 8) % debris_y_areas) * debris_x_areas)) + ((xf_rename22 >> 8) % debris_x_areas);
                    if (fastshot_num[i_rename22] >= 255)
                    {
                      loop_counter[174]++;
                      {
                        goto return22;
                      }
                    }

                    if (fastshot_num[i_rename22] >= fastshot_max[i_rename22])
                    {
                      loop_counter[175]++;
                      if (fastshot_num[i_rename22] == 0)
                      {
                        loop_counter[176]++;
                        fastshot_ptr[i_rename22] = (debris_t *) malloc((fastshot_max[i_rename22] = 16) * (sizeof(*fastshot_ptr[i_rename22])));
                      }
                      else
                      {
                        fastshot_ptr[i_rename22] = (debris_t *) realloc(fastshot_ptr[i_rename22], (fastshot_max[i_rename22] += fastshot_max[i_rename22]) * (sizeof(*fastshot_ptr[i_rename22])));
                      }

                      if (fastshot_ptr[i_rename22] == 0)
                      {
                        loop_counter[177]++;
                        error("No memory for debris");
                        fastshot_num[i_rename22] = 0;
                        {
                          goto return22;
                        }
                      }

                    }

                    fastshot_ptr[i_rename22][fastshot_num[i_rename22]].x = (unsigned char) xf_rename22;
                    fastshot_ptr[i_rename22][fastshot_num[i_rename22]].y = (unsigned char) yf_rename22;
                    fastshot_num[i_rename22]++;
                    ;
                    return22:
                    ;

                  }
                    break;

                  case OBJ_TORPEDO:
                    len_rename9 = distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN;
                    Send_missile(conn_rename9, x_rename9, y_rename9, len_rename9, shot->missile_dir);
                    break;

                  case OBJ_SMART_SHOT:
                    len_rename9 = distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN;
                    Send_missile(conn_rename9, x_rename9, y_rename9, len_rename9, shot->missile_dir);
                    break;

                  case OBJ_HEAT_SHOT:
                    len_rename9 = distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN;
                    Send_missile(conn_rename9, x_rename9, y_rename9, len_rename9, shot->missile_dir);
                    break;

                  case OBJ_BALL:
                    Send_ball(conn_rename9, x_rename9, y_rename9, shot->id);
                    break;

                  case OBJ_MINE:
                  {
                    int id_rename9 = 0;
                    int laid_by_team_rename9 = 0;
                    int confused_rename9 = 0;
                    mineobject *mine_rename9 = MINE_PTR(shot);
                    {
                      int Wrap_length_result0_rename9;
                      Wrap_length_result0_rename9 = Wrap_length(pl_rename9->pos.x - mine_rename9->pos.x, pl_rename9->pos.y - mine_rename9->pos.y);
                      if (identifyMines && (Wrap_length_result0_rename9 < ((SHIP_SZ + MINE_SENSE_BASE_RANGE) + (pl_rename9->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))))
                      {
                        loop_counter[178]++;
                        id_rename9 = mine_rename9->id;
                        if (id_rename9 == NO_ID)
                        {
                          loop_counter[179]++;
                          id_rename9 = EXPIRED_MINE_ID;
                        }

                        {
                          int BIT_result0_rename9;
                          BIT_result0_rename9 = BIT(mine_rename9->status, CONFUSED);
                          if (BIT_result0_rename9)
                          {
                            loop_counter[180]++;
                            confused_rename9 = 1;
                          }

                        }
                      }

                    }
                    {
                      int BIT_result0_rename9;
                      BIT_result0_rename9 = BIT(Players[GetInd[mine_rename9->id]]->status, PAUSE);
                      if ((mine_rename9->id != NO_ID) && BIT_result0_rename9)
                      {
                        loop_counter[181]++;
                        laid_by_team_rename9 = 1;
                      }
                      else
                      {
                        laid_by_team_rename9 = Team_immune(mine_rename9->id, pl_rename9->id) || (BIT(mine_rename9->status, OWNERIMMUNE) && (mine_rename9->owner == pl_rename9->id));
                        if (confused_rename9)
                        {
                          loop_counter[182]++;
                          id_rename9 = 0;
                          laid_by_team_rename9 = rfrac() < 0.5f;
                        }

                      }

                    }
                    Send_mine(conn_rename9, x_rename9, y_rename9, laid_by_team_rename9, id_rename9);
                  }
                    break;

                  case OBJ_ITEM:
                  {
                    int item_type_rename9 = shot->info;
                    {
                      int BIT_result0_rename9;
                      BIT_result0_rename9 = BIT(shot->status, RANDOM_ITEM);
                      if (BIT_result0_rename9)
                      {
                        loop_counter[183]++;
                        item_type_rename9 = Choose_random_item();
                      }

                    }
                    Send_item(conn_rename9, x_rename9, y_rename9, item_type_rename9);
                  }
                    break;

                  default:
                    error("Frame_shots: Shot type %d not defined.", shot->type);
                    break;

                }

              }

              return9:
              ;

            }
            {
              int ind_rename10 = ind_rename1;
              int conn_rename10 = conn_rename1;
              int i_rename10;
              int k_rename10;
              int mask_rename10;
              int shownuke_rename10;
              int size_rename10;
              player *pl_rename10 = Players[ind_rename10];
              object * shot;
              DFLOAT x_rename10;
              DFLOAT y_rename10;
              {
                num_radar = 0;
                return23:
                ;

              }
              if (nukesOnRadar)
              {
                loop_counter[184]++;
                mask_rename10 = ((OBJ_SMART_SHOT | OBJ_TORPEDO) | OBJ_HEAT_SHOT) | OBJ_MINE;
              }
              else
              {
                mask_rename10 = missilesOnRadar ? (OBJ_SMART_SHOT | OBJ_TORPEDO) | OBJ_HEAT_SHOT : 0;
                mask_rename10 |= minesOnRadar ? OBJ_MINE : 0;
              }

              if (treasuresOnRadar)
              {
                loop_counter[185]++;
                mask_rename10 |= OBJ_BALL;
              }

              if (asteroidsOnRadar)
              {
                loop_counter[186]++;
                mask_rename10 |= OBJ_ASTEROID;
              }

              if (mask_rename10)
              {
                loop_counter[187]++;
                for (i_rename10 = 0; i_rename10 < NumObjs; i_rename10++)
                {
                  loop_counter[188]++;
                  shot = Obj[i_rename10];
                  {
                    int BIT_result0_rename10;
                    BIT_result0_rename10 = BIT(shot->type, mask_rename10);
                    if (!BIT_result0_rename10)
                    {
                      loop_counter[189]++;
                      continue;
                    }

                  }
                  shownuke_rename10 = nukesOnRadar && shot->mods.nuclear;
                  if (shownuke_rename10 && (frame_loops & 2))
                  {
                    loop_counter[190]++;
                    size_rename10 = 3;
                  }
                  else
                  {
                    size_rename10 = 0;
                  }

                  {
                    int BIT_result0_rename10;
                    BIT_result0_rename10 = BIT(shot->type, OBJ_MINE);
                    if (BIT_result0_rename10)
                    {
                      loop_counter[191]++;
                      if ((!minesOnRadar) && (!shownuke_rename10))
                      {
                        loop_counter[192]++;
                        continue;
                      }

                      if ((frame_loops % 8) >= 6)
                      {
                        loop_counter[193]++;
                        continue;
                      }

                    }
                    else
                    {
                      int BIT_result0_rename10;
                      BIT_result0_rename10 = BIT(shot->type, OBJ_BALL);
                      if (BIT_result0_rename10)
                      {
                        loop_counter[194]++;
                        size_rename10 = 2;
                      }
                      else
                      {
                        int BIT_result0_rename10;
                        BIT_result0_rename10 = BIT(shot->type, OBJ_ASTEROID);
                        if (BIT_result0_rename10)
                        {
                          loop_counter[195]++;
                          size_rename10 = WIRE_PTR(shot)->size + 1;
                          size_rename10 |= 0x80;
                        }
                        else
                        {
                          if ((!missilesOnRadar) && (!shownuke_rename10))
                          {
                            loop_counter[196]++;
                            continue;
                          }

                          if (frame_loops & 1)
                          {
                            loop_counter[197]++;
                            continue;
                          }

                        }

                      }

                    }

                  }
                  x_rename10 = shot->pos.x;
                  y_rename10 = shot->pos.y;
                  {
                    int Wrap_length_result0_rename10;
                    Wrap_length_result0_rename10 = Wrap_length(pl_rename10->pos.x - x_rename10, pl_rename10->pos.y - y_rename10);
                    if (Wrap_length_result0_rename10 <= pl_rename10->sensor_range)
                    {
                      loop_counter[198]++;
                      {
                        int s_rename24 = size_rename10;
                        int y_rename24 = (int) y_rename10;
                        int x_rename24 = (int) x_rename10;
                        radar_t *p_rename24;
                        if ((num_radar + 1) > max_radar)
                        {
                          loop_counter[199]++;
                          if (max_radar <= 0)
                          {
                            loop_counter[200]++;
                            max_radar = 1 + 2;
                            radar_ptr = (radar_t *) malloc(max_radar * (sizeof(radar_t)));
                            num_radar = 0;
                          }
                          else
                          {
                            max_radar = (max_radar << 1) + 1;
                            radar_ptr = (radar_t *) realloc(radar_ptr, max_radar * (sizeof(radar_t)));
                          }

                          if (radar_ptr == NULL)
                          {
                            loop_counter[201]++;
                            error("No memory");
                            num_radar = (max_radar = 0);
                            {
                              goto return24;
                            }
                          }

                        }

                        ;
                        p_rename24 = &radar_ptr[num_radar++];
                        p_rename24->x = x_rename24;
                        p_rename24->y = y_rename24;
                        p_rename24->size = s_rename24;
                        return24:
                        ;

                      }
                    }

                  }
                }

              }

              {
                int BIT_result0_rename10;
                BIT_result0_rename10 = BIT(World.rules->mode, TEAM_PLAY);
                if (((playersOnRadar || BIT_result0_rename10) || (NumPseudoPlayers > 0)) || (NumAlliances > 0))
                {
                  loop_counter[202]++;
                  for (k_rename10 = 0; k_rename10 < num_player_shuffle; k_rename10++)
                  {
                    loop_counter[203]++;
                    i_rename10 = player_shuffle_ptr[k_rename10];
                    {
                      int BIT_result0_rename10;
                      BIT_result0_rename10 = BIT(Players[i_rename10]->status, (PLAYING | PAUSE) | GAME_OVER);
                      int TEAM_result1_rename10;
                      TEAM_result1_rename10 = TEAM(i_rename10, ind_rename10);
                      int ALLIANCE_result2_rename10;
                      ALLIANCE_result2_rename10 = ALLIANCE(ind_rename10, i_rename10);
                      int OWNS_TANK_result3_rename10;
                      OWNS_TANK_result3_rename10 = OWNS_TANK(ind_rename10, i_rename10);
                      if (((Players[i_rename10]->conn == conn_rename10) || (BIT_result0_rename10 != PLAYING)) || ((((!TEAM_result1_rename10) && (!ALLIANCE_result2_rename10)) && (!OWNS_TANK_result3_rename10)) && ((!playersOnRadar) || (!pl_rename10->visibility[i_rename10].canSee))))
                      {
                        loop_counter[204]++;
                        continue;
                      }

                    }
                    x_rename10 = Players[i_rename10]->pos.x;
                    y_rename10 = Players[i_rename10]->pos.y;
                    {
                      int BIT_result0_rename10;
                      BIT_result0_rename10 = BIT(World.rules->mode, LIMITED_VISIBILITY);
                      int Wrap_length_result1_rename10;
                      Wrap_length_result1_rename10 = Wrap_length(pl_rename10->pos.x - x_rename10, pl_rename10->pos.y - y_rename10);
                      if (BIT_result0_rename10 && (Wrap_length_result1_rename10 > pl_rename10->sensor_range))
                      {
                        loop_counter[205]++;
                        continue;
                      }

                    }
                    {
                      int BIT_result0_rename10;
                      BIT_result0_rename10 = BIT(pl_rename10->used, HAS_COMPASS);
                      int BIT_result1_rename10;
                      BIT_result1_rename10 = BIT(pl_rename10->lock.tagged, LOCK_PLAYER);
                      if (((BIT_result0_rename10 && BIT_result1_rename10) && (GetInd[pl_rename10->lock.pl_id] == i_rename10)) && ((frame_loops % 5) >= 3))
                      {
                        loop_counter[206]++;
                        continue;
                      }

                    }
                    size_rename10 = 3;
                    {
                      int TEAM_result0_rename10;
                      TEAM_result0_rename10 = TEAM(i_rename10, ind_rename10);
                      int ALLIANCE_result1_rename10;
                      ALLIANCE_result1_rename10 = ALLIANCE(ind_rename10, i_rename10);
                      int OWNS_TANK_result2_rename10;
                      OWNS_TANK_result2_rename10 = OWNS_TANK(ind_rename10, i_rename10);
                      if ((TEAM_result0_rename10 || ALLIANCE_result1_rename10) || OWNS_TANK_result2_rename10)
                      {
                        loop_counter[207]++;
                        size_rename10 |= 0x80;
                      }

                    }
                    {
                      int s_rename25 = size_rename10;
                      int y_rename25 = (int) y_rename10;
                      int x_rename25 = (int) x_rename10;
                      radar_t *p_rename25;
                      if ((num_radar + 1) > max_radar)
                      {
                        loop_counter[208]++;
                        if (max_radar <= 0)
                        {
                          loop_counter[209]++;
                          max_radar = 1 + 2;
                          radar_ptr = (radar_t *) malloc(max_radar * (sizeof(radar_t)));
                          num_radar = 0;
                        }
                        else
                        {
                          max_radar = (max_radar << 1) + 1;
                          radar_ptr = (radar_t *) realloc(radar_ptr, max_radar * (sizeof(radar_t)));
                        }

                        if (radar_ptr == NULL)
                        {
                          loop_counter[210]++;
                          error("No memory");
                          num_radar = (max_radar = 0);
                          {
                            goto return25;
                          }
                        }

                      }

                      ;
                      p_rename25 = &radar_ptr[num_radar++];
                      p_rename25->x = x_rename25;
                      p_rename25->y = y_rename25;
                      p_rename25->size = s_rename25;
                      return25:
                      ;

                    }
                  }

                }

              }
              {
                int conn_rename26 = conn_rename10;
                int i_rename26;
                int dest_rename26;
                int tmp_rename26;
                radar_t *p_rename26;
                const int radar_width_rename26 = 256;
                int radar_height_rename26 = (radar_width_rename26 * World.y) / World.x;
                int radar_x_rename26;
                int radar_y_rename26;
                int send_x_rename26;
                int send_y_rename26;
                shuffle_t *radar_shuffle_rename26;
                size_t shuffle_bufsize_rename26;
                {
                  int MIN_result0_rename26;
                  MIN_result0_rename26 = MIN(256, 65535);
                  if (num_radar > MIN_result0_rename26)
                  {
                    loop_counter[211]++;
                    num_radar = MIN(256, 65535);
                  }

                }
                shuffle_bufsize_rename26 = num_radar * (sizeof(shuffle_t));
                radar_shuffle_rename26 = (shuffle_t *) malloc(shuffle_bufsize_rename26);
                if (radar_shuffle_rename26 == ((shuffle_t *) NULL))
                {
                  loop_counter[212]++;
                  {
                    goto return26;
                  }
                }

                for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                {
                  loop_counter[213]++;
                  radar_shuffle_rename26[i_rename26] = i_rename26;
                }

                for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                {
                  loop_counter[214]++;
                  dest_rename26 = (int) (rfrac() * num_radar);
                  tmp_rename26 = radar_shuffle_rename26[i_rename26];
                  radar_shuffle_rename26[i_rename26] = radar_shuffle_rename26[dest_rename26];
                  radar_shuffle_rename26[dest_rename26] = tmp_rename26;
                }

                {
                  int Get_conn_version_result0_rename26;
                  Get_conn_version_result0_rename26 = Get_conn_version(conn_rename26);
                  if (Get_conn_version_result0_rename26 <= 0x4400)
                  {
                    loop_counter[215]++;
                    for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                    {
                      loop_counter[216]++;
                      p_rename26 = &radar_ptr[radar_shuffle_rename26[i_rename26]];
                      radar_x_rename26 = (radar_width_rename26 * p_rename26->x) / World.width;
                      radar_y_rename26 = (radar_height_rename26 * p_rename26->y) / World.height;
                      send_x_rename26 = (World.width * radar_x_rename26) / radar_width_rename26;
                      send_y_rename26 = (World.height * radar_y_rename26) / radar_height_rename26;
                      Send_radar(conn_rename26, send_x_rename26, send_y_rename26, p_rename26->size);
                    }

                  }
                  else
                  {
                    unsigned char buf_rename26[3 * 256];
                    int buf_index_rename26 = 0;
                    int fast_count_rename26 = 0;
                    if (num_radar > 256)
                    {
                      loop_counter[217]++;
                      num_radar = 256;
                    }

                    for (i_rename26 = 0; i_rename26 < num_radar; i_rename26++)
                    {
                      loop_counter[218]++;
                      p_rename26 = &radar_ptr[radar_shuffle_rename26[i_rename26]];
                      radar_x_rename26 = (radar_width_rename26 * p_rename26->x) / World.width;
                      radar_y_rename26 = (radar_height_rename26 * p_rename26->y) / World.height;
                      if (radar_y_rename26 >= 1024)
                      {
                        loop_counter[219]++;
                        continue;
                      }

                      buf_rename26[buf_index_rename26++] = (unsigned char) radar_x_rename26;
                      buf_rename26[buf_index_rename26++] = (unsigned char) (radar_y_rename26 & 0xFF);
                      buf_rename26[buf_index_rename26] = (unsigned char) ((radar_y_rename26 >> 2) & 0xC0);
                      if (p_rename26->size & 0x80)
                      {
                        loop_counter[220]++;
                        buf_rename26[buf_index_rename26] |= (unsigned char) 0x20;
                      }

                      buf_rename26[buf_index_rename26] |= (unsigned char) (p_rename26->size & 0x07);
                      buf_index_rename26++;
                      fast_count_rename26++;
                    }

                    if (fast_count_rename26 > 0)
                    {
                      loop_counter[221]++;
                      Send_fastradar(conn_rename26, buf_rename26, fast_count_rename26);
                    }

                  }

                }
                free(radar_shuffle_rename26);
                return26:
                ;

              }
              return10:
              ;

            }
            {
              int ind_rename11 = i_rename1;
              player *pl_rename11 = Players[ind_rename11];
              if (pl_rename11->lose_item_state != 0)
              {
                loop_counter[222]++;
                Send_loseitem(pl_rename11->lose_item, pl_rename11->conn);
                if (pl_rename11->lose_item_state == 1)
                {
                  loop_counter[223]++;
                  pl_rename11->lose_item_state = -5;
                }

                if (pl_rename11->lose_item_state < 0)
                {
                  loop_counter[224]++;
                  pl_rename11->lose_item_state++;
                }

              }

              return11:
              ;

            }
            {
              int conn_rename12 = conn_rename1;
              int i_rename12;
              for (i_rename12 = 0; i_rename12 < DEBRIS_TYPES; i_rename12++)
              {
                loop_counter[225]++;
                if (debris_num[i_rename12] != 0)
                {
                  loop_counter[226]++;
                  Send_debris(conn_rename12, i_rename12, (unsigned char *) debris_ptr[i_rename12], debris_num[i_rename12]);
                  debris_num[i_rename12] = 0;
                }

              }

              return12:
              ;

            }
            {
              int conn_rename13 = conn_rename1;
              int i_rename13;
              for (i_rename13 = 0; i_rename13 < (DEBRIS_TYPES * 2); i_rename13++)
              {
                loop_counter[227]++;
                if (fastshot_num[i_rename13] != 0)
                {
                  loop_counter[228]++;
                  Send_fastshot(conn_rename13, i_rename13, (unsigned char *) fastshot_ptr[i_rename13], fastshot_num[i_rename13]);
                  fastshot_num[i_rename13] = 0;
                }

              }

              return13:
              ;

            }
          }

          sound_play_queued(Players[ind_rename1]);
          Send_end_of_frame(conn_rename1);
        }

        oldTimeLeft_rename1 = newTimeLeft_rename1;
        {
          free(radar_ptr);
          radar_ptr = NULL;
          num_radar = 0;
          max_radar = 0;
          return14:
          ;

        }
        return1:
        ;

      }
    }

  }

  if ((((!NoQuit) && (NumPlayers == (NumRobots + NumPseudoPlayers))) && (!login_in_progress)) && (!NumQueuedPlayers))
  {
    loop_counter[229]++;
    if (!NoPlayersEnteredYet)
    {
      loop_counter[230]++;
      {
        int return_value;
        player *pl_rename2;
        char msg_rename2[MSG_LEN];
        if (ShutdownServer == 0)
        {
          loop_counter[231]++;
          errno = 0;
          error("Shutting down...");
          sprintf(msg_rename2, "shutting down: %s", ShutdownReason);
        }
        else
        {
          sprintf(msg_rename2, "server exiting");
        }

        while (NumPlayers > 0)
        {
          loop_counter[232]++;
          pl_rename2 = Players[NumPlayers - 1];
          if (pl_rename2->conn == NOT_CONNECTED)
          {
            loop_counter[233]++;
            Delete_player(NumPlayers - 1);
          }
          else
          {
            Destroy_connection(pl_rename2->conn, msg_rename2);
          }

        }

        Meta_gone();
        Contact_cleanup();
        Free_players();
        Free_shots();
        Free_map();
        Free_cells();
        Free_options();
        Log_game("END");
        exit(0);
        {
          return_value = FALSE;
          goto return2;
        }
        return2:
        ;

      }
    }

    {
      int time_result0;
      time_result0 = time(NULL);
      if ((serverTime + (5 * 60)) < time_result0)
      {
        loop_counter[234]++;
        error("First player has yet to show his butt, I'm bored... Bye!");
        Log_game("NOSHOW");
        {
          int return_value;
          player *pl_rename3;
          char msg_rename3[MSG_LEN];
          if (ShutdownServer == 0)
          {
            loop_counter[235]++;
            errno = 0;
            error("Shutting down...");
            sprintf(msg_rename3, "shutting down: %s", ShutdownReason);
          }
          else
          {
            sprintf(msg_rename3, "server exiting");
          }

          while (NumPlayers > 0)
          {
            loop_counter[236]++;
            pl_rename3 = Players[NumPlayers - 1];
            if (pl_rename3->conn == NOT_CONNECTED)
            {
              loop_counter[237]++;
              Delete_player(NumPlayers - 1);
            }
            else
            {
              Destroy_connection(pl_rename3->conn, msg_rename3);
            }

          }

          Meta_gone();
          Contact_cleanup();
          Free_players();
          Free_shots();
          Free_map();
          Free_cells();
          Free_options();
          Log_game("END");
          exit(0);
          {
            return_value = FALSE;
            goto return3;
          }
          return3:
          ;

        }
      }

    }
  }

  Queue_loop();

  print_loop_counter:
  end_timing();
  {
    printf("loop counter = (");
    int i;
    for (i = 0; i < 238; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
  print_timing();

}

void Main_loop(void)
{
//---------------------modified by TJSong----------------------//
    if(client_join == 0){
        goto wait_for_client_join;
    }else{
        //after certain jobs, finish the game
        if(main_job_cnt > 3000)
            End_game();
    }
//---------------------modified by TJSong----------------------//

//---------------------modified by TJSong----------------------//
    static int jump = 0;

#if HETERO_EN
    int pid = getpid();
#endif
   print_deadline(DEADLINE_TIME); //print deadline 
//---------------------modified by TJSong----------------------//

//---------------------modified by TJSong----------------------//
    // Perform slicing and prediction
    struct slice_return predicted_exec_time;
    predicted_exec_time.big = 0;
    predicted_exec_time.little = 0;
    /*
      CASE 0 = to get prediction equation
      CASE 3 = running on default linux governors
      CASE 4 = running on our prediction
      CASE 6 = running on pid
    */
    #if GET_PREDICT /* CASE 0 */
      predicted_exec_time = _SLICE_();
    #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
      predicted_exec_time = _SLICE_();
      moment_timing_print(0); //moment_start
    #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
      moment_timing_print(0); //moment_start
      
      start_timing();
      predicted_exec_time = _SLICE_();
      end_timing();
      slice_time = print_slice_timing();
      
      start_timing();
      #if OVERHEAD_EN //with overhead
        #if HETERO_EN
          current_core = set_freq_hetero(predicted_exec_time.big, 
              predicted_exec_time.little, slice_time, DEADLINE_TIME, 
              AVG_DVFS_TIME, pid, is_stable_big, is_stable_little); //do dvfs
        #else
          #if CORE
            set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #else
            set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #endif
        #endif
      #endif
      end_timing();
      dvfs_time = print_dvfs_timing();

      moment_timing_print(1); //moment_start
    #elif ORACLE_EN /* CASE 5 */
    #elif PID_EN /* CASE 6 */
      moment_timing_print(0); //moment_start
      
      start_timing();
      predicted_exec_time = pid_controller(exec_time); //pid == slice
      end_timing();
      slice_time = print_slice_timing();
      
      start_timing();
      #if CORE
        set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
      #else
        set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
      #endif
      end_timing();
      dvfs_time = print_dvfs_timing();
      
      moment_timing_print(1); //moment_start
    #elif PROACTIVE_EN /* CASE 7 */
    #endif
//---------------------modified by TJSong----------------------//

            start_timing();

            main_job_cnt++;
wait_for_client_join:
            ;
            main_loops++;

            if ((main_loops & 0x3F) == 0) {
                Meta_update(0);
            }

            /*
             * Check for possible shutdown, the server will
             * shutdown when ShutdownServer (a counter) reaches 0.
             * If the counter is < 0 then no shutdown is in progress.
             */
            if (ShutdownServer >= 0) {
                if (ShutdownServer == 0) {
                    End_game();
                }
                else {
                    ShutdownServer--;
                }
            }

            Input();

            if (NumPlayers > NumRobots + NumPseudoPlayers || RawMode) {

                if (NoPlayersEnteredYet) {
                    if (NumPlayers > NumRobots + NumPseudoPlayers) {
                        NoPlayersEnteredYet = false;
                        if (gameDuration > 0.0) {
                            xpprintf("%s Server will stop in %g minutes.\n", showtime(), gameDuration);
                            gameOverTime = (time_t)(gameDuration * 60) + time((time_t *)NULL);
                        }
                    }
                }

                Update_objects();

                if ((main_loops % UPDATES_PR_FRAME) == 0) {
                    Frame_update();
                }
            }

            if (!NoQuit
                    && NumPlayers == NumRobots + NumPseudoPlayers
                    && !login_in_progress
                    && !NumQueuedPlayers) {

                if (!NoPlayersEnteredYet) {
                    End_game();
                }
                if (serverTime + 5*60 < time(NULL)) {
                    error("First player has yet to show his butt, I'm bored... Bye!");
                    Log_game("NOSHOW");
                    End_game();
                }
            }

            Queue_loop();
            if(client_join == 0)
                return;

  // End timing of loop and print out time
  end_timing();
//---------------------modified by TJSong----------------------//
    _DEFINE_TIME_();

    #if IDLE_EN
      additional_dvfs_times =
        dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2] +
        dvfs_table[MIN_FREQ/100000-2][cur_freq/100000-2];
    #endif

    #if ONLINE_EN /* CASE 0, 2, 3 and 4 */
      #if GET_PREDICT || GET_OVERHEAD \
            || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN) \
            || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN) 
        start_timing();
        #if !HETERO_EN
          (void)get_predicted_time(TYPE_SOLVE, solver, NULL, 0, exec_time,
            cur_freq);
        #elif HETERO_EN
          if     (current_core == 1)
            is_stable_big    = get_predicted_time_big   (TYPE_SOLVE, 
                solver_big,    NULL, 0, exec_time, cur_freq);
          else if(current_core == 0)
            is_stable_little = get_predicted_time_little(TYPE_SOLVE, 
                solver_little, NULL, 0, exec_time, cur_freq);
        #endif
        end_timing();
        update_time = exec_timing();
      #endif
    #endif

    _DELAY_();

    _PRINT_INFO_();
//---------------------modified by TJSong----------------------//
}


/*
 *  Last function, exit with grace.
 */
int End_game(void)
{
    player		*pl;
    char		msg[MSG_LEN];

//---------------------modified by TJSong----------------------//
    fclose_all();//TJSong
//---------------------modified by TJSong----------------------//

    if (ShutdownServer == 0) {
	errno = 0;
	error("Shutting down...");
	sprintf(msg, "shutting down: %s", ShutdownReason);
    } else {
	sprintf(msg, "server exiting");
    }

    while (NumPlayers > 0) {	/* Kick out all remaining players */
	pl = Players[NumPlayers - 1];
	if (pl->conn == NOT_CONNECTED) {
	    Delete_player(NumPlayers - 1);
	} else {
	    Destroy_connection(pl->conn, msg);
	}
    }

    /* Tell meta server that we are gone. */
    Meta_gone();

    Contact_cleanup();

    Free_players();
    Free_shots();
    Free_map();
    Free_cells();
    Free_options();
    Log_game("END");			    /* Log end */

#ifndef _WINDOWS
    exit (0);
#endif
	return(FALSE);                  /* return FALSE so windows bubbles out of the main loop */
}

/*
 * Return a good team number for a player.
 *
 * If the team is not specified, the player is assigned
 * to a non-empty team which has space.
 *
 * If there is none or only one team with playing (i.e. non-paused)
 * players the player will be assigned to a randomly chosen empty team.
 *
 * If there is more than one team with playing players,
 * the player will be assigned randomly to a team which
 * has the least number of playing players.
 *
 * If all non-empty teams are full, the player is assigned
 * to a randomly chosen available team.
 *
 * Prefer not to place players in the robotTeam if possible.
 */
int Pick_team(int pick_for_type)
{
    int			i,
			least_players,
			num_available_teams = 0,
			playing_teams = 0,
			losing_team;
    player		*pl;
    int			playing[MAX_TEAMS];
    int			free_bases[MAX_TEAMS];
    int			available_teams[MAX_TEAMS];
    long		team_score[MAX_TEAMS];
    long		losing_score;

    for (i = 0; i < MAX_TEAMS; i++) {
	free_bases[i] = World.teams[i].NumBases - World.teams[i].NumMembers;
	playing[i] = 0;
	team_score[i] = 0;
	available_teams[i] = 0;
    }
    if (restrictRobots) {
	if (pick_for_type == PickForRobot) {
	    if (free_bases[robotTeam] > 0) {
		return robotTeam;
	    } else {
		return TEAM_NOT_SET;
	    }
	}
    }
    if (reserveRobotTeam) {
	if (pick_for_type != PickForRobot) {
	    free_bases[robotTeam] = 0;
	}
    }

    /*
     * Find out which teams have actively playing members.
     * Exclude paused players and tanks.
     * And calculate the score for each team.
     */
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (IS_TANK_PTR(pl)) {
	    continue;
	}
	if (BIT(pl->status, PAUSE)) {
	    continue;
	}
	if (!playing[pl->team]++) {
	    playing_teams++;
	}
	if (IS_HUMAN_PTR(pl) || IS_ROBOT_PTR(pl)) {
	    team_score[pl->team] += pl->score;
	}
    }
    if (playing_teams <= 1) {
	for (i = 0; i < MAX_TEAMS; i++) {
	    if (!playing[i] && free_bases[i] > 0) {
		available_teams[num_available_teams++] = i;
	    }
	}
    } else {
	least_players = NumPlayers;
	for (i = 0; i < MAX_TEAMS; i++) {
	    /* We fill teams with players first. */
	    if (playing[i] > 0 && free_bases[i] > 0) {
		if (playing[i] < least_players) {
		    least_players = playing[i];
		}
	    }
	}

	for (i = 0; i < MAX_TEAMS; i++) {
	    if (free_bases[i] > 0) {
		if (least_players == NumPlayers
		    || playing[i] == least_players) {
		    available_teams[num_available_teams++] = i;
		}
	    }
	}
    }

    if (!num_available_teams) {
	for (i = 0; i < MAX_TEAMS; i++) {
	    if (free_bases[i] > 0) {
		available_teams[num_available_teams++] = i;
	    }
	}
    }

    if (num_available_teams == 1) {
	return available_teams[0];
    }

    if (num_available_teams > 1) {
	losing_team = -1;
	losing_score = LONG_MAX;
	for (i = 0; i < num_available_teams; i++) {
	    if (team_score[available_teams[i]] < losing_score
		&& available_teams[i] != robotTeam) {
		losing_team = available_teams[i];
		losing_score = team_score[losing_team];
	    }
	}
	return losing_team;
    }

    /*NOTREACHED*/
    return TEAM_NOT_SET;
}


/*
 * Return status for server
 *
 * TODO
*/
void Server_info(char *str, unsigned max_size)
{
    int			i, j, k;
    player		*pl, **order, *best = NULL;
    DFLOAT		ratio, best_ratio = -1e7;
    char		name[MAX_CHARS];
    char		lblstr[MAX_CHARS];
    char		msg[MSG_LEN];

    sprintf(str,
	    "SERVER VERSION...: %s\n"
	    "STATUS...........: %s\n"
	    "MAX SPEED........: %d fps\n"
	    "WORLD (%3dx%3d)..: %s\n"
	    "      AUTHOR.....: %s\n"
	    "PLAYERS (%2d/%2d)..:\n",
	    server_version,
	    (game_lock && ShutdownServer == -1) ? "locked" :
	    (!game_lock && ShutdownServer != -1) ? "shutting down" :
	    (game_lock && ShutdownServer != -1) ? "locked and shutting down" : "ok",
	    FPS,
	    World.x, World.y, World.name, World.author,
	    NumPlayers, World.NumBases);

    if (strlen(str) >= max_size) {
	errno = 0;
	error("Server_info string overflow (%d)", max_size);
	str[max_size - 1] = '\0';
	return;
    }
    if (NumPlayers <= 0) {
	return;
    }

    sprintf(msg,
	   "\nNO:  TM: NAME:             LIFE:   SC:    PLAYER:\n"
	   "-------------------------------------------------\n");
    if (strlen(msg) + strlen(str) >= max_size) {
	return;
    }
    strlcat(str, msg, max_size);

    if ((order = (player **) malloc(NumPlayers * sizeof(player *))) == NULL) {
	error("No memory for order");
	return;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (BIT(World.rules->mode, LIMITED_LIVES)) {
	    ratio = (DFLOAT) pl->score;
	} else {
	    ratio = (DFLOAT) pl->score / (pl->life + 1);
	}
	if ((best == NULL
		|| ratio > best_ratio)
	    && !BIT(pl->status, PAUSE)) {
	    best_ratio = ratio;
	    best = pl;
	}
	for (j = 0; j < i; j++) {
	    if (order[j]->score < pl->score) {
		for (k = i; k > j; k--) {
		    order[k] = order[k - 1];
		}
		break;
	    }
	}
	order[j] = pl;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = order[i];
	strlcpy(name, pl->name, MAX_CHARS);
	if (IS_ROBOT_PTR(pl)) {
	    if ((k = Robot_war_on_player(GetInd[pl->id])) != NO_ID) {
		sprintf(name + strlen(name), " (%s)", Players[GetInd[k]]->name);
		if (strlen(name) >= 19) {
		    strcpy(&name[17], ")");
		}
	    }
	}
	sprintf(lblstr, "%c%c %-19s%03d%6d",
		(pl == best) ? '*' : pl->mychar,
		(pl->team == TEAM_NOT_SET) ? ' ' : (pl->team + '0'),
		name, (int)pl->life, (int)pl->score);
	sprintf(msg, "%2d... %-36s%s@%s\n",
		i+1, lblstr, pl->realname,
		IS_HUMAN_PTR(pl)
		? pl->hostname
		: "xpilot.org");
	if (strlen(msg) + strlen(str) >= max_size)
	    break;
	strlcat(str, msg, max_size);
    }
    free(order);
}


static void Handle_signal(int sig_no)
{
    errno = 0;

#ifndef _WINDOWS
    switch (sig_no) {

    case SIGHUP:
	if (NoQuit) {
	    signal(SIGHUP, SIG_IGN);
	    return;
	}
	error("Caught SIGHUP, terminating.");
	End_game();
	break;
    case SIGINT:
	error("Caught SIGINT, terminating.");
	End_game();
	break;
    case SIGTERM:
	error("Caught SIGTERM, terminating.");
	End_game();
	break;

    default:
	error("Caught unkown signal: %d", sig_no);
	End_game();
	break;
    }
#endif
    _exit(sig_no);	/* just in case */
}


void Log_game(const char *heading)
{
#ifdef LOG
    char str[1024];
    FILE *fp;
    char timenow[81];
    struct tm *ptr;
    time_t lt;

    if (!Log)
	return;

    lt = time(NULL);
    ptr = localtime(&lt);
    strftime(timenow,79,"%I:%M:%S %p %Z %A, %B %d, %Y",ptr);

    sprintf(str,"%-50.50s\t%10.10s@%-15.15s\tWorld: %-25.25s\t%10.10s\n",
	    timenow,
	    Server.owner,
	    Server.host,
	    World.name,
	    heading);

    if ((fp = fopen(Conf_logfile(), "a")) == NULL) {	/* Couldn't open file */
	error("Couldn't open log file, contact %s", Conf_localguru());
	return;
    }

    fputs(str, fp);
    fclose(fp);
#endif
}

void Game_Over(void)
{
    long		maxsc, minsc;
    int			i, win, loose;
    char		msg[128];

    Set_message("Game over...");

    /*
     * Hack to prevent Compute_Game_Status from starting over again...
     */
    gameDuration = -1.0;

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	int teamscore[MAX_TEAMS];
	maxsc = -32767;
	minsc = 32767;
	win = loose = -1;

	for (i=0; i < MAX_TEAMS; i++) {
	    teamscore[i] = 1234567; /* These teams are not used... */
	}
	for (i=0; i < NumPlayers; i++) {
	    int team;
	    if (IS_HUMAN_IND(i)) {
		team = Players[i]->team;
		if (teamscore[team] == 1234567) {
		    teamscore[team] = 0;
		}
		teamscore[team] += Players[i]->score;
	    }
	}

	for (i=0; i < MAX_TEAMS; i++) {
	    if (teamscore[i] != 1234567) {
		if (teamscore[i] > maxsc) {
		    maxsc = teamscore[i];
		    win = i;
		}
		if (teamscore[i] < minsc) {
		    minsc = teamscore[i];
		    loose = i;
		}
	    }
	}

	if (win != -1) {
	    sprintf(msg,"Best team (%ld Pts): Team %d", maxsc, win);
	    Set_message(msg);
	    xpprintf("%s\n", msg);
	}

	if (loose != -1 && loose != win) {
	    sprintf(msg,"Worst team (%ld Pts): Team %d", minsc, loose);
	    Set_message(msg);
	    xpprintf("%s\n", msg);
	}
    }

    maxsc = -32767;
    minsc = 32767;
    win = loose = -1;

    for (i = 0; i < NumPlayers; i++) {
	SET_BIT(Players[i]->status, GAME_OVER);
	if (IS_HUMAN_IND(i)) {
	    if (Players[i]->score > maxsc) {
		maxsc = Players[i]->score;
		win = i;
	    }
	    if (Players[i]->score < minsc) {
		minsc = Players[i]->score;
		loose = i;
	    }
	}
    }
    if (win != -1) {
	sprintf(msg,"Best human player: %s", Players[win]->name);
	Set_message(msg);
	xpprintf("%s\n", msg);
    }
    if (loose != -1 && loose != win) {
	sprintf(msg,"Worst human player: %s", Players[loose]->name);
	Set_message(msg);
	xpprintf("%s\n", msg);
    }
}


void Server_log_admin_message(int ind, const char *str)
{
    /*
     * Only log the message if logfile already exists,
     * is writable and less than some KBs in size.
     */
    const char		*logfilename = adminMessageFileName;
    const int		logfile_size_limit = adminMessageFileSizeLimit;
    FILE		*fp;
    struct stat		st;
    player		*pl = Players[ind];
    char		msg[MSG_LEN * 2];

    if ((logfilename != NULL) &&
	(logfilename[0] != '\0') &&
	(logfile_size_limit > 0) &&
	(access(logfilename, 2) == 0) &&
	(stat(logfilename, &st) == 0) &&
	(st.st_size + 80 < logfile_size_limit) &&
	((size_t)(logfile_size_limit - st.st_size - 80) > strlen(str)) &&
	((fp = fopen(logfilename, "a")) != NULL))
    {
	fprintf(fp,
		"%s[%s]{%s@%s(%s)|%s}:\n"
		"\t%s\n",
		showtime(),
		pl->name,
		pl->realname, pl->hostname,
		Get_player_addr(ind),
		Get_player_dpy(ind),
		str);
	fclose(fp);
	sprintf(msg, "%s [%s]:[%s]", str, pl->name, "GOD");
	Set_player_message(pl, msg);
    }
    else {
	Set_player_message(pl, " < GOD doesn't seem to be listening>");
    }
}


/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
extern char asteroid_version[];
extern char cannon_version[];
extern char cell_version[];
extern char checknames_version[];
extern char cmdline_version[];
extern char collision_version[];
extern char command_version[];
extern char config_version[];
extern char contact_version[];
extern char error_version[];
extern char event_version[];
extern char fileparser_version[];
extern char frame_version[];
extern char id_version[];
extern char item_version[];
extern char laser_version[];
extern char map_version[];
extern char math_version[];
extern char metaserver_version[];
extern char net_version[];
extern char netserver_version[];
extern char objpos_version[];
extern char option_version[];
extern char parser_version[];
extern char play_version[];
extern char player_version[];
extern char portability_version[];
extern char robot_version[];
extern char robotdef_version[];
extern char rules_version[];
extern char saudio_version[];
extern char sched_version[];
extern char score_version[];
extern char server_version[];
extern char ship_version[];
extern char shipshape_version[];
extern char shot_version[];
extern char socklib_version[];
extern char update_version[];
extern char walls_version[];
extern char wildmap_version[];

static void Check_server_versions(void)
{
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "asteroid", asteroid_version },
	{ "cannon", cannon_version },
	{ "cell", cell_version },
	{ "checknames", checknames_version },
	{ "cmdline", cmdline_version },
	{ "collision", collision_version },
	{ "command", command_version },
	{ "config", config_version },
	{ "contact", contact_version },
	{ "error", error_version },
	{ "event", event_version },
	{ "fileparser", fileparser_version },
	{ "frame", frame_version },
	{ "id", id_version },
	{ "item", item_version },
	{ "laser", laser_version },
	{ "map", map_version },
	{ "math", math_version },
	{ "metaserver", metaserver_version },
	{ "net", net_version },
	{ "netserver", netserver_version },
	{ "objpos", objpos_version },
	{ "option", option_version },
	{ "parser", parser_version },
	{ "play", play_version },
	{ "player", player_version },
	{ "portability", portability_version },
	{ "robot", robot_version },
	{ "robotdef", robotdef_version },
	{ "rules", rules_version },
	{ "saudio", saudio_version },
	{ "sched", sched_version },
	{ "score", score_version },
	{ "server", server_version },
	{ "ship", ship_version },
	{ "shipshape", shipshape_version },
	{ "shot", shot_version },
	{ "socklib", socklib_version },
	{ "update", update_version },
	{ "walls", walls_version },
	{ "wildmap", wildmap_version },
    };
    int			i;
    int			oops = 0;

    for (i = 0; i < NELEM(file_versions); i++) {
	if (strcmp(VERSION, file_versions[i].versionstr)) {
	    oops++;
	    error("Source file %s.c (\"%s\") is not compiled "
		  "for the current version (\"%s\")!",
		  file_versions[i].filename,
		  file_versions[i].versionstr,
		  VERSION);
	}
    }
    if (oops) {
	error("%d version inconsistency errors, cannot continue.", oops);
	error("Please recompile this program properly.");
	exit(1);
    }
}

#if defined(PLOCKSERVER) && defined(__linux__)
/*
 * Patches for Linux plock support by Steve Payne <srp20@cam.ac.uk>
 * also added the -pLockServer command line option.
 * All messed up by BG again, with thanks and apologies to Steve.
 */
/* Linux doesn't seem to have plock(2).  *sigh* (BG) */
#if !defined(PROCLOCK) || !defined(UNLOCK)
#define PROCLOCK	0x01
#define UNLOCK		0x00
#endif
static int plock(int op)
{
#if defined(MCL_CURRENT) && defined(MCL_FUTURE)
    return op ? mlockall(MCL_CURRENT | MCL_FUTURE) : munlockall();
#else
    return -1;
#endif
}
#endif

/*
 * Lock the server process data and code segments into memory
 * if this program has been compiled with the PLOCKSERVER flag.
 * Or unlock the server process if the argument is false.
 */
int plock_server(int onoff)
{
#ifdef PLOCKSERVER
    int			op;

    if (onoff) {
	op = PROCLOCK;
    }
    else {
	op = UNLOCK;
    }
    if (plock(op) == -1) {
	static int num_plock_errors;
	if (++num_plock_errors <= 3) {
	    error("Can't plock(%d)", op);
	}
	return -1;
    }
    return onoff;
#else
    if (onoff) {
	xpprintf("Can't plock: Server was not compiled with plock support\n");
    }
    return 0;
#endif
}
