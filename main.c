#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"
#include "grid.h"

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  int grid_size_px;
} GFXContext;

void draw_cell(GFXContext* context, Cell* cell, int i, int j)
{
  SDL_Rect cell_rect = {i * 70, j * 70, 64, 64};
  SDL_Renderer* renderer = context->renderer;
  if (cell->fall_y > 0.f) {
	cell_rect.y -= 70 * cell->fall_y;
  }
  switch (cell->type) {
	case CELL_DIAMOND:
	  SDL_SetRenderDrawColor(renderer, 63, 255, 255, 255);
	  break;
	case CELL_EMERALD:
	  SDL_SetRenderDrawColor(renderer, 63, 255, 63, 255);
	  break;
	case CELL_RUBY:
	  SDL_SetRenderDrawColor(renderer, 255, 63, 63, 255);
	  break;
	case CELL_GRAPE:
	  SDL_SetRenderDrawColor(renderer, 255, 63, 255, 255);
	  break;
	case CELL_BANANA:
	  SDL_SetRenderDrawColor(renderer, 255, 255, 63, 255);
	  break;



	case CELL_VOID:
	default:
	  return;
  }
  SDL_RenderDrawRect(renderer, &cell_rect);
}

void draw_grid(GFXContext* context, Grid* grid)
{
  for (int i = 0; i < grid->width; ++i) {
	for (int j = grid->height - 1; j >= 0; --j) {
	  draw_cell(context, get_cell_or_null(grid, i, j), i, j);
	}
  }
}

void update_grid(Grid* grid, float dt)
{
  grid->stable = true;
  for (int i = 0; i < grid->width; ++i) {
	for (int j = 0; j < grid->height; ++j) {
	  Cell* cell = get_cell_or_null(grid, i, j);
	  if (cell->fall_y > 0) {
		grid->stable = false;
		cell->fall_vy += dt * 16.f;
		cell->fall_y -= cell->fall_vy * dt;
	  }
	  if (cell->fall_y < 0.f) {
		cell->fall_y = 0.f;
		cell->fall_vy = 0.f;
	  }
	}
  }
}

void handle_click(Grid* grid, int x, int y, SFXContext* sfx_context)
{
  int gridx = x / 70;
  int gridy = y / 70;
  if (gridx >= 0 && gridx < grid->width &&
	  gridy >= 0 && gridy < grid->height) {
	if (grid->stable) {
	  if (remove_cluster(grid, gridx, gridy)) {
		play_audio(sfx_context->effects.popped);
	  } else {
		play_audio(sfx_context->effects.cant_pop);
	  }
	}
  } else {
	play_audio(sfx_context->effects.cant_pop);
  }
}

bool game_loop(Grid* grid, GFXContext* gfx_context, SFXContext* sfx_context) // return true if it should continue
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
	switch (event.type) {
	  case SDL_QUIT:
		return false;
		break;
	  case SDL_MOUSEBUTTONDOWN:
		handle_click(grid, event.button.x, event.button.y, sfx_context);
		break;
	}
  }
  update_grid(grid, 0.04f);
  SDL_SetRenderDrawColor(gfx_context->renderer, 0, 0, 0, 255);
  SDL_RenderClear(gfx_context->renderer);
  draw_grid(gfx_context, grid);
  SDL_RenderPresent(gfx_context->renderer);
  return true;
}

int main()
{
  GFXContext gfx_context;
  SFXContext sfx_context;

  srand(1);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	perror(SDL_GetError());
	return -1;
  }
  Mix_Init(0);
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
	printf("SDL_mixer error!\n");
	return -1;
  }
  load_sfx(&sfx_context);
  SDL_CreateWindowAndRenderer(800, 600, 0, &gfx_context.window, &gfx_context.renderer);
  Grid* grid = newGrid(8, 8);

  bool running = true;

  int t0 = SDL_GetTicks(), t = t0, st = 0.f;
  while (running) {
	t = SDL_GetTicks();
	st += t - t0;
	t0 = t;
	while (st > 40) {
	  st -= 40;
	  running = game_loop(grid, &gfx_context, &sfx_context);
	}
  }

  delGrid(&grid);
  SDL_DestroyRenderer(gfx_context.renderer);
  SDL_DestroyWindow(gfx_context.window);
  clean_sfx(&sfx_context);
  Mix_CloseAudio();
  SDL_Quit();
  return 0;
}

