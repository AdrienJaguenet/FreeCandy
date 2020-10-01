#pragma once

typedef enum {
  CELL_VOID = 0,
  CELL_DIAMOND = 1,
  CELL_EMERALD = 2,
  CELL_RUBY = 3,
  CELL_GRAPE = 4,
  CELL_BANANA = 5
} CellType;

typedef struct {
  CellType type;
  float fall_y;
  float fall_vy;

  // contiguous cells detection algorithm
  bool selected;
} Cell;

typedef struct {
  int width, height;
  Cell** cells;
  bool stable;
} Grid;

CellType random_cell_type();

Grid* newGrid(int width, int height);

void delGrid(Grid** grid);

Cell* get_cell_or_null(Grid* grid, int x, int y);

void get_moore_neighbours(Grid* grid, int x, int y, Cell** up, Cell** down, Cell** left, Cell** right);

int select_contiguous(Grid* grid, int src_x, int src_y);

bool remove_cluster(Grid* grid, int x, int y);

