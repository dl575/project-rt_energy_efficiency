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

//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <stdio.h>
//#include <signal.h>
//#include <errno.h>
//#include <time.h>
//#include <limits.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//
//#ifndef _WINDOWS
//# include <unistd.h>
//# ifndef __hpux
//#  include <sys/time.h>
//# endif
//# include <pwd.h>
//# include <sys/param.h>
//#endif
//
//#ifdef PLOCKSERVER
//# if defined(__linux__)
//#  include <sys/mman.h>
//# else
//#  include <sys/lock.h>
//# endif
//#endif
//
//#ifdef _WINDOWS
//# include <io.h>
//# include "NT/winServer.h"
//# include "NT/winSvrThread.h"
//#endif
//
//#define	SERVER
//#include "version.h"
//#include "config.h"
//#include "types.h"
//#include "serverconst.h"
//#include "global.h"
//#include "proto.h"
//#include "socklib.h"
//#include "map.h"
//#include "bit.h"
//#include "sched.h"
//#include "netserver.h"
//#include "error.h"
//#include "portability.h"
//#include "server.h"
//#include "commonproto.h"

char server_version[] = VERSION;

#ifndef	lint
//char xpilots_versionid[] = "@(#)$" TITLE " $";
#endif

/*
 * Global variables
 */
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

static void Check_server_versions();
extern void Main_loop();
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

		/*
    xpprintf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided LICENSE file.\n\n");
		 */

    init_error(argv[0]);
    Check_server_versions();

    seedMT((unsigned)time((time_t *)0) * Get_process_id());

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
    install_timer_tick(Main_loop, timer_tick_rate);

    sched();
    xpprintf("sched returned!?");
    End_game();
#endif

    return 1;
}

