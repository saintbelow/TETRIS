#ifndef TETRIS_H
#define TETRIS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/* ─────────────────────── CONSTANTS ─────────────────────── */
#define WINDOW_WIDTH       1000
#define WINDOW_HEIGHT      800
#define GRID_COLS          10
#define GRID_ROWS          20
#define CELL_SIZE          32.0f
#define GRID_OFFSET_X      300.0f
#define GRID_OFFSET_Y      50.0f

#define MAX_PARTICLES      2000
#define MAX_TRAIL_POINTS   50

#define NUM_TETROMINOS     7
#define TETRO_SIZE         4

#define PI 3.14159265358979323846f

#define NUM_THEMES         10

/* ─────────────────── GAME SCREENS ─────────────────────── */
typedef enum {
    SCREEN_MENU,
    SCREEN_PLAYING,
    SCREEN_HIGHSCORES,
    SCREEN_ABOUT
} GameScreen;

/* ─────────────────── DIFFICULTY ──────────────────────── */
#define NUM_DIFFICULTIES 4

typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD,
    DIFF_INSANE
} Difficulty;

/* ─────────────────── HIGH SCORES ──────────────────────── */
#define MAX_HIGH_SCORES  10
#define SAVE_MAGIC       0x4E454F4E  /* "NEON" */

typedef struct {
    int score;
    int lines;
    int level;
} HighScoreEntry;

typedef struct {
    HighScoreEntry entries[MAX_HIGH_SCORES];
    int count;
} HighScoreTable;

/* ─────────────────── SAVE DATA ────────────────────────── */
typedef struct {
    uint32_t magic;
    int grid[GRID_ROWS][GRID_COLS];
    int current_type, current_rotation, current_x, current_y;
    int next_type, held_type;
    bool can_hold;
    int score, lines_cleared, level, combo;
    double fall_interval;
    bool valid;  /* whether save data is present */
} SaveData;

/* ─────────────────── COLOR TYPE ─────────────────────────── */
typedef struct {
    float r, g, b, a;
} Color;

/* ─────────────────── THEME SYSTEM ─────────────────────── */
typedef struct {
    const char *name;
    const char *subtitle;
    Color pieces[NUM_TETROMINOS];
    Color bg_top, bg_bot;         /* Background gradient (top, bottom) */
    Color accent;                 /* BG grid line / scan tint */
    Color grid_line, grid_border; /* Grid styling */
    Color grid_bg;                /* Grid background fill */
    Color ui_primary;             /* Primary accent (titles) */
    Color ui_secondary;           /* Secondary accent */
    Color ui_text;                /* Default text */
    float glow_strength;          /* 0.0 – 1.0 */
    float bloom_strength;         /* 0.0 – 1.0 */
    float scanline_strength;      /* 0.0 – 1.0 */
} Theme;

/* Cyberpunk neon colors for each piece */
static const Color NEON_COLORS[NUM_TETROMINOS] = {
    {0.0f,  1.0f,  1.0f,  1.0f},   /* I - Cyan    */
    {1.0f,  1.0f,  0.0f,  1.0f},   /* O - Yellow  */
    {0.8f,  0.0f,  1.0f,  1.0f},   /* T - Purple  */
    {0.0f,  1.0f,  0.3f,  1.0f},   /* S - Green   */
    {1.0f,  0.1f,  0.3f,  1.0f},   /* Z - Red     */
    {0.0f,  0.5f,  1.0f,  1.0f},   /* J - Blue    */
    {1.0f,  0.5f,  0.0f,  1.0f},   /* L - Orange  */
};

/* ─────────────────── TETROMINO SHAPES ──────────────────── */
/* Each tetromino: 4 rotations x 4x4 grid */
static const int TETROMINOS[NUM_TETROMINOS][4][TETRO_SIZE][TETRO_SIZE] = {
    /* I */
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
    },
    /* O */
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
    },
    /* T */
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
    /* S */
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
    /* Z */
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},
    },
    /* J */
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}},
    },
    /* L */
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
};

/* ─────────────────── PARTICLE SYSTEM ───────────────────── */
typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    float size;
    Color color;
    bool active;
} Particle;

