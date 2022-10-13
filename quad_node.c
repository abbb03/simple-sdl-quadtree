#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define QT_NODE_CAPACITY 4
#define QT_MAX_LEVEL 4

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
    free(aabb->center);
    free(aabb);
}

int aabb_contains_point(AABB *aabb, point *p) {
    if (p->x < aabb->center->x - aabb->half_dimension || p->x > aabb->center->x + aabb->half_dimension) {
        return 0;
    }
    if (p->y < aabb->center->y - aabb->half_dimension || p->y > aabb->center->y + aabb->half_dimension) {
        return 0;
    }
    return 1;
}

int aabb_intersects_aabb() {
    return 0;
}

typedef struct quad_node {
    int level;
    int curr_obj_index;
    AABB *boundary;
    point **points;
    struct quad_node *north_west;
    struct quad_node *north_east;
    struct quad_node *south_west;
    struct quad_node *south_east;
} quad_tree;

quad_tree *quad_tree_new(AABB *boundary, int level) {
    quad_tree *qt = (quad_tree *)malloc(sizeof(quad_tree));
    qt->level = level;
    qt->curr_obj_index = -1;
    qt->points = (point **)malloc(QT_NODE_CAPACITY * sizeof(point));
    for (int i = 0; i < QT_NODE_CAPACITY; i++) {
        qt->points[i] = NULL;
    }
    qt->boundary = boundary;
    qt->north_west = NULL;
    qt->north_east = NULL;
    qt->south_west = NULL;
    qt->south_east = NULL;

    return qt;
}

int quad_tree_count_objects(quad_tree *qt) {
    int i = 0;
    while (qt->points[i] != NULL && i < QT_NODE_CAPACITY) {
        i++;
    }

    return i;
}

quad_tree *quad_tree_subdivide(quad_tree *qt) {
    float half_dim = qt->boundary->half_dimension / 2;
    // North West
    point *nw_p = point_new(qt->boundary->center->x - half_dim, qt->boundary->center->y + half_dim);
    qt->north_west = quad_tree_new(aabb_new(nw_p, half_dim), 0);
    
    // North East
    point *ne_p = point_new(qt->boundary->center->x + half_dim, qt->boundary->center->y + half_dim);
    qt->north_east = quad_tree_new(aabb_new(ne_p, half_dim), 0);
    
    // South West
    point *sw_p = point_new(qt->boundary->center->x - half_dim, qt->boundary->center->y - half_dim);
    qt->south_west = quad_tree_new(aabb_new(sw_p, half_dim), 0);
    
    // South East
    point *se_p = point_new(qt->boundary->center->x + half_dim, qt->boundary->center->y - half_dim);
    qt->south_east = quad_tree_new(aabb_new(se_p, half_dim), 0);

    return qt;
}

int quad_tree_insert(quad_tree *qt, point *p) {
    if (!aabb_contains_point(qt->boundary, p)) {
        return 0;
    }

    if (++qt->curr_obj_index < QT_NODE_CAPACITY && qt->north_west == NULL) {
        qt->points[qt->curr_obj_index] = p;
        return 1;
    }

    if (qt->north_west == NULL) {
        quad_tree_subdivide(qt);
    }

    if (quad_tree_insert(qt->north_west, p)) {
        return 1;
    }
    if (quad_tree_insert(qt->north_east, p)) {
        return 1;
    }
    if (quad_tree_insert(qt->south_west, p)) {
        return 1;
    }
    if (quad_tree_insert(qt->south_east, p)) {
        return 1;
    }

    return 0;
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

void quad_tree_draw_points(SDL_Renderer *r, quad_tree *qt) {
    int count_objects = quad_tree_count_objects(qt);
    for (int i = 0; i < count_objects; i++) {
        SDL_RenderDrawPoint(r, qt->points[i]->x, qt->points[i]->y);
    }

    if (qt->north_west != NULL) {
        quad_tree_draw_points(r, qt->north_west);
    }
    if (qt->north_east != NULL) {
        quad_tree_draw_points(r, qt->north_west);
    }
    if (qt->south_west != NULL) {
        quad_tree_draw_points(r, qt->north_west);
    }
    if (qt->south_east != NULL) {
        quad_tree_draw_points(r, qt->north_west);
    }
}

void quad_tree_free(quad_tree *qt) {
    aabb_free(qt->boundary);
    for (int i = 0; i < QT_NODE_CAPACITY; i++) {
        free(qt->points[i]);
    }

    if (qt->north_west != NULL) {
        quad_tree_free(qt->north_west);
    }
    if (qt->north_east != NULL) {
        quad_tree_free(qt->north_east);
    }
    if (qt->south_west != NULL) {
        quad_tree_free(qt->south_west);
    }
    if (qt->south_east != NULL) {
        quad_tree_free(qt->south_east);
    }

    free(qt);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Dust simulation",  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_SIZE, WIN_SIZE, SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    point *center = point_new(WIN_SIZE / 2, WIN_SIZE / 2);
    AABB *aabb = aabb_new(center, WIN_SIZE / 2);
    quad_tree *qt = quad_tree_new(aabb, 0);

    int running = 1;
    SDL_Event event;
    while(running) {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_MOUSEBUTTONDOWN:
                printf ("MOUSE CLICK\n");
                int x, y;
                SDL_GetMouseState( &x, &y );
                if (SDL_MOUSEBUTTONDOWN) {
                    point *pp = (point *)malloc(sizeof(point));
                    pp->x = x;
                    pp->y = y;
                    quad_tree_insert(qt, pp);
                }
                break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        quad_tree_draw_points(renderer, qt);
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