void Main_loop()
{
  struct timeval start, end;
  /*
  FILE *time_fp;
  time_fp = fopen("times.txt", "w");
  if (time_fp == NULL) {
    printf(stderr, "Error opening output file times.txt.\n");
    exit(1);
  }
  */

  gettimeofday(&start, NULL);

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

  gettimeofday(&end, NULL);
  //fprintf(time_fp, "time %d = %d us\n", main_loops, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
  printf("time %d = %d us\n", main_loops, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
}


/*
 *  Last function, exit with grace.
 */
int End_game()
{
    player		*pl;
    char		msg[MSG_LEN];

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


// void Game_Over()
// {
//     long		maxsc, minsc;
//     int			i, win, loose;
//     char		msg[128];
// 
//     Set_message("Game over...");
// 
//     /*
//      * Hack to prevent Compute_Game_Status from starting over again...
//      */
//     gameDuration = -1.0;
// 
//     if (BIT(World.rules->mode, TEAM_PLAY)) {
// 	int teamscore[MAX_TEAMS];
// 	maxsc = -32767;
// 	minsc = 32767;
// 	win = loose = -1;
// 
// 	for (i=0; i < MAX_TEAMS; i++) {
// 	    teamscore[i] = 1234567; /* These teams are not used... */
// 	}
// 	for (i=0; i < NumPlayers; i++) {
// 	    int team;
// 	    if (IS_HUMAN_IND(i)) {
// 		team = Players[i]->team;
// 		if (teamscore[team] == 1234567) {
// 		    teamscore[team] = 0;
// 		}
// 		teamscore[team] += Players[i]->score;
// 	    }
// 	}
// 
// 	for (i=0; i < MAX_TEAMS; i++) {
// 	    if (teamscore[i] != 1234567) {
// 		if (teamscore[i] > maxsc) {
// 		    maxsc = teamscore[i];
// 		    win = i;
// 		}
// 		if (teamscore[i] < minsc) {
// 		    minsc = teamscore[i];
// 		    loose = i;
// 		}
// 	    }
// 	}
// 
// 	if (win != -1) {
// 	    sprintf(msg,"Best team (%ld Pts): Team %d", maxsc, win);
// 	    Set_message(msg);
// 	    xpprintf("%s\n", msg);
// 	}
// 
// 	if (loose != -1 && loose != win) {
// 	    sprintf(msg,"Worst team (%ld Pts): Team %d", minsc, loose);
// 	    Set_message(msg);
// 	    xpprintf("%s\n", msg);
// 	}
//     }
// 
//     maxsc = -32767;
//     minsc = 32767;
//     win = loose = -1;
// 
//     for (i = 0; i < NumPlayers; i++) {
// 	SET_BIT(Players[i]->status, GAME_OVER);
// 	if (IS_HUMAN_IND(i)) {
// 	    if (Players[i]->score > maxsc) {
// 		maxsc = Players[i]->score;
// 		win = i;
// 	    }
// 	    if (Players[i]->score < minsc) {
// 		minsc = Players[i]->score;
// 		loose = i;
// 	    }
// 	}
//     }
//     if (win != -1) {
// 	sprintf(msg,"Best human player: %s", Players[win]->name);
// 	Set_message(msg);
// 	xpprintf("%s\n", msg);
//     }
//     if (loose != -1 && loose != win) {
// 	sprintf(msg,"Worst human player: %s", Players[loose]->name);
// 	Set_message(msg);
// 	xpprintf("%s\n", msg);
//     }
// }


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

static void Check_server_versions()
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

/* $Id: frame.c,v 5.42 2002/05/13 20:38:10 bertg Exp $
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

//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <math.h>
//#include <errno.h>
//#include <time.h>
//#include <limits.h>
//#include <sys/types.h>
//
//#ifndef _WINDOWS
//# include <unistd.h>
//# include <sys/param.h>
//#endif
//
//#ifdef _WINDOWS
//# include "NT/winServer.h"
//#endif
//
//#define SERVER
//#include "version.h"
//#include "config.h"
//#include "serverconst.h"
//#include "global.h"
//#include "proto.h"
//#include "bit.h"
//#include "netserver.h"
//#include "saudio.h"
//#include "error.h"
//#include "commonproto.h"


char frame_version[] = VERSION;


#define MAX_SHUFFLE_INDEX	65535
#define MAX_VISIBLE_OBJECTS	maxVisibleObject


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
long			frame_loops = 1;
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

// static int block_inview(block_visibility_t *bv, int x, int y)
// {
//     return ((x > bv->world.x && x < bv->world.x + horizontal_blocks)
// 	    || (x > bv->realWorld.x && x < bv->realWorld.x + horizontal_blocks))
// 	&& ((y > bv->world.y && y < bv->world.y + vertical_blocks)
// 	    || (y > bv->realWorld.y && y < bv->realWorld.y + vertical_blocks));
// }

#define DEBRIS_STORE(xd,yd,color,offset) \
    int			i;						  \
    if (xd < 0) {							  \
	xd += World.width;						  \
    }									  \
    if (yd < 0) {							  \
	yd += World.height;						  \
    }									  \
    if ((unsigned) xd >= (unsigned)view_width || (unsigned) yd >= (unsigned)view_height) {	  \
	/*								  \
	 * There's some rounding error or so somewhere.			  \
	 * Should be possible to resolve it.				  \
	 */								  \
	return;								  \
    }									  \
									  \
    i = offset + color * debris_areas					  \
	+ (((yd >> 8) % debris_y_areas) * debris_x_areas)		  \
	+ ((xd >> 8) % debris_x_areas);					  \
									  \
    if (num_ >= 255) {							  \
	return;								  \
    }									  \
    if (num_ >= max_) {							  \
	if (num_ == 0) {						  \
	    ptr_ = (debris_t *) malloc((max_ = 16) * sizeof(*ptr_));	  \
	} else {							  \
	    ptr_ = (debris_t *) realloc(ptr_, (max_ += max_) * sizeof(*ptr_)); \
	}								  \
	if (ptr_ == 0) {						  \
	    error("No memory for debris");				  \
	    num_ = 0;							  \
	    return;							  \
	}								  \
    }									  \
    ptr_[num_].x = (unsigned char) xd;					  \
    ptr_[num_].y = (unsigned char) yd;					  \
    num_++;

static void fastshot_store(int xf, int yf, int color, int offset)
{
#define ptr_		(fastshot_ptr[i])
#define num_		(fastshot_num[i])
#define max_		(fastshot_max[i])
    DEBRIS_STORE(xf, yf, color, offset);
#undef ptr_
#undef num_
#undef max_
}

static void debris_store(int xf, int yf, int color)
{
#define ptr_		(debris_ptr[i])
#define num_		(debris_num[i])
#define max_		(debris_max[i])
    DEBRIS_STORE(xf, yf, color, 0);
#undef ptr_
#undef num_
#undef max_
}

static void fastshot_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES * 2; i++) {
	if (fastshot_num[i] != 0) {
	    Send_fastshot(conn, i,
			  (unsigned char *) fastshot_ptr[i],
			  fastshot_num[i]);
	    fastshot_num[i] = 0;
	}
    }
}

static void debris_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES; i++) {
	if (debris_num[i] != 0) {
	    Send_debris(conn, i,
			(unsigned char *) debris_ptr[i],
			debris_num[i]);
	    debris_num[i] = 0;
	}
    }
}

static void Frame_radar_buffer_reset()
{
    num_radar = 0;
}

static void Frame_radar_buffer_add(int x, int y, int s)
{
    radar_t		*p;

    EXPAND(radar_ptr, num_radar, max_radar, radar_t, 1);
    p = &radar_ptr[num_radar++];
    p->x = x;
    p->y = y;
    p->size = s;
}

