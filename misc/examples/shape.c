// Demo of typesafe polymorphism in C99, using STC.

#include <stdlib.h>
#include <stdio.h>
#include <stc/ccommon.h>

#define DYN_CAST(T, s) \
    (&T##_api == (s)->api ? (T*)(s) : (T*)0)

// Shape definition
// ============================================================

typedef struct {
    float x, y;
} Point;

typedef struct Shape Shape;

struct ShapeAPI {
    void (*drop)(Shape*);
    void (*draw)(const Shape*);
};

struct Shape {
    struct ShapeAPI* api;
    uint32_t color;
    uint16_t style;
    uint8_t thickness;
    uint8_t hardness;
};

void Shape_drop(Shape* shape)
{
    printf("shape destructed\n");
}

void Shape_delete(Shape* shape)
{
    if (shape) {
        shape->api->drop(shape);
        c_free(shape);
    }
}

// Triangle implementation
// ============================================================

typedef struct {
    Shape shape;
    Point p[3];
} Triangle;

extern struct ShapeAPI Triangle_api;


Triangle Triangle_from(Point a, Point b, Point c) {
    Triangle t = {{&Triangle_api}, {a, b, c}};
    return t;
}

static void Triangle_draw(const Shape* shape)
{
    const Triangle* self = DYN_CAST(Triangle, shape);
    printf("Triangle : (%g,%g), (%g,%g), (%g,%g)\n",
           self->p[0].x, self->p[0].y,
           self->p[1].x, self->p[1].y,
           self->p[2].x, self->p[2].y);
}

struct ShapeAPI Triangle_api = {
    .drop = Shape_drop,
    .draw = Triangle_draw,
};

// Polygon implementation
// ============================================================

#define i_type PointVec 
#define i_val Point
#include <stc/cstack.h>

typedef struct {
    Shape shape;
    PointVec points;
} Polygon;

extern struct ShapeAPI Polygon_api;


Polygon Polygon_init(void) {
    Polygon p = {{&Polygon_api}, {0}};
    return p;
}

void Polygon_addPoint(Polygon* self, Point p)
{
    PointVec_push(&self->points, p);
}

static void Polygon_drop(Shape* shape)
{
    Polygon* self = DYN_CAST(Polygon, shape);
    printf("poly destructed\n");
    PointVec_drop(&self->points);
}

static void Polygon_draw(const Shape* shape)
{
    const Polygon* self = DYN_CAST(Polygon, shape);
    printf("Polygon  :");
    c_foreach (i, PointVec, self->points)
        printf(" (%g,%g)", i.ref->x, i.ref->y);
    puts("");
}

struct ShapeAPI Polygon_api = {
    .drop = Polygon_drop,
    .draw = Polygon_draw,
};

// Test
// ============================================================

#define i_type Shapes
#define i_val Shape*
#define i_valdrop(x) Shape_delete(*x)
#define i_no_clone
#include <stc/cstack.h>

void testShape(const Shape* shape)
{
    shape->api->draw(shape);
}


int main(void)
{
    Shapes shapes = {0};

    Triangle* tri1 = c_new(Triangle, Triangle_from((Point){5, 7}, (Point){12, 7}, (Point){12, 20}));
    Polygon* pol1 = c_new(Polygon, Polygon_init());
    Polygon* pol2 = c_new(Polygon, Polygon_init());

    c_forlist (i, Point, {{50, 72}, {123, 73}, {127, 201}, {828, 333}})
        Polygon_addPoint(pol1, *i.ref);

    c_forlist (i, Point, {{5, 7}, {12, 7}, {12, 20}, {82, 33}, {17, 56}})
        Polygon_addPoint(pol2, *i.ref);
    
    Shapes_push(&shapes, &tri1->shape);
    Shapes_push(&shapes, &pol1->shape);
    Shapes_push(&shapes, &pol2->shape);

    c_foreach (i, Shapes, shapes)
        testShape(*i.ref);

    Shapes_drop(&shapes);
}
