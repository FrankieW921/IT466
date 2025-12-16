#include <SDL.h>            
#include <SDL_mixer.h>

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_camera.h"
#include "gf3d_mesh.h"

#include "game.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "world.h"
#include "camera_entity.h"
#include "hud.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;
static Mix_Music* track;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();
void main_menu();

void exitGame()
{
    _done = 1;
}


int main(int argc,char *argv[])
{
    //local variables
    //Sprite *bg;
    float theta = 0;
    GFC_Vector3D cam = { 0,-45,15 };
    GFC_Vector3D lightPos = { -10, 0, 25 };

    Mesh* skybox;
    GFC_Matrix4 skyboxID;
    Texture* skyTexture;
    
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //audio
    gfc_sound_init_config("config/audio.cfg");
    Mix_VolumeMusic(16);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    entity_system_init(100);
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    //Sprite* bg = gf2d_sprite_load_image("images/bg_flat.png");
    gf2d_mouse_load("actors/mouse.actor");
    //testworld = world_load("defs/terrain/terrain1.def");
    world_load("defs/terrain/terrain1.def");
    gf3d_camera_look_at(gfc_vector3d(0, 0, 0), &cam);
    player_spawn(gfc_vector3d(0, 0, 1), GFC_COLOR_WHITE);
    camera_entity_new();

    skybox = gf3d_mesh_load("models/sky/sky.obj");
    skyTexture = gf3d_texture_load("models/sky/sky.png");
    gfc_matrix4_identity(skyboxID);
    init_start_buttons();

    main_menu();
    // main game loop    
    while(!_done)
    {
        gfc_input_update();
        SDL_GetKeyboardState(NULL);
        gf2d_mouse_update();
        gf2d_font_update();
        //camera updaes
        gf3d_camera_update_view();
        gf3d_vgraphics_render_start();
                //3d draws
                entity_think_all();
                entity_update_all();
                camera_think();

                gf3d_mesh_sky_draw(skybox, skyboxID, GFC_COLOR_WHITE, skyTexture);
                world_draw(get_the_world());
                entity_draw_all(get_the_world()->lightPosition, GFC_COLOR_WHITE);
                //2D draws
                //gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
                player_ui_draw();
                draw_all_huds();
                //gf2d_font_draw_line_tag("ALT+F4 to commit mecha epicness",FT_H4,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                gf2d_mouse_draw();
                world_think(get_the_world());
        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        if (gfc_input_command_down("mainMenu")) main_menu();
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}

void start_game(int stage) {
    if (track) Mix_FreeMusic(track);
    Mix_HaltMusic();
    track = Mix_LoadMUS("music/Battle-a2.mp3");
    if (track) {
        slog("TRYING TO PLAY MOOSIC");
        Mix_PlayMusic(track, -1);
    }
    else {
        slog("COULDNT PLAY THE GOOD STUFF");
    }

    if (stage == 1) {
        world_load("defs/terrain/terrain1.def");
    }
    else if (stage == 2) {
        world_load("defs/terrain/terrain2.def");
    }
    else if (stage == 3) {
        world_load("defs/terrain/terrain3.def");
    }
    else {
        slog("Invalid stage ID"); return;
    }

    player_spawn(gfc_vector3d(0, 0, 1), GFC_COLOR_WHITE);
    camera_entity_new();
    disable_start_menu();
}

void main_menu() {
    if (track) Mix_FreeMusic(track);
    Mix_HaltMusic();
    track = Mix_LoadMUS("music/AC4Atitle.mp3");
    if (track) {
        slog("TRYING TO PLAY MOOSIC");
        Mix_PlayMusic(track, -1);
    }
    else {
        slog("COULDNT PLAY THE GOOD STUFF");
    }

    enable_start_menu();
    camera_entity_free();
    world_free(get_the_world());
    entity_free(get_the_player());
}

void start_edit() {
    world_load("defs/terrain/editworld.def");
    editor_spawn(gfc_vector3d(0, 0, 1), GFC_COLOR_WHITE);
    camera_entity_new();
    disable_start_menu();
}

/*eol@eof*/
