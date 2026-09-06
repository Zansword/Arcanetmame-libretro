#include "state.h"
#include "retrofile.h"

void retro_poll_mame_input(void);

static int rtwi=320,rthe=240,topw=1024; // DEFAULT TEXW/TEXH/PITCH
int SHIFTON=-1;
char RPATH[512];
int RETRO_FATAL_ERROR=0;

extern int mmain(int argc, const char *argv);
extern bool draw_this_frame;

#if !defined(HAVE_OPENGL) && !defined(HAVE_OPENGLES) && !defined(HAVE_RGB32)
#define M16B
#endif

#ifdef M16B
	uint16_t videoBuffer[1024*1024];
	#define PITCH 1
#else
	unsigned int videoBuffer[1024*1024];
	#define PITCH 2*1
#endif 

retro_video_refresh_t video_cb = NULL;
retro_environment_t environ_cb = NULL;

const char *retro_save_directory;
const char *retro_system_directory;
const char *retro_content_directory;

retro_log_printf_t log_cb;

void retro_debug_log(const char *message)
{
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "%s", message);
   else
      printf("%s\n", message);
}

static retro_input_state_t input_state_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;

#define RETRO_PAD_DESCRIPTORS(port, player) \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, player " Coin" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  player " Start" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      player " Button 1" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      player " Button 2" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      player " Button 3" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      player " Button 4" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      player " Button 5" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      player " Button 6" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    player " Up" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  player " Down" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  player " Left" }, \
   { port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, player " Right" },

static const struct retro_input_descriptor input_descriptors[] = {
   RETRO_PAD_DESCRIPTORS(0, "P1")
   RETRO_PAD_DESCRIPTORS(1, "P2")
   RETRO_PAD_DESCRIPTORS(2, "P3")
   RETRO_PAD_DESCRIPTORS(3, "P4")
   RETRO_PAD_DESCRIPTORS(4, "P5")
   RETRO_PAD_DESCRIPTORS(5, "P6")
   RETRO_PAD_DESCRIPTORS(6, "P7")
   RETRO_PAD_DESCRIPTORS(7, "P8")
   { 0, 0, 0, 0, NULL }
};

#undef RETRO_PAD_DESCRIPTORS

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "retroogl.c"
#endif

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
static retro_input_poll_t input_poll_cb;

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "arcanetmame_mouse_enable", "Mouse support; disabled|enabled" },
      { "arcanetmame_cheat_enable", "Cheats; disabled|enabled" },
      { "arcanetmame_overclock", "Main CPU Overclock; Default|10|20|30|40|50|60|70|80|90|100|110|120|130|140|150|160|170|180|190|200|210|220|230|240|250|260|270|280|290|300|310|320|330|340|350|360|370|380|390|400" },
      { "arcanetmame_skip_gameinfo", "Hide game information screen (Restart); enabled|disabled" },
      { "arcanetmame_samples", "Use external samples (Restart); enabled|disabled" },
      { "arcanetmame_sample_rate", "Sample rate hint (Restart); 48000|44100|32000|22050" },
      { "arcanetmame_frameskip", "Frameskip (Restart); 0|1|2|3|4|5|6|7|8|9|10" },
      { "arcanetmame_brightness", "Screen brightness (Restart); 1.0|0.5|0.6|0.7|0.8|0.9|1.1|1.2|1.3|1.4|1.5|1.6|1.7|1.8|1.9|2.0" },
      { "arcanetmame_contrast", "Screen contrast (Restart); 1.0|0.5|0.6|0.7|0.8|0.9|1.1|1.2|1.3|1.4|1.5|1.6|1.7|1.8|1.9|2.0" },
      { "arcanetmame_gamma", "Screen gamma (Restart); 1.0|0.5|0.6|0.7|0.8|0.9|1.1|1.2|1.3|1.4|1.5|1.6|1.7|1.8|1.9|2.0|2.1|2.2|2.3|2.4|2.5|2.6|2.7|2.8|2.9|3.0" },
      { "arcanetmame_videoapproach1_enable", "Fixed 1024x768 rendering; disabled|enabled" },
      { NULL, NULL },
   };

   environ_cb = cb;

   if (environ_cb)
      environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