static void Frame_radar_buffer_send(int conn)
{
    int			i;
    int			dest;
    int			tmp;
    radar_t		*p;
    const int		radar_width = 256;
    int			radar_height = (radar_width * World.y) / World.x;
    int			radar_x;
    int			radar_y;
    int			send_x;
    int			send_y;
    shuffle_t		*radar_shuffle;
    size_t		shuffle_bufsize;

    if (num_radar > MIN(256, MAX_SHUFFLE_INDEX)) {
	num_radar = MIN(256, MAX_SHUFFLE_INDEX);
    }
    shuffle_bufsize = (num_radar * sizeof(shuffle_t));
    radar_shuffle = (shuffle_t *) malloc(shuffle_bufsize);
    if (radar_shuffle == (shuffle_t *) NULL) {
	return;
    }
    for (i = 0; i < num_radar; i++) {
	radar_shuffle[i] = i;
    }
    /* permute. */
    for (i = 0; i < num_radar; i++) {
	dest = (int)(rfrac() * num_radar);
	tmp = radar_shuffle[i];
	radar_shuffle[i] = radar_shuffle[dest];
	radar_shuffle[dest] = tmp;
    }

    if (Get_conn_version(conn) <= 0x4400) {
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    send_x = (World.width * radar_x) / radar_width;
	    send_y = (World.height * radar_y) / radar_height;
	    Send_radar(conn, send_x, send_y, p->size);
	}
    }
    else {
	unsigned char buf[3*256];
	int buf_index = 0;
	int fast_count = 0;

	if (num_radar > 256) {
	    num_radar = 256;
	}
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    if (radar_y >= 1024) {
		continue;
	    }
	    buf[buf_index++] = (unsigned char)(radar_x);
	    buf[buf_index++] = (unsigned char)(radar_y & 0xFF);
	    buf[buf_index] = (unsigned char)((radar_y >> 2) & 0xC0);
	    if (p->size & 0x80) {
		buf[buf_index] |= (unsigned char)(0x20);
	    }
	    buf[buf_index] |= (unsigned char)(p->size & 0x07);
	    buf_index++;
	    fast_count++;
	}
	if (fast_count > 0) {
	    Send_fastradar(conn, buf, fast_count);
	}
    }

    free(radar_shuffle);
}

static void Frame_radar_buffer_free()
{
    free(radar_ptr);
    radar_ptr = NULL;
    num_radar = 0;
    max_radar = 0;
}


/*
 * Fast conversion of `num' into `str' starting at position `i', returns
 * index of character after converted number.
 */
static int num2str(int num, char *str, int i)
{
    int	digits, t;

    if (num < 0) {
	str[i++] = '-';
	num = -num;
    }
    if (num < 10) {
	str[i++] = '0' + num;
	return i;
    }
    for (t = num, digits = 0; t; t /= 10, digits++)
	;
    for (t = i+digits-1; t >= 0; t--) {
	str[t] = num % 10;
	num /= 10;
    }
    return i + digits;
}

static int Frame_status(int conn, int ind)
{
    static char		mods[MAX_CHARS];
    player		*pl = Players[ind];
    int			n,
			lock_ind,
			lock_id = NO_ID,
			lock_dist = 0,
			lock_dir = 0,
			i,
			showautopilot;

    /*
     * Don't make lock visible during this frame if;
     * 0) we are not player locked or compass is not on.
     * 1) we have limited visibility and the player is out of range.
     * 2) the player is invisible and he's not in our team.
     * 3) he's not actively playing.
     * 4) we have blind mode and he's not on the visible screen.
     * 5) his distance is zero.
     */

    CLR_BIT(pl->lock.tagged, LOCK_VISIBLE);
    if (BIT(pl->lock.tagged, LOCK_PLAYER) && BIT(pl->used, HAS_COMPASS)) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee || OWNS_TANK(ind, lock_ind) || TEAM(ind, lock_ind) || ALLIANCE(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
		|| inview(Players[lock_ind]->pos.x, Players[lock_ind]->pos.y))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = (int)Wrap_findDir((int)(Players[lock_ind]->pos.x - pl->pos.x),
				    (int)(Players[lock_ind]->pos.y - pl->pos.y));
	    lock_dist = (int)pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (frame_loops % 8) < 4);
    else if (BIT(pl->used, HAS_AUTOPILOT))
	showautopilot = (frame_loops % 8) < 4;
    else
	showautopilot = 0;

    /*
     * Don't forget to modify Receive_modifier_bank() in netserver.c
     */
    i = 0;
    if (BIT(pl->mods.nuclear, FULLNUCLEAR))
	mods[i++] = 'F';
    if (BIT(pl->mods.nuclear, NUCLEAR))
	mods[i++] = 'N';
    if (BIT(pl->mods.warhead, CLUSTER))
	mods[i++] = 'C';
    if (BIT(pl->mods.warhead, IMPLOSION))
	mods[i++] = 'I';
    if (pl->mods.velocity) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'V';
	i = num2str (pl->mods.velocity, mods, i);
    }
    if (pl->mods.mini) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'X';
	i = num2str (pl->mods.mini + 1, mods, i);
    }
    if (pl->mods.spread) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'Z';
	i = num2str (pl->mods.spread, mods, i);
    }
    if (pl->mods.power) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'B';
	i = num2str (pl->mods.power, mods, i);
    }
    if (pl->mods.laser) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'L';
	mods[i++] = (BIT(pl->mods.laser, STUN) ? 'S' : 'B');
    }
    mods[i] = '\0';
    n = Send_self(conn,
		  pl,
		  lock_id,
		  lock_dist,
		  lock_dir,
		  showautopilot,
		  Players[GetInd[Get_player_id(conn)]]->status,
		  mods);
    if (n <= 0) {
	return 0;
    }

    if (BIT(pl->used, HAS_EMERGENCY_THRUST))
	Send_thrusttime(conn,
			pl->emergency_thrust_left,
			pl->emergency_thrust_max);
    if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
	Send_shieldtime(conn,
			pl->emergency_shield_left,
			pl->emergency_shield_max);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(conn, pl->count);
    }
    if (BIT(pl->used, HAS_PHASING_DEVICE))
	Send_phasingtime(conn,
			 pl->phasing_left,
			 pl->phasing_max);
    if (ShutdownServer != -1) {
	Send_shutdown(conn, ShutdownServer, ShutdownDelay);
    }

    if (round_delay_send > 0) {
	Send_rounddelay(conn, round_delay, roundDelaySeconds * FPS);
    }

    return 1;
}

