#include <vector>
#include <math.h>
#include "place.h"
#include "main.h"

rows_t place(std::vector<cell_t>& cells)
{
    int grid_w = ceil(sqrt(cells.size()));
    int grid_h = ceil(sqrt(cells.size()));

    rows_t rows(grid_h);

    for (int i=0; i<cells.size(); i++) {
        rows[i/grid_w].push_back(&cells[i]);
    }

    add_pass_throughs(rows);

    return rows;
}

void add_pass_throughs(rows_t& rows)
{
    for (auto &row : rows) {
        for (auto &cell : row) {
            // for each terminal in this cell
            //      get cell and terminal of destination
            //      if cell 
        }
    }
}