static void create_save_directories(void)
{
   static const char *save_directories[] = {
      "cfg", "nvram", "memcard", "input", "states", "snaps", "diff"
   };
   static const char *system_directories[] = {
      "samples", "artwork", "cheat", "ini"
   };
   char path[1024];
   int i;

   if (retro_save_directory != NULL)
   {
      sprintf(path, "%s%c%s", retro_save_directory, slash, core);
      retro_make_directory(path);
      for (i = 0; i < (int)(sizeof(save_directories) / sizeof(save_directories[0])); i++)
      {
         sprintf(path, "%s%c%s%c%s", retro_save_directory, slash, core, slash, save_directories[i]);
         retro_make_directory(path);
      }
   }

   if (retro_system_directory != NULL)
   {
      sprintf(path, "%s%c%s", retro_system_directory, slash, core);
      retro_make_directory(path);
      for (i = 0; i < (int)(sizeof(system_directories) / sizeof(system_directories[0])); i++)
      {
         sprintf(path, "%s%c%s%c%s", retro_system_directory, slash, core, slash, system_directories[i]);
         retro_make_directory(path);
      }
   }

   retro_debug_log("retro_init: MAME save and system directories prepared");
}

static void check_variables(void)
{
   struct retro_variable var = {0};
   var.key = "arcanetmame_mouse_enable";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         mouse_enable = false;
      if (!strcmp(var.value, "enabled"))
         mouse_enable = true;
   }

   var.key = "arcanetmame_skip_gameinfo";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      skip_gameinfo_enable = !strcmp(var.value, "enabled");

   var.key = "arcanetmame_samples";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      samples_enable = !strcmp(var.value, "enabled");

   var.key = "arcanetmame_sample_rate";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strncpy(sample_rate, var.value, sizeof(sample_rate) - 1);
      sample_rate[sizeof(sample_rate) - 1] = 0;
   }

   var.key = "arcanetmame_frameskip";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      frameskip_level = atoi(var.value);
      if (frameskip_level < 0)
         frameskip_level = 0;
      if (frameskip_level > 10)
         frameskip_level = 10;
   }

   var.key = "arcanetmame_brightness";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strncpy(screen_brightness, var.value, sizeof(screen_brightness) - 1);
      screen_brightness[sizeof(screen_brightness) - 1] = 0;
   }

   var.key = "arcanetmame_contrast";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strncpy(screen_contrast, var.value, sizeof(screen_contrast) - 1);
      screen_contrast[sizeof(screen_contrast) - 1] = 0;
   }

   var.key = "arcanetmame_gamma";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strncpy(screen_gamma, var.value, sizeof(screen_gamma) - 1);
      screen_gamma[sizeof(screen_gamma) - 1] = 0;
   }

   var.key = "arcanetmame_cheat_enable";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         cheats_enable = false;
      if (!strcmp(var.value, "enabled"))
         cheats_enable = true;
   }

   var.key = "arcanetmame_overclock";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Default"))
         overclock_percent = 0;
      else
      {
         overclock_percent = atoi(var.value);
         if (overclock_percent < 10)
            overclock_percent = 10;
         if (overclock_percent > 400)
         overclock_percent = 400;
      }
   }

   var.key = "arcanetmame_videoapproach1_enable";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         videoapproach1_enable = false;
      if (!strcmp(var.value, "enabled"))
         videoapproach1_enable = true;
   }

}