static void Frame_map(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i,
			k,
			x,
			y,
			conn_bit = (1 << conn);
    block_visibility_t	bv;
    const int		fuel_packet_size = 5;
    const int		cannon_packet_size = 5;
    const int		target_packet_size = 7;
    const int		wormhole_packet_size = 5;
    int			bytes_left = 2000;
    int			max_packet;
    int			packet_count;

    x = pl->pos.bx;
    y = pl->pos.by;
    bv.world.x = x - (horizontal_blocks >> 1);
    bv.world.y = y - (vertical_blocks >> 1);
    bv.realWorld = bv.world;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	if (bv.world.x < 0 && bv.world.x + horizontal_blocks < World.x) {
	    bv.world.x += World.x;
	}
	else if (bv.world.x > 0 && bv.world.x + horizontal_blocks > World.x) {
	    bv.realWorld.x -= World.x;
	}
	if (bv.world.y < 0 && bv.world.y + vertical_blocks < World.y) {
	    bv.world.y += World.y;
	}
	else if (bv.world.y > 0 && bv.world.y + vertical_blocks > World.y) {
	    bv.realWorld.y -= World.y;
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / target_packet_size);
    i = MAX(0, pl->last_target_update);
    for (k = 0; k < World.NumTargets; k++) {
	target_t *targ;
	if (++i >= World.NumTargets) {
	    i = 0;
	}
	targ = &World.targets[i];
	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& block_inview(&bv, targ->pos.x, targ->pos.y))) {
	    Send_target(conn, i, targ->dead_time, targ->damage);
	    pl->last_target_update = i;
	    bytes_left -= target_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / cannon_packet_size);
    i = MAX(0, pl->last_cannon_update);
    for (k = 0; k < World.NumCannons; k++) {
	if (++i >= World.NumCannons) {
	    i = 0;
	}
	if (block_inview(&bv,
			 World.cannon[i].blk_pos.x,
			 World.cannon[i].blk_pos.y)) {
	    if (BIT(World.cannon[i].conn_mask, conn_bit) == 0) {
		Send_cannon(conn, i, World.cannon[i].dead_time);
		pl->last_cannon_update = i;
		bytes_left -= max_packet * cannon_packet_size;
		if (++packet_count >= max_packet) {
		    break;
		}
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / fuel_packet_size);
    i = MAX(0, pl->last_fuel_update);
    for (k = 0; k < World.NumFuels; k++) {
	if (++i >= World.NumFuels) {
	    i = 0;
	}
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
	    if (World.block[World.fuel[i].blk_pos.x]
			   [World.fuel[i].blk_pos.y] == FUEL) {
		if (block_inview(&bv,
				 World.fuel[i].blk_pos.x,
				 World.fuel[i].blk_pos.y)) {
		    Send_fuel(conn, i, (int) World.fuel[i].fuel);
		    pl->last_fuel_update = i;
		    bytes_left -= max_packet * fuel_packet_size;
		    if (++packet_count >= max_packet) {
			break;
		    }
		}
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / wormhole_packet_size);
    i = MAX(0, pl->last_wormhole_update);
    for (k = 0; k < World.NumWormholes; k++) {
	wormhole_t *worm;
	if (++i >= World.NumWormholes) {
	    i = 0;
	}
	worm = &World.wormHoles[i];
	if (wormholeVisible
	    && worm->temporary
	    && (worm->type == WORM_IN
		|| worm->type == WORM_NORMAL)
	    && block_inview(&bv, worm->pos.x, worm->pos.y)) {
	    /* This is really a stupid bug: he first converts
	       the perfect blocksizes to pixels which the
	       client is perfectly capable of doing itself.
	       Then he sends the pixels in signed shorts.
	       This will fail on big maps. */
	    int	x = (worm->pos.x * BLOCK_SZ) + BLOCK_SZ / 2,
		y = (worm->pos.y * BLOCK_SZ) + BLOCK_SZ / 2;
	    Send_wormhole(conn, x, y);
	    pl->last_wormhole_update = i;
	    bytes_left -= max_packet * wormhole_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }
}


static void Frame_shuffle_objects()
{
    int				i;
    size_t			memsize;

    num_object_shuffle = MIN(NumObjs, MAX_VISIBLE_OBJECTS);

    if (max_object_shuffle < num_object_shuffle) {
	if (object_shuffle_ptr != NULL) {
	    free(object_shuffle_ptr);
	}
	max_object_shuffle = num_object_shuffle;
	memsize = max_object_shuffle * sizeof(shuffle_t);
	object_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (object_shuffle_ptr == NULL) {
	    max_object_shuffle = 0;
	}
    }

    if (max_object_shuffle < num_object_shuffle) {
	num_object_shuffle = max_object_shuffle;
    }

    for (i = 0; i < num_object_shuffle; i++) {
	object_shuffle_ptr[i] = i;
    }
    /* permute. */
    for (i = num_object_shuffle - 1; i >= 0; --i) {
	if (object_shuffle_ptr[i] == i) {
	    int j = (int)(rfrac() * i);
	    shuffle_t tmp = object_shuffle_ptr[j];
	    object_shuffle_ptr[j] = object_shuffle_ptr[i];
	    object_shuffle_ptr[i] = tmp;
	}
    }
}


static void Frame_shuffle_players()
{
    int				i;
    size_t			memsize;

    num_player_shuffle = MIN(NumPlayers, MAX_SHUFFLE_INDEX);

    if (max_player_shuffle < num_player_shuffle) {
	if (player_shuffle_ptr != NULL) {
	    free(player_shuffle_ptr);
	}
	max_player_shuffle = num_player_shuffle;
	memsize = max_player_shuffle * sizeof(shuffle_t);
	player_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (player_shuffle_ptr == NULL) {
	    max_player_shuffle = 0;
	}
    }

    if (max_player_shuffle < num_player_shuffle) {
	num_player_shuffle = max_player_shuffle;
    }

    for (i = 0; i < num_player_shuffle; i++) {
	player_shuffle_ptr[i] = i;
    }
    /* permute. */
    for (i = 0; i < num_player_shuffle; i++) {
	int j = (int)(rfrac() * num_player_shuffle);
	shuffle_t tmp = player_shuffle_ptr[j];
	player_shuffle_ptr[j] = player_shuffle_ptr[i];
	player_shuffle_ptr[i] = tmp;
    }
}


static void Frame_shuffle()
{
    if (last_frame_shuffle != frame_loops) {
	last_frame_shuffle = frame_loops;
	Frame_shuffle_objects();
	Frame_shuffle_players();
    }
}

static void Frame_shots(int conn, int ind)
{
    player			*pl = Players[ind];
    register int		x, y;
    int				i, k, color;
    int				fuzz = 0, teamshot, len;
    int				obj_count;
    object			*shot;
    object			**obj_list;
    int				hori_blocks, vert_blocks;

    hori_blocks = (view_width + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    vert_blocks = (view_height + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    Cell_get_objects(OBJ_X_IN_BLOCKS(pl), OBJ_Y_IN_BLOCKS(pl),
		     MAX(hori_blocks, vert_blocks), num_object_shuffle,
		     &obj_list,
		     &obj_count);
    for (k = 0; k < num_object_shuffle; k++) {
	i = object_shuffle_ptr[k];
	if (i >= obj_count) {
	    continue;
	}
	shot = obj_list[i];
	x = shot->pos.x;
	y = shot->pos.y;
	if (!inview(x, y)) {
	    continue;
	}
	if ((color = shot->color) == BLACK) {
	    xpprintf("black %d,%d\n", shot->type, shot->id);
	    color = WHITE;
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	    if ((fuzz >>= 7) < 0x40) {
		fuzz = randomMT();
	    }
	    if ((fuzz & 0x7F) >= spark_rand) {
		/*
		 * produce a sparkling effect by not displaying
		 * particles every frame.
		 */
		break;
	    }
	    /*
	     * The number of colors which the client
	     * uses for displaying debris is bigger than 2
	     * then the color used denotes the temperature
	     * of the debris particles.
	     * Higher color number means hotter debris.
	     */
	    if (debris_colors >= 3) {
		if (debris_colors > 4) {
		    if (color == BLUE) {
			color = (shot->life >> 1);
		    } else {
			color = (shot->life >> 2);
		    }
		} else {
		    if (color == BLUE) {
			color = (shot->life >> 2);
		    } else {
			color = (shot->life >> 3);
		    }
		}
		if (color >= debris_colors) {
		    color = debris_colors - 1;
		}
	    }

	    debris_store((int)(shot->pos.x - pv.world.x),
			 (int)(shot->pos.y - pv.world.y),
			 color);
	    break;

	case OBJ_WRECKAGE:
	    if (spark_rand != 0 || wreckageCollisionMayKill) {
		wireobject *wreck = WIRE_PTR(shot);
		Send_wreckage(conn, x, y, (u_byte)wreck->info,
			      wreck->size, wreck->rotation);
	    }
	    break;

	case OBJ_ASTEROID: {
		wireobject *ast = WIRE_PTR(shot);
		Send_asteroid(conn, x, y,
			      (u_byte)ast->info, ast->size, ast->rotation);
	    }
	    break;

	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    if (Team_immune(shot->id, pl->id)
		|| (shot->id != NO_ID
		    && BIT(Players[GetInd[shot->id]]->status, PAUSE))
		|| (shot->id == NO_ID
		    && BIT(World.rules->mode, TEAM_PLAY)
		    && shot->team == pl->team)) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->id == pl->id
		&& selfImmunity) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (frame_loops & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else {
		teamshot = 0;
	    }

	    fastshot_store((int)(shot->pos.x - pv.world.x),
			   (int)(shot->pos.y - pv.world.y),
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->missile_dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->missile_dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->missile_dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, x, y, shot->id);
	    break;
	case OBJ_MINE:
	    {
		int id = 0;
		int laid_by_team = 0;
		int confused = 0;
		mineobject *mine = MINE_PTR(shot);

		/* calculate whether ownership of mine can be determined */
		if (identifyMines
		    && (Wrap_length(pl->pos.x - mine->pos.x,
				    pl->pos.y - mine->pos.y)
			< (SHIP_SZ + MINE_SENSE_BASE_RANGE
			   + pl->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))) {
		    id = mine->id;
		    if (id == NO_ID)
			id = EXPIRED_MINE_ID;
		    if (BIT(mine->status, CONFUSED))
			confused = 1;
		}
		if (mine->id != NO_ID
		    && BIT(Players[GetInd[mine->id]]->status, PAUSE)) {
		    laid_by_team = 1;
		} else {
		    laid_by_team = (Team_immune(mine->id, pl->id)
				    || (BIT(mine->status, OWNERIMMUNE)
					&& mine->owner == pl->id));
		    if (confused) {
			id = 0;
			laid_by_team = (rfrac() < 0.5f);
		    }
		}
		Send_mine(conn, x, y, laid_by_team, id);
	    }
	    break;

	case OBJ_ITEM:
	    {
		int item_type = shot->info;

		if (BIT(shot->status, RANDOM_ITEM)) {
		    item_type = Choose_random_item();
		}

		Send_item(conn, x, y, item_type);
	    }
	    break;

	default:
	    error("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(int conn, int ind)
{
    player			*pl = Players[ind],
				*pl_i;
    pulse_t			*pulse;
    int				i, j, k, color, dir;
    DFLOAT			x, y;

    for (j = 0; j < NumPulses; j++) {
	pulse = Pulses[j];
	if (pulse->len <= 0) {
	    continue;
	}
	x = pulse->pos.x;
	y = pulse->pos.y;
	if (BIT (World.rules->mode, WRAP_PLAY)) {
	    if (x < 0) {
		x += World.width;
	    }
	    else if (x >= World.width) {
		x -= World.width;
	    }
	    if (y < 0) {
		y += World.height;
	    }
	    else if (y >= World.height) {
		y -= World.height;
	    }
	}
	if (inview(x, y)) {
	    dir = pulse->dir;
	} else {
	    x += tcos(pulse->dir) * pulse->len;
	    y += tsin(pulse->dir) * pulse->len;
	    if (BIT (World.rules->mode, WRAP_PLAY)) {
		if (x < 0) {
		    x += World.width;
		}
		else if (x >= World.width) {
		    x -= World.width;
		}
		if (y < 0) {
		    y += World.height;
		}
		else if (y >= World.height) {
		    y -= World.height;
		}
	    }
	    if (inview(x, y)) {
		dir = MOD2(pulse->dir + RES/2, RES);
	    }
	    else {
		continue;
	    }
	}
	if (Team_immune(pulse->id, pl->id)) {
	    color = BLUE;
	} else if (pulse->id == pl->id
	    && selfImmunity) {
	    color = BLUE;
	} else {
	    color = RED;
	}
	Send_laser(conn, color, (int)x, (int)y, pulse->len, dir);
    }
    for (i = 0; i < NumEcms; i++) {
	ecm_t *ecm = Ecms[i];
	Send_ecm(conn, (int)ecm->pos.x, (int)ecm->pos.y, ecm->size);
    }
    for (i = 0; i < NumTransporters; i++) {
	trans_t *trans = Transporters[i];
	player 	*victim = Players[GetInd[trans->target]],
		*pl = (trans->id == NO_ID ? NULL : Players[GetInd[trans->id]]);
	DFLOAT 	x = (pl ? pl->pos.x : trans->pos.x),
		y = (pl ? pl->pos.y : trans->pos.y);
	Send_trans(conn, victim->pos.x, victim->pos.y, (int)x, (int)y);
    }
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = World.cannon + i;
	if (cannon->tractor_count > 0) {
	    player *t = Players[GetInd[cannon->tractor_target]];
	    if (inview(t->pos.x, t->pos.y)) {
		int j;
		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   (int)(t->pos.x + t->ship->pts[j][t->dir].x),
				   (int)(t->pos.y + t->ship->pts[j][t->dir].y),
				   (int)cannon->pix_pos.x,
				   (int)cannon->pix_pos.y, 1);
		}
	    }
	}
    }

    for (k = 0; k < num_player_shuffle; k++) {
	i = player_shuffle_ptr[k];
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	if (!inview(pl_i->pos.x, pl_i->pos.y)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(conn,
			pl_i->pos.x,
			pl_i->pos.y,
			pl_i->count);
	    continue;
	}

	/* Don't transmit information if fighter is invisible */
	if (pl->visibility[i].canSee
	    || i == ind
	    || TEAM(i, ind)
	    || ALLIANCE(i, ind)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(conn,
		      pl_i->pos.x,
		      pl_i->pos.y,
		      pl_i->id,
		      pl_i->dir,
		      BIT(pl_i->used, HAS_SHIELD) != 0,
		      BIT(pl_i->used, HAS_CLOAKING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_EMERGENCY_SHIELD) != 0,
		      BIT(pl_i->used, HAS_PHASING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_DEFLECTOR) != 0
	    );
	}
	if (BIT(pl_i->used, HAS_REFUEL)) {
	    if (inview(World.fuel[pl_i->fs].pix_pos.x,
		       World.fuel[pl_i->fs].pix_pos.y)) {
		Send_refuel(conn,
			    (int)World.fuel[pl_i->fs].pix_pos.x,
			    (int)World.fuel[pl_i->fs].pix_pos.y,
			    pl_i->pos.x,
			    pl_i->pos.y);
	    }
	}
	if (BIT(pl_i->used, HAS_REPAIR)) {
	    DFLOAT x = (DFLOAT)(World.targets[pl_i->repair_target].pos.x + 0.5) * BLOCK_SZ;
	    DFLOAT y = (DFLOAT)(World.targets[pl_i->repair_target].pos.y + 0.5) * BLOCK_SZ;
	    if (inview(x, y)) {
		/* same packet as refuel */
		Send_refuel(conn, pl_i->pos.x, pl_i->pos.y, (int) x, (int) y);
	    }
	}
	if (BIT(pl_i->used, HAS_TRACTOR_BEAM)) {
	    player *t = Players[GetInd[pl_i->lock.pl_id]];
	    if (inview(t->pos.x, t->pos.y)) {
		int j;

		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   (int)(t->pos.x + t->ship->pts[j][t->dir].x),
				   (int)(t->pos.y + t->ship->pts[j][t->dir].y),
				   pl_i->pos.x,
				   pl_i->pos.y, 1);
		}
	    }
	}

	if (pl_i->ball != NULL
	    && inview(pl_i->ball->pos.x, pl_i->ball->pos.y)) {
	    Send_connector(conn,
			   pl_i->ball->pos.x,
			   pl_i->ball->pos.y,
			   pl_i->pos.x,
			   pl_i->pos.y, 0);
	}
    }
}

static void Frame_radar(int conn, int ind)
{
    int			i, k, mask, shownuke, size;
    player		*pl = Players[ind];
    object		*shot;
    DFLOAT		x, y;

    Frame_radar_buffer_reset();

#ifndef NO_SMART_MIS_RADAR
    if (nukesOnRadar) {
	mask = OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_MINE;
    } else {
	mask = (missilesOnRadar ?
		(OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT) : 0);
	mask |= (minesOnRadar) ? OBJ_MINE : 0;
    }
    if (treasuresOnRadar)
	mask |= OBJ_BALL;
    if (asteroidsOnRadar)
	mask |= OBJ_ASTEROID;

    if (mask) {
	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[i];
	    if (! BIT(shot->type, mask))
		continue;

	    shownuke = (nukesOnRadar && (shot)->mods.nuclear);
	    if (shownuke && (frame_loops & 2)) {
		size = 3;
	    } else {
		size = 0;
	    }

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (frame_loops % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		size = 2;
	    } else if (BIT(shot->type, OBJ_ASTEROID)) {
		size = WIRE_PTR(shot)->size + 1;
		size |= 0x80;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (frame_loops & 1)
		    continue;
	    }

	    x = shot->pos.x;
	    y = shot->pos.y;
	    if (Wrap_length(pl->pos.x - x,
			    pl->pos.y - y) <= pl->sensor_range) {
		Frame_radar_buffer_add((int)x, (int)y, size);
	    }
	}
    }
#endif

    if (playersOnRadar
	|| BIT(World.rules->mode, TEAM_PLAY)
	|| NumPseudoPlayers > 0
	|| NumAlliances > 0) {
	for (k = 0; k < num_player_shuffle; k++) {
	    i = player_shuffle_ptr[k];
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necessarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams or alliances if;
	     *			no playersOnRadar or if not visible
	     */
	    if (Players[i]->conn == conn
		|| BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
		|| (!TEAM(i, ind)
		    && !ALLIANCE(ind, i)
		    && !OWNS_TANK(ind, i)
		    && (!playersOnRadar || !pl->visibility[i].canSee))) {
		continue;
	    }
	    x = Players[i]->pos.x;
	    y = Players[i]->pos.y;
	    if (BIT(World.rules->mode, LIMITED_VISIBILITY)
		&& Wrap_length(pl->pos.x - x,
			       pl->pos.y - y) > pl->sensor_range) {
		continue;
	    }
	    if (BIT(pl->used, HAS_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd[pl->lock.pl_id] == i
		&& frame_loops % 5 >= 3) {
		continue;
	    }
	    size = 3;
	    if (TEAM(i, ind) || ALLIANCE(ind, i) || OWNS_TANK(ind, i)) {
		size |= 0x80;
	    }
	    Frame_radar_buffer_add((int)x, (int)y, size);
	}
    }

    Frame_radar_buffer_send(conn);
}

static void Frame_lose_item_state(int ind)
{
    player		*pl = Players[ind];

    if (pl->lose_item_state != 0) {
	Send_loseitem(pl->lose_item, pl->conn);
	if (pl->lose_item_state == 1)
	    pl->lose_item_state = -5;
	if (pl->lose_item_state < 0)
	    pl->lose_item_state++;
    }
}

static void Frame_parameters(int conn, int ind)
{
    player		*pl = Players[ind];

    Get_display_parameters(conn, &view_width, &view_height,
			   &debris_colors, &spark_rand);
    debris_x_areas = (view_width + 255) >> 8;
    debris_y_areas = (view_height + 255) >> 8;
    debris_areas = debris_x_areas * debris_y_areas;
    horizontal_blocks = (view_width + (BLOCK_SZ - 1)) / BLOCK_SZ;
    vertical_blocks = (view_height + (BLOCK_SZ - 1)) / BLOCK_SZ;

    pv.world.x = pl->pos.x - view_width / 2;	/* Scroll */
    pv.world.y = pl->pos.y - view_height / 2;
    pv.realWorld = pv.world;
    if (BIT (World.rules->mode, WRAP_PLAY)) {
	if (pv.world.x < 0 && pv.world.x + view_width < World.width) {
	    pv.world.x += World.width;
	}
	else if (pv.world.x > 0 && pv.world.x + view_width >= World.width) {
	    pv.realWorld.x -= World.width;
	}
	if (pv.world.y < 0 && pv.world.y + view_height < World.height) {
	    pv.world.y += World.height;
	}
	else if (pv.world.y > 0 && pv.world.y + view_height >= World.height) {
	    pv.realWorld.y -= World.height;
	}
    }
}

void Frame_update()
{
    int			i,
			conn,
			ind;
    player		*pl;
    time_t		newTimeLeft = 0;
    static time_t	oldTimeLeft;
    static bool		game_over_called = false;

    if (++frame_loops >= LONG_MAX)	/* Used for misc. timing purposes */
	frame_loops = 1;

    Frame_shuffle();

    if (gameDuration > 0.0
	&& game_over_called == false
	&& oldTimeLeft != (newTimeLeft = gameOverTime - time(NULL))) {
	/*
	 * Do this once a second.
	 */
	if (newTimeLeft <= 0) {
	    Game_Over();
	    ShutdownServer = 30 * FPS;	/* Shutdown in 30 seconds */
	    game_over_called = true;
	}
    }

    for (i = 0; i < num_player_shuffle; i++) {
	pl = Players[i];
	conn = pl->conn;
	if (conn == NOT_CONNECTED) {
	    continue;
	}
	if (BIT(pl->status, PAUSE|GAME_OVER)
	    && !allowViewing
	    && !pl->isowner) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     * Owner always gets full framerate even if paused.
	     * With allowViewing on, everyone gets full framerate.
	     */
	    if (BIT(pl->status, PAUSE)) {
		if (frame_loops & 0x03) {
		    continue;
		}
	    } else {
		if (frame_loops & 0x01) {
		    continue;
		}
	    }
	}

	/*
	* Reduce frame rate to player's own rate.
	*/
	if (pl->player_count > 0) {
	    pl->player_round++;
	    if (pl->player_round >= pl->player_count) {
		pl->player_round = 0;
		continue;
	    }
	}

	if (Send_start_of_frame(conn) == -1) {
	    continue;
	}
	if (newTimeLeft != oldTimeLeft) {
	    Send_time_left(conn, newTimeLeft);
	} else if (maxRoundTime > 0 && roundtime >= 0) {
	    Send_time_left(conn, (roundtime + FPS - 1) / FPS);
	}
	/*
	 * If status is GAME_OVER or PAUSE'd, the user may look through the
	 * other players 'eyes'.  If PAUSE'd this only works on team members.
	 * We can't use TEAM() macro as PAUSE'd players are always on
	 * equivalent teams.
	 *
	 * This is done by using two indexes, one
	 * determining which data should be used (ind, set below) and
	 * one determining which connection to send it to (conn).
	 */
	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    if ((BIT(pl->status, (GAME_OVER|PLAYING)) == (GAME_OVER|PLAYING))
		|| (BIT(pl->status, PAUSE) &&
		    ((BIT(World.rules->mode, TEAM_PLAY)
		      && pl->team != TEAM_NOT_SET
		      && pl->team == Players[GetInd[pl->lock.pl_id]]->team)
		    || pl->isowner
		    || allowViewing))) {
		ind = GetInd[pl->lock.pl_id];
	    } else {
		ind = i;
	    }
	} else {
	    ind = i;
	}
	if (Players[ind]->damaged > 0) {
	    Send_damaged(conn, Players[ind]->damaged);
	} else {
	    Frame_parameters(conn, ind);
      int Frame_status_result;
      Frame_status_result = Frame_status(conn, ind);
	    if (Frame_status_result <= 0) {
		continue;
	    }
	    Frame_map(conn, ind);
	    Frame_ships(conn, ind);
	    Frame_shots(conn, ind);
	    Frame_radar(conn, ind);
	    Frame_lose_item_state(i);
	    debris_end(conn);
	    fastshot_end(conn);
	}
	sound_play_queued(Players[ind]);
	Send_end_of_frame(conn);
    }
    oldTimeLeft = newTimeLeft;

    Frame_radar_buffer_free();
}

void Set_message(const char *message)
{
    player		*pl;
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	strlcpy(tmp, message, MSG_LEN);
	msg = tmp;
    } else {
	msg = message;
    }
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];
	if (pl->conn != NOT_CONNECTED) {
	    Send_message(pl->conn, msg);
	}
    }
}

void Set_player_message(player *pl, const char *message)
{
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	memcpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    if (pl->conn != NOT_CONNECTED) {
	Send_message(pl->conn, msg);
    }
    else if (IS_ROBOT_PTR(pl)) {
	Robot_message(GetInd[pl->id], msg);
    }
}

