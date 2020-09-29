#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  CELL_VOID = 0,
  CELL_DIAMOND = 1,
  CELL_EMERALD = 2,
  CELL_RUBY = 3
} CellType;

typedef struct {
  CellType type;
  float fall_y;
  bool falling;

  // contiguous cells detection algorithm
  bool selected;
} Cell;

typedef struct {
  int width, height;
  Cell** cells;
  bool stable;
} Grid;

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  int grid_size_px;
} GFXContext;

CellType random_cell_type()
{
  switch (rand() % 3) {
	case 0:
	  return CELL_DIAMOND;
	case 1:
	  return CELL_EMERALD;
	case 2:
	  return CELL_RUBY;
	default:
	  return CELL_VOID;
  }
}

Grid* newGrid(int width, int height)
{
  Grid* grid = malloc(sizeof(Grid));
  grid->width = width;
  grid->height = height;
  grid->stable = false;
  grid->cells = malloc(sizeof(Cell*) * width);
  for (int i = 0; i < width; ++i) {
	grid->cells[i] = malloc(sizeof(Cell) * height);
	for (int j = 0; j < height; ++j) {
	  grid->cells[i][j].type = random_cell_type();
	  grid->cells[i][j].falling = false;
	  grid->cells[i][j].fall_y = 0.f;
	}
  }
  return grid;
}

void delGrid(Grid** grid) {
  for (int i = 0; i < (*grid)->width; ++i) {
	free((*grid)->cells[i]);
  }
  free((*grid)->cells);
  free(*grid);
  *grid = NULL;
}

void draw_cell(GFXContext* context, Cell* cell, int i, int j)
{
  SDL_Rect cell_rect = {i * 70, j * 70, 64, 64};
  SDL_Renderer* renderer = context->renderer;
  if (cell->falling) {
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

	case CELL_VOID:
	default:
	  return;
  }
  SDL_RenderDrawRect(renderer, &cell_rect);
}

Cell* get_up_or_null(Grid* grid, int x, int y)
{
  if (x > 0) {
	return &grid->cells[x][y - 1];
  } else {
	return NULL;
  }
}

void get_moore_neighbours(Grid* grid, int x, int y, Cell** up, Cell** down, Cell** left, Cell** right)
{
  if (x > 0) {
	*left = &grid->cells[x - 1][y];
  } else {
	*left = NULL;
  }

  if (x < grid->width - 1) {
	*right = &grid->cells[x + 1][y];
  } else {
	*right = NULL;
  }

  *up = get_up_or_null(grid, x, y);

  if (y < grid->height- 1) {
	*down = &grid->cells[x][y + 1];
  } else {
	*down = NULL;
  }
}

void select_contiguous(Grid* grid, int src_x, int src_y)
{
  Cell *src = &grid->cells[src_x][src_y];
  Cell *up, *down, *left, *right;
  get_moore_neighbours(grid, src_x, src_y, &up, &down, &left, &right);
  src->selected = true;

  if (up && !up->selected && up->type == src->type) {
	select_contiguous(grid, src_x, src_y - 1);
  }
  if (down && !down->selected && down->type == src->type) {
	select_contiguous(grid, src_x, src_y + 1);
  }
  if (left && !left->selected && left->type == src->type) {
	select_contiguous(grid, src_x - 1, src_y);
  }
  if (right && !right->selected && right->type == src->type) {
	select_contiguous(grid, src_x + 1, src_y);
  }
}

void remove_cell(Grid* grid, int x, int y)
{
  Cell* this = &grid->cells[x][y];
  this->falling = true;
  this->fall_y = 1.f;
  for (int i = y - 1; i >= 0; --i) {
	Cell* other = &grid->cells[x][i];
	if (other->type != CELL_VOID) {
	  this->fall_y = (float) y - i;
	  this->type = other->type;
	  remove_cell(grid, x, i);
	  return;
	}
  }
  this->type = random_cell_type();
}

void draw_grid(GFXContext* context, Grid* grid)
{
  for (int i = 0; i < grid->width; ++i) {
	for (int j = 0; j < grid->height; ++j) {
	  draw_cell(context, &grid->cells[i][j], i, j);
	}
  }
}

void update_grid(Grid* grid, float dt)
{
  grid->stable = true;
  for (int i = 0; i < grid->width; ++i) {
	for (int j = 0; j < grid->height; ++j) {
	  Cell* cell = &grid->cells[i][j];
	  if (cell->falling) {
		grid->stable = false;
		cell->fall_y -= dt;
	  }
	  if (cell->fall_y < 0.f) {
		cell->falling = false;
		cell->fall_y = 0.f;
	  }
	}
  }
}

void handle_click(Grid* grid, int x, int y)
{
  int gridx = x / 70;
  int gridy = y / 70;
  if (grid->stable &&
	  gridx >= 0 && gridx < grid->width &&
	  gridy >= 0 && gridy < grid->height) {
	// mark all cells as unselected
	for (int i = 0; i < grid->width; ++i) {
	  for (int j = 0; j < grid->height; ++j) {
		grid->cells[i][j].selected = false;
	  }
	}
	// select cells next to the clicked one
	select_contiguous(grid, gridx, gridy);

	// delete selected cells
	for (int i = 0; i < grid->width; ++i) {
	  for (int j = 0; j < grid->height; ++j) {
		if (grid->cells[i][j].selected) {
		  grid->cells[i][j].type = CELL_VOID;
		}
	  }
	}
	for (int i = 0; i < grid->width; ++i) {
	  for (int j = 0; j < grid->height; ++j) {
		if (grid->cells[i][j].selected) {
		  remove_cell(grid, i, j);
		}
	  }
	}
  }
}

int main()
{
  srand(0);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	perror(SDL_GetError());
	return -1;
  }
  GFXContext context;
  SDL_CreateWindowAndRenderer(800, 600, 0, &context.window, &context.renderer);
  Grid* grid = newGrid(8, 8);

  bool running = true;
  SDL_Event event;
  while (running) {
	while (SDL_PollEvent(&event)) {
	  switch (event.type) {
		case SDL_QUIT:
		  running = false;
		  break;
		case SDL_MOUSEBUTTONDOWN:
		  handle_click(grid, event.button.x, event.button.y);
		  break;
	  }
	}

	update_grid(grid, 0.002f);
	SDL_SetRenderDrawColor(context.renderer, 0, 0, 0, 255);
	SDL_RenderClear(context.renderer);
	draw_grid(&context, grid);
	SDL_RenderPresent(context.renderer);
  }

  delGrid(&grid);
  SDL_DestroyRenderer(context.renderer);
  SDL_DestroyWindow(context.window);
  SDL_Quit();
  return 0;
}