static void apply_overclock(void)
{
   running_machine *machine = retro_get_machine();

   if (overclock_percent > 0 && machine != NULL && machine->firstcpu != NULL)
      cpu_set_clockscale(machine->firstcpu, (float)overclock_percent / 100.0f);
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{   	
   memset(info, 0, sizeof(*info));
   info->library_name = "ArcanetMame";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   info->library_version = "0.135" GIT_VERSION;
   info->valid_extensions = "zip|chd|7z";
   info->need_fullpath = true;   
   info->block_extract = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   check_variables();

   info->geometry.base_width  = rtwi;
   info->geometry.base_height = rthe;

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "AV_INFO: width=%d height=%d\n",info->geometry.base_width,info->geometry.base_height);

   info->geometry.max_width    = 1024;
   info->geometry.max_height   = 768;

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "AV_INFO: max_width=%d max_height=%d\n",info->geometry.max_width,info->geometry.max_height);

   info->geometry.aspect_ratio = retro_aspect;

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "AV_INFO: aspect_ratio = %f\n",info->geometry.aspect_ratio);

   info->timing.fps            = retro_fps;
   info->timing.sample_rate    = 48000.0;

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "AV_INFO: fps = %f sample_rate = %f\n",info->timing.fps,info->timing.sample_rate);

}

void retro_init (void)
{
   struct retro_log_callback log;
   const char *system_dir  = NULL;
   const char *content_dir = NULL;
   const char *save_dir    = NULL;

   retro_system_directory = ".";
   retro_content_directory = ".";
   retro_save_directory = ".";

   if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir && *system_dir)
   {
      /* if defined, use the system directory */
      retro_system_directory = system_dir;
   }

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "SYSTEM_DIRECTORY: %s", retro_system_directory);

   if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY, &content_dir) && content_dir && *content_dir)
   {
      // if defined, use the system directory
      retro_content_directory=content_dir;
   }

   if (log_cb)
      log_cb(RETRO_LOG_INFO, "CONTENT_DIRECTORY: %s", retro_content_directory);


   if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
      /* If save directory is defined use it, 
       * otherwise use system directory. */
      retro_save_directory = *save_dir ? save_dir : retro_system_directory;

   }
   else
   {
      /* make retro_save_directory the same,
       * in case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY 
       * is not implemented by the frontend. */
      retro_save_directory=retro_system_directory;
   }
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "SAVE_DIRECTORY: %s", retro_save_directory);

   create_save_directories();
}

extern void retro_finish();
extern void retro_main_loop();

int RLOOP=1;
static int runfirst = 1;

void retro_deinit(void)
{
   if(retro_load_ok)
   {
      retro_debug_log("retro_deinit: saving and finishing machine");
      retro_finish();
   }
   retro_load_ok = false;
   runfirst = 1;
   LOGI("Retro DeInit\n");
}

void retro_reset (void)
{
   mame_reset = 1;
}


void retro_run (void)
{ 
   int result;
   bool updated = false;

   if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      check_variables();
      apply_overclock();
   }

   if(runfirst==1)
   {
 	runfirst++;
   retro_debug_log("retro_run: starting mmain");
   	result=mmain(1,RPATH);
   retro_debug_log("retro_run: mmain returned");
   	if(result!=1){
        	printf("Error: mame return an error\n");
		exit(0);
   	} 
        retro_load_ok  = true;
      apply_overclock();

	return;
   }

   if (NEWGAME_FROM_OSD == 1)
   {
      struct retro_system_av_info ninfo;

      retro_get_system_av_info(&ninfo);

      environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &ninfo);

      if (log_cb)
         log_cb(RETRO_LOG_INFO, "ChangeAV: w:%d h:%d ra:%f.\n",
               ninfo.geometry.base_width, ninfo.geometry.base_height, ninfo.geometry.aspect_ratio);

      NEWGAME_FROM_OSD=0;
   }

   if(RETRO_FATAL_ERROR==1){
	LOGI("MAME FATAL ERROR! exit now.\n");
	exit(0);
   }
   if (input_poll_cb && input_state_cb)
      retro_poll_mame_input();
   retro_debug_log("retro_run: entering retro_main_loop");
   retro_main_loop();
	retro_debug_log("retro_run: retro_main_loop returned");
	RLOOP=1;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	do_gl2d();