/* ─────────────────── GAME STATE ────────────────────────── */
typedef struct {
    int grid[GRID_ROWS][GRID_COLS];      /* 0 = empty, 1-7 = piece type */

    /* Current piece */
    int current_type;
    int current_rotation;
    int current_x, current_y;

    /* Next piece */
    int next_type;

    /* Held piece */
    int held_type;
    bool can_hold;

    /* Ghost piece Y */
    int ghost_y;

    /* Scoring */
    int score;
    int lines_cleared;
    int level;
    int combo;

    /* Timing */
    double fall_timer;
    double fall_interval;
    double lock_timer;
    bool lock_delay_active;

    /* Animation */
    float line_clear_anim[GRID_ROWS];   /* 0.0 = no anim, 1.0 = full anim */
    bool  line_clearing[GRID_ROWS];
    bool  animating_clear;
    double clear_timer;

    /* Game state */
    bool game_over;
    bool paused;
    double time;

    /* Particles */
    Particle particles[MAX_PARTICLES];
    int particle_count;

    /* Screen shake */
    float shake_intensity;
    float shake_timer;

    /* Glow pulse */
    float glow_pulse;

    /* Input */
    bool key_left, key_right, key_down;
    double das_timer;
    double arr_timer;
    bool das_active;

    /* Screen */
    GameScreen screen;

    /* Mouse */
    double mouse_x, mouse_y;
    bool mouse_clicked;

    /* High scores */
    HighScoreTable high_scores;

    /* Save state */
    bool has_save;

    /* Theme */
    int current_theme;

    /* Difficulty */
    Difficulty difficulty;

    /* 3D mode */
    bool mode_3d;
    float mode_3d_factor;          /* 0.0 = flat 2D, 1.0 = full 3D */
    float mode_transition_timer;   /* < 0 = idle, 0..1 = transitioning */
    float fog_alpha;               /* fog overlay during transition */

    /* Fullscreen state */
    bool fullscreen;
    int saved_win_x, saved_win_y, saved_win_w, saved_win_h;

} GameState;

/* ─────────────── OPENGL RENDERER STATE ─────────────────── */
typedef struct {
    GLuint quad_vao, quad_vbo;
    GLuint basic_shader;
    GLuint glow_shader;
    GLuint particle_shader;
    GLuint bg_shader;
    GLuint text_shader;

    /* Uniform locations cached */
    GLint basic_proj_loc, basic_color_loc, basic_model_loc;
    GLint glow_proj_loc, glow_color_loc, glow_model_loc, glow_intensity_loc, glow_time_loc;
    GLint particle_proj_loc, particle_color_loc, particle_model_loc, particle_alpha_loc;
    GLint bg_proj_loc, bg_time_loc, bg_resolution_loc;

    /* Projection matrix (orthographic) */
    float projection[16];

    /* Actual framebuffer dimensions (may differ from window on HiDPI) */
    int fb_width, fb_height;

    /* Letterboxed viewport (maintains WINDOW_WIDTH:WINDOW_HEIGHT aspect ratio) */
    int vp_x, vp_y, vp_w, vp_h;

    /* Mouse coordinate transform: virtual = (window - offset) * scale */
    float mouse_offset_x, mouse_offset_y;
    float mouse_scale_x, mouse_scale_y;

    /* Bloom pipeline */
    GLuint scene_fbo, scene_tex;
    GLuint blur_fbo[2], blur_tex[2];
    GLuint bright_shader, blur_shader, composite_shader;
    GLint  bright_threshold_loc;
    GLint  blur_direction_loc, blur_image_loc;
    GLint  composite_strength_loc;

    /* Background theme uniforms */
    GLint bg_top_loc, bg_bot_loc, bg_accent_loc, bg_scan_str_loc;
    GLint bg_piece_colors_loc;
    GLint bg_mode3d_loc;
} Renderer;

/* ─────────────── FUNCTION DECLARATIONS ─────────────────── */

/* Init / cleanup */
GLFWwindow* init_window(void);
void init_renderer(Renderer *r, GLFWwindow *window);
void init_game(GameState *g);
void cleanup_renderer(Renderer *r);

/* Shaders */
GLuint compile_shader(const char *vert_src, const char *frag_src);

/* Game logic */
void spawn_piece(GameState *g);
bool check_collision(GameState *g, int type, int rotation, int x, int y);
void lock_piece(GameState *g);
void check_lines(GameState *g);
void update_ghost(GameState *g);
void rotate_piece(GameState *g, int dir);
void hold_piece(GameState *g);
void hard_drop(GameState *g);
void update_game(GameState *g, double dt);

/* Particles */
void spawn_particles(GameState *g, float x, float y, Color color, int count);
void spawn_line_clear_particles(GameState *g, int row);
void update_particles(GameState *g, double dt);

/* Rendering */
void render_background(Renderer *r, GameState *g);
void render_grid(Renderer *r, GameState *g);
void render_current_piece(Renderer *r, GameState *g);
void render_ghost_piece(Renderer *r, GameState *g);
void render_locked_blocks(Renderer *r, GameState *g);
void render_particles(Renderer *r, GameState *g);
void render_ui(Renderer *r, GameState *g);
void render_game(Renderer *r, GameState *g);

/* Utility */
void ortho_matrix(float *m, float l, float r, float b, float t, float n, float f);
void identity_matrix(float *m);
void translate_matrix(float *m, float x, float y);
void scale_matrix(float *m, float sx, float sy);

/* Input */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

/* Save / Load */
void save_game(GameState *g);
void load_game(GameState *g);
void delete_save(void);
void save_high_scores(HighScoreTable *ht);
void load_high_scores(HighScoreTable *ht);
void submit_high_score(GameState *g);

/* Config */
void save_config(GameState *g);
void load_config(GameState *g);

#endif /* TETRIS_H */
