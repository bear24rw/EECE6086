#ifndef __POINT_H__
#define __POINT_H__

typedef struct point_t point_t;

struct point_t{
    int x, y;

    point_t() {}

    point_t(int _x, int _y) {
        x = _x;
        y = _y;
    }

    point_t operator+(const point_t p) {
        return point_t(x+p.x, y+p.y);
    }

    point_t operator-(const point_t p) {
        return point_t(x-p.x, y-p.y);
    }
};

#endif
