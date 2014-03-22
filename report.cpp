#include "main.h"
#include "place.h"
#include "report.h"

void write_report(rows_t& rows, channels_t& channels)
{
    printf("Number of cell: %d\n", num_cells);
    printf("Number of nets: %d\n", num_nets);
    printf("\n");


    printf("Placement\n");
    printf("---------\n");

    point_t grid_size = calculate_grid_size();
    printf("Initial grid size: %d %d\n", grid_size.x, grid_size.y);

    int num_feed_throughs = 0;
    for (auto &row : rows) {
        for (auto &cell : row) {
            if (cell->feed_through)
                num_feed_throughs++;
        }
    }
    printf("Number of feed through cells: %d\n", num_feed_throughs);
    printf("\n");

    printf("Routing\n");
    printf("-------\n");

    int num_vias = 0;
    for (auto &row : rows) {
        for (auto &cell : row) {
            for (auto &term : cell->terms) {
                if (term.dest_term == nullptr) continue;
                if (term.track_num == VERTICAL) continue;
                if (term.track_num == UNROUTED) continue;
                num_vias++;
            }
        }
    }

    printf("Number of vias: %d\n", num_vias);

    int num_tracks = 0;
    for (auto &channel : channels) {
        num_tracks += channel.tracks.size();
    }

    printf("Total number of tracks: %d\n", num_tracks);
    printf("Total wirelength: %d\n", total_wire_length(channels));
    printf("\n");

    point_t size = bounding_box(rows, &channels);
    printf("Size\n");
    printf("----\n");
    printf("Total width: %d\n", size.x);
    printf("Total height: %d\n", size.y);
    printf("Total area: %d\n", size.x*size.y);
    printf("Squareness (width/height): %f\n", (double)size.x/(double)size.y);
    printf("\n");

    printf("Time\n");
    printf("----\n");
    printf("Place time: %fs\n", double(place_time - start_time) / CLOCKS_PER_SEC);
    printf("Route time: %fs\n", double(route_time - place_time) / CLOCKS_PER_SEC);
    printf("Total time: %fs\n", double(end_time   - start_time) / CLOCKS_PER_SEC);

}