#else
      if (video_cb && draw_this_frame)
      		video_cb(videoBuffer,rtwi, rthe, topw << PITCH);
      else if (video_cb)
      		video_cb(NULL,rtwi, rthe, topw << PITCH); 
#endif
}

void prep_retro_rotation(int rot)
{
   LOGI("Rotation:%d\n",rot);
   if (environ_cb)
      environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rot);
}

/*
static void keyboard_cb(bool down, unsigned keycode, uint32_t character, uint16_t mod)
{
#ifdef KEYDBG
   printf( "Down: %s, Code: %d, Char: %u, Mod: %u. \n",
         down ? "yes" : "no", keycode, character, mod);
#endif
   if (keycode>=320);
   else
   {
      if(down && keycode==RETROK_LSHIFT)
      {
         SHIFTON=-SHIFTON;					
         if(SHIFTON == 1)
            retrokbd_state[keycode]=1;
         else
            retrokbd_state[keycode]=0;	
      }
      else if(keycode!=RETROK_LSHIFT)
      {
         if (down)
            retrokbd_state[keycode]=1;	
         else if (!down)
            retrokbd_state[keycode]=0;
      }
   }
}
*/
bool retro_load_game(const struct retro_game_info *info) 
{
   char basename[128];
   int result=0;
#if 0
   struct retro_keyboard_callback cb = { keyboard_cb };
   environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cb);
#endif

#ifndef M16B
   enum retro_pixel_format fmt =RETRO_PIXEL_FORMAT_XRGB8888;
#else
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#endif

   if (!info || !info->path || !*info->path)
      return false;

   if (!environ_cb || !environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "RGB pixel format is not supported.\n");
      exit(0);
   }

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, (void *)input_descriptors);
   check_variables();

#ifdef M16B
   memset(videoBuffer,0,1024*1024*2);
#else
   memset(videoBuffer,0,1024*1024*2*2);
#endif

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#ifdef HAVE_OPENGLES
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
#else
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
#endif
   hw_render.context_reset = context_reset;
   hw_render.context_destroy = context_destroy;

   if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;
#endif

   basename[0] = '\0';
   extract_basename(basename, info->path, sizeof(basename));
   extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));
   strcpy(RPATH,info->path);

   RETRO_FATAL_ERROR=0;

printf("Exit retro_load_game\n");
   return 1;
}

void retro_unload_game(void)
{
   if (retro_load_ok)
   {
      running_machine *machine = retro_get_machine();
      if (machine != NULL)
      {
         retro_debug_log("retro_unload_game: scheduling machine exit");
         mame_schedule_exit(machine);
      }
   }
   pauseg = -1;
   runfirst = 1;

	LOGI("Retro unload_game\n");	
}

size_t retro_serialize_size(void)
{
   running_machine *machine = retro_get_machine();
   return machine ? state_save_get_size(machine) : 0;
}

bool retro_serialize(void *data, size_t size)
{
   running_machine *machine = retro_get_machine();
   return machine != NULL && state_save_write_buffer(machine, data, size) == STATERR_NONE;
}

bool retro_unserialize(const void *data, size_t size)
{
   running_machine *machine = retro_get_machine();
   return machine != NULL && state_save_read_buffer(machine, data, size) == STATERR_NONE;
}

unsigned retro_get_region (void) {return RETRO_REGION_NTSC;}
void *retro_get_memory_data(unsigned type) {return 0;}
size_t retro_get_memory_size(unsigned type) {return 0;}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info){return false;}
void retro_cheat_reset(void){}
void retro_cheat_set(unsigned unused, bool unused1, const char* unused2){}
void retro_set_controller_port_device(unsigned in_port, unsigned device){}
