#ifndef __SVG_H__
#define __SVG_H__

#define PX_PER_GRID 10

// how many extra grid cells on each side of the circuit
#define GRID_BORDER 5

void write_svg(std::string filename, rows_t& rows, channels_t& channels);

#endif
