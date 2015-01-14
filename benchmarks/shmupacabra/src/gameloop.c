#include "camera.h"
#include "config.h"
#include "world.h"
#include "render.h"
#include "gameloop.h"
#include "audio.h"

Camera  *cam_list;      /* List of all cameras (sorted by "sort" value). */
Camera  *debug_cam;     /* Camera rendering debugging visuals. */

/* Keep track of last frame time. */
static uint32_t before;

/* FPS variables. */
static uint32_t fps_time;       /* Last FPS update time. */
static int fps_count;
static float frames_per_second;

/*
 * Game time is the time spent actually advancing (stepping) worlds. It can be
 * less than actual time (time since program start) due to lag or someone
 * suspending engine process.
 */
uint64_t game_time;

/*
 * run_game function with loop counters.
 */
void run_game_loop_counters(lua_State *L)
{
  int loop_counter[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t now = SDL_GetTicks();
  uint32_t delta_time = now >= before ? now - before : (((uint32_t) (-1)) - before) + now;
  before = now;
  if (delta_time > 50)
  {
    loop_counter[0]++;
    delta_time = 50;
  }

  uint32_t game_delta_time = delta_time;
  game_time += game_delta_time;
  fps_count++;
  if ((now - fps_time) >= config.FPSUpdateInterval)
  {
    loop_counter[1]++;
    frames_per_second = (fps_count * 1000.0) / (now - fps_time);
    fps_time = now;
    fps_count = 0;
  }

  process_events(L);
  extern mem_pool mp_world;
  for (World *world = mp_first(&mp_world); world != NULL; world = mp_next(world))
  {
    loop_counter[2]++;
    if (world->killme)
    {
      loop_counter[3]++;
      continue;
    }

    while (game_time >= world->next_step_time)
    {
      loop_counter[4]++;
      world->next_step_time += world->step_ms;
      world_step(world, L);
      process_events(L);
      if (world->killme)
      {
        loop_counter[5]++;
        break;
      }

    }

  }

  for (World *world = mp_first(&mp_world); world != NULL;)
  {
    loop_counter[6]++;
    if (world->killme)
    {
      loop_counter[7]++;
      World *tmp = world;
      world = mp_next(world);
      world_free(tmp);
      continue;
    }

    if (world->static_body.step == 0)
    {
      loop_counter[8]++;
      world_step(world, L);
    }

    world = mp_next(world);
  }

  render_clear();
  for (Camera *cam = cam_list; cam != NULL; cam = cam->next)
  {
    loop_counter[9]++;
    if (!cam->disabled)
    {
      loop_counter[10]++;
      render(cam);
    }

  }

  if ((debug_cam != NULL) && (debug_cam->objtype == OBJTYPE_CAMERA))
  {
    loop_counter[11]++;
    render_debug(debug_cam);
  }

  {
    printf("loop counter = (");
    int i;
    for (i = 0; i < 12; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}

/*
 * Original run_game function.
 */
void
run_game(lua_State *L)
{
        uint32_t now = SDL_GetTicks();   /* Current real time. */
        
        /*
         * Compute how much time has passed since last time. Watch out for time
         * wrap-around.
         */
        uint32_t delta_time = (now >= before) ? now - before :
        (uint32_t)-1 - before + now;
        before = now;
        
        /*
         * If there was some huge lag, don't make worlds catch
         * up. Instead, assume last frame took 50ms.
         */
        if (delta_time > 50)
                delta_time = 50;
        
        /* Game speed always normal. */
        uint32_t game_delta_time = delta_time;
        game_time += game_delta_time;	/* Advance game time. */
        
        /* Calculate frames per second. */
        fps_count++;
        if (now - fps_time >= config.FPSUpdateInterval) {
                frames_per_second = fps_count*1000.0 / (now - fps_time);
                fps_time = now;
                fps_count = 0;
                //log_msg("FPS: %.2f", frames_per_second);
        }
        
        process_events(L);
        
#if ENABLE_AUDIO
        /* Dynamically adjust volume. */
        audio_adjust_volume();
#endif
        
        /* Step worlds. */
        extern mem_pool mp_world;
        for (World *world = mp_first(&mp_world); world != NULL;
             world = mp_next(world)) {
                if (world->killme)
                        continue;       /* We deal with these below. */
                
                /* Bring world up to present game time. */
                while (game_time >= world->next_step_time) {
                        world->next_step_time += world->step_ms;
                        
                        /*
                         * Step world -- execute body step functions, timers,
                         * collision handlers.
                         */
                        world_step(world, L);
                        
                        /*
                         * Handle user input. To be more responsive, we do this
                         * here between steps too.
                         */
                        process_events(L);
                        
                        if (world->killme)
                                break;	/* No need to keep going. */
                }
        }
        
        /*
         * Deal with worlds that have either been destroyed or created in the
         * loop above. Must do this here so scripts get a chance to set
         * everything up before a frame is rendered.
         */
        for (World *world = mp_first(&mp_world); world != NULL;) {
                if (world->killme) {
                        /*
                         * Remove dying worlds. Take care to get next world
                         * pointer before destruction.
                         */
                        World *tmp = world;
                        world = mp_next(world);
                        world_free(tmp);
                        continue;
                }
                
                /* Perform the initial step on recently created worlds. */
                if (world->static_body.step == 0) {
                        /*
                         * We must give scripts control over what the contents
                         * of the world look like before drawing it. Otherwise
                         * we get such artifacts as camera centered on origin
                         * even though it should be tracking a player character.
                         */
                        world_step(world, L);
                }
                world = mp_next(world);
        }
        
        /*
         * Draw what each camera sees.
         */
        render_clear();
        for (Camera *cam = cam_list; cam != NULL; cam = cam->next) {
                if (!cam->disabled)
                        render(cam);
        }
        
#ifndef NDEBUG
        /* Debug stuff is drawn over normal stuff. */
        if (debug_cam != NULL && debug_cam->objtype == OBJTYPE_CAMERA)
                render_debug(debug_cam);
#endif
}
