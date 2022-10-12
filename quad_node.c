#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define QT_NODE_CAPACITY (4)
#define MAX_ARRAY_SIZE (1024)

#define WIN_SIZE 640

typedef struct {
    int x;
    int y;
} point;

point *point_new(int x, int y) {
    point *p = (point *)malloc(sizeof(point));
    p->x = x;
    p->y = y;
    return p;
}

void point_free(point *p) {
    free(p);
}

typedef struct {
    point *center;
    float half_dimension;
} AABB;

AABB *aabb_new(point *center, float half_dimension) {
    AABB *aabb = (AABB *)malloc(sizeof(AABB));
    aabb->center = center;
    aabb->half_dimension = half_dimension;
    return aabb;
}

void aabb_free(AABB *aabb) {
    free(&aabb->center);
    free(aabb);
}

int aabb_contains_point(AABB *aabb, point *p) {
    if (aabb->center->x + aabb->half_dimension) {

    }
    return 0;
}

int aabb_intersects_aabb() {
    return 0;
}

typedef struct quad_node {
    AABB *boundary;
    point **points;
    struct quad_node *north_west;
    struct quad_node *north_east;
    struct quad_node *south_west;
    struct quad_node *south_east;
} quad_tree;

quad_tree *quad_tree_new(AABB *boundary) {
    quad_tree *qt = (quad_tree *)malloc(sizeof(quad_tree));
    qt->boundary = boundary;
    qt->north_west = NULL;
    qt->north_east = NULL;
    qt->south_west = NULL;
    qt->south_east = NULL;

    return qt;
}

void quad_tree_draw_bounds(SDL_Renderer *r, quad_tree *qt) {
    SDL_RenderDrawLine(
        r, 
        qt->boundary->center->x, 
        qt->boundary->center->y, 
        qt->boundary->center->x + qt->boundary->half_dimension, 
        qt->boundary->center->y
    );
    SDL_RenderDrawLine(
        r, 
        qt->boundary->center->x, 
        qt->boundary->center->y, 
        qt->boundary->center->x - qt->boundary->half_dimension, 
        qt->boundary->center->y
    );
    SDL_RenderDrawLine(
        r, 
        qt->boundary->center->x, 
        qt->boundary->center->y, 
        qt->boundary->center->x, 
        qt->boundary->center->y - qt->boundary->half_dimension
    );
    SDL_RenderDrawLine(
        r, 
        qt->boundary->center->x, 
        qt->boundary->center->y, 
        qt->boundary->center->x, 
        qt->boundary->center->y + qt->boundary->half_dimension
    );
    if (qt->north_west != NULL) {
        quad_tree_draw_bounds(r, qt->north_west);
    }
    if (qt->north_east != NULL) {
        quad_tree_draw_bounds(r, qt->north_east);
    }
    if (qt->south_west != NULL) {
        quad_tree_draw_bounds(r, qt->south_west);
    }
    if (qt->south_east != NULL) {
        quad_tree_draw_bounds(r, qt->south_east);
    }
}

void quad_tree_free(quad_tree *qt) {
    free(qt->boundary);
    free(qt->north_west);
    free(qt->north_east);
    free(qt->south_west);
    free(qt->south_east);
    free(qt);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Dust simulation",  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_SIZE, WIN_SIZE, SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    point *center = point_new(WIN_SIZE / 2, WIN_SIZE / 2);
    AABB *aabb = aabb_new(center, WIN_SIZE / 2);
    quad_tree *qt = quad_tree_new(aabb);

    point *center2 = point_new(center->x / 2, center->y / 2);
    AABB *aabb2 = aabb_new(center2 , aabb->half_dimension / 2);
    qt->north_west = quad_tree_new(aabb2);

    point *center3 = point_new(center2->x / 2, center2->y / 2);
    AABB *aabb3 = aabb_new(center3 , aabb2->half_dimension / 2);
    qt->north_west->north_west = quad_tree_new(aabb3);

    int running = 1;
    SDL_Event event;
    while(running) {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDrawPoint(renderer, 25, 25);
        quad_tree_draw_bounds(renderer, qt);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderPresent(renderer);
    }

    quad_tree_free(qt);

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}