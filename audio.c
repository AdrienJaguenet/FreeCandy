#include "audio.h"

void play_audio(Mix_Chunk* sample)
{
  Mix_PlayChannel(-1, sample, 0);
}

void try_load_wav(Mix_Chunk** chunk, const char* path)
{
  *chunk = Mix_LoadWAV(path);
  if (*chunk == NULL) {
	printf("Could not load WAV at '%s'. Reason: '%s'.\n", path, Mix_GetError());
  }
}

void load_sfx(SFXContext* ctx)
{
  try_load_wav(&ctx->effects.popped, "sfx/pop.wav");
  try_load_wav(&ctx->effects.cant_pop, "sfx/err.wav");
}

void clean_sfx(SFXContext* ctx)
{
  Mix_FreeChunk(ctx->effects.popped);
  Mix_FreeChunk(ctx->effects.cant_pop);
}


