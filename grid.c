#include <stdlib.h>
#include <stdbool.h>

#include "grid.h"

CellType random_cell_type()
{
  switch (rand() % 5) {
	case 0:
	  return CELL_DIAMOND;
	case 1:
	  return CELL_EMERALD;
	case 2:
	  return CELL_RUBY;
	case 3:
	  return CELL_GRAPE;
	case 4:
	  return CELL_BANANA;
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
	  grid->cells[i][j].fall_y = 0.f;
	  grid->cells[i][j].fall_vy = 0.f;
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

Cell* get_cell_or_null(Grid* grid, int x, int y)
{
  if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
	return &grid->cells[x][y];
  } else {
	return NULL;
  }
}

void get_moore_neighbours(Grid* grid, int x, int y, Cell** up, Cell** down, Cell** left, Cell** right)
{
  *left = get_cell_or_null(grid, x - 1, y);
  *right = get_cell_or_null(grid, x + 1, y);
  *up = get_cell_or_null(grid, x, y - 1);
  *down = get_cell_or_null(grid, x, y + 1);
}

int select_contiguous(Grid* grid, int src_x, int src_y)
{
  int acc = 1;
  Cell *src = get_cell_or_null(grid, src_x, src_y);
  Cell *up, *down, *left, *right;
  get_moore_neighbours(grid, src_x, src_y, &up, &down, &left, &right);
  src->selected = true;

  if (up && !up->selected && up->type == src->type) {
	acc += select_contiguous(grid, src_x, src_y - 1);
  }
  if (down && !down->selected && down->type == src->type) {
	acc += select_contiguous(grid, src_x, src_y + 1);
  }
  if (left && !left->selected && left->type == src->type) {
	acc += select_contiguous(grid, src_x - 1, src_y);
  }
  if (right && !right->selected && right->type == src->type) {
	acc += select_contiguous(grid, src_x + 1, src_y);
  }
  return acc;
}

// returns true if a cluster has been removed
bool remove_cluster(Grid* grid, int x, int y)
{
	// mark all cells as unselected
	for (int i = 0; i < grid->width; ++i) {
	  for (int j = 0; j < grid->height; ++j) {
		get_cell_or_null(grid, i, j)->selected = false;
	  }
	}
	// select cells next to the clicked one
	int cluster_size = select_contiguous(grid, x, y);
	if (cluster_size < 3) {
	  return false;
	}

	// delete selected cells
	int n_removed = 0;
	for (int i = 0; i < grid->width; ++i) {
	  for (int j = 0; j < grid->height; ++j) {
		Cell* this = get_cell_or_null(grid, i, j);
		if (this->selected) {
		  this->type = CELL_VOID;
		  ++ n_removed;
		}
	  }
	}

	for (int i = 0; i < grid->width; ++i) {
	  // We start from second-to-last cell
	  for (int j = grid->height - 2; j >= 0; --j) {
		Cell* this = get_cell_or_null(grid, i, j);
		Cell* below = get_cell_or_null(grid, i, j + 1);
		// Will it fall ?
		if (this->type != CELL_VOID && below->type == CELL_VOID) {
		  // Find the next non-empty cell, or the bottom
		  int k = grid->height - 1;
		  while (k > j) {
			below = get_cell_or_null(grid, i, k);
			if (below->type == CELL_VOID) {
			  break;
			}
			--k;
		  }

		  below->fall_y = (float) (k - j);
		  below->type = this->type;
		  this->type = CELL_VOID;
		}
	  }
	  // All cells at the top should be void
	  // Count them all
	  int n_void = 0;
	  while (n_void < grid->height && get_cell_or_null(grid, i, n_void)->type == CELL_VOID) {
		++n_void;
	  }
	  for (int j = 0; j < n_void; ++j) {
		Cell* this = get_cell_or_null(grid, i, j);
		this->fall_y = (float) n_void;
		this->type = random_cell_type();
	  }
	}
	return true;
}




