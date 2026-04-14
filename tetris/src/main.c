/* ═══════════════════════════════════════════════════════════════════════════
 *  NEON TETRIS — A Cyberpunk Tetris in Pure C + OpenGL 3.3
 *  By Rishi Saha | IIIT Kalyani | 2026
 *
 *  Features:
 *    - OpenGL 3.3 Core Profile + GLEW + GLFW
 *    - Neon glow shaders with pulsing effects
 *    - Particle explosions on line clears
 *    - Ghost piece, hold piece, next piece preview
 *    - Combo scoring system with levels
 *    - Screen shake on hard drops & line clears
 *    - Animated cyberpunk grid background
 *    - CRT scanline overlay
 *    - Pixel font text rendering
 *    - Mouse-clickable UI buttons
 *    - Main menu, pause, and game over screens
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "../include/tetris.h"
#include "../include/shaders.h"
#include "../include/sound.h"

/* Global state (needed for GLFW callbacks) */
static GameState g_game;
static Renderer *g_renderer;

/* ═══════════════════════════════════════════════════════════
 *  THEME DEFINITIONS — 10 visual themes
 * ═══════════════════════════════════════════════════════════ */

static const Theme THEMES[NUM_THEMES] = {
    /* ─── 0: Cyberpunk Neon (default) ─── */
    {
        "CYBERPUNK NEON", "NEON EDITION",
        /* pieces: I, O, T, S, Z, J, L */
        {{0.0f,1.0f,1.0f,1.0f},{1.0f,1.0f,0.0f,1.0f},{0.8f,0.0f,1.0f,1.0f},
         {0.0f,1.0f,0.3f,1.0f},{1.0f,0.1f,0.3f,1.0f},{0.0f,0.5f,1.0f,1.0f},
         {1.0f,0.5f,0.0f,1.0f}},
        {0.05f,0.02f,0.1f,1.0f}, {0.02f,0.01f,0.05f,1.0f},  /* bg top/bot */
        {0.1f,0.02f,0.2f,1.0f},                                /* accent */
        {0.15f,0.05f,0.25f,0.4f}, {0.4f,0.1f,0.6f,0.8f},     /* grid */
        {0.0f,0.0f,0.0f,0.7f},                                 /* grid bg */
        {0.0f,1.0f,1.0f,1.0f}, {0.8f,0.3f,1.0f,1.0f},        /* ui primary/secondary */
        {1.0f,1.0f,1.0f,0.9f},                                 /* ui text */
        1.0f, 0.6f, 1.0f                                       /* glow/bloom/scanline */
    },
    /* ─── 1: Classic ─── */
    {
        "CLASSIC", "ORIGINAL EDITION",
        {{0.0f,0.75f,1.0f,1.0f},{1.0f,0.84f,0.0f,1.0f},{0.6f,0.2f,1.0f,1.0f},
         {0.0f,0.9f,0.0f,1.0f},{1.0f,0.0f,0.0f,1.0f},{0.0f,0.0f,1.0f,1.0f},
         {1.0f,0.55f,0.0f,1.0f}},
        {0.02f,0.02f,0.02f,1.0f}, {0.0f,0.0f,0.0f,1.0f},
        {0.05f,0.05f,0.05f,1.0f},
        {0.15f,0.15f,0.15f,0.3f}, {0.4f,0.4f,0.4f,0.6f},
        {0.0f,0.0f,0.0f,0.8f},
        {1.0f,1.0f,1.0f,1.0f}, {0.8f,0.8f,0.8f,1.0f},
        {1.0f,1.0f,1.0f,0.9f},
        0.15f, 0.0f, 0.0f
    },
    /* ─── 2: Pastel Dream ─── */
    {
        "PASTEL DREAM", "SOFT EDITION",
        {{0.53f,0.81f,0.92f,1.0f},{1.0f,0.95f,0.7f,1.0f},{0.82f,0.63f,0.87f,1.0f},
         {0.6f,0.93f,0.6f,1.0f},{0.96f,0.5f,0.45f,1.0f},{0.7f,0.7f,1.0f,1.0f},
         {1.0f,0.78f,0.65f,1.0f}},
        {0.14f,0.1f,0.12f,1.0f}, {0.08f,0.06f,0.07f,1.0f},
        {0.12f,0.08f,0.1f,1.0f},
        {0.25f,0.2f,0.22f,0.3f}, {0.5f,0.4f,0.45f,0.5f},
        {0.02f,0.02f,0.03f,0.6f},
        {0.82f,0.63f,0.87f,1.0f}, {0.53f,0.81f,0.92f,1.0f},
        {1.0f,0.95f,0.9f,0.9f},
        0.3f, 0.25f, 0.0f
    },
    /* ─── 3: Monochrome ─── */
    {
        "MONOCHROME", "GRAYSCALE EDITION",
        {{1.0f,1.0f,1.0f,1.0f},{0.85f,0.85f,0.85f,1.0f},{0.7f,0.7f,0.7f,1.0f},
         {0.78f,0.78f,0.78f,1.0f},{0.6f,0.6f,0.6f,1.0f},{0.82f,0.82f,0.82f,1.0f},
         {0.92f,0.92f,0.92f,1.0f}},
        {0.04f,0.04f,0.04f,1.0f}, {0.0f,0.0f,0.0f,1.0f},
        {0.08f,0.08f,0.08f,1.0f},
        {0.18f,0.18f,0.18f,0.35f}, {0.4f,0.4f,0.4f,0.7f},
        {0.0f,0.0f,0.0f,0.75f},
        {1.0f,1.0f,1.0f,1.0f}, {0.6f,0.6f,0.6f,1.0f},
        {0.9f,0.9f,0.9f,0.9f},
        0.5f, 0.3f, 0.5f
    },
    /* ─── 4: Inverse ─── */
    {
        "INVERSE", "LIGHT EDITION",
        {{0.0f,0.45f,0.5f,1.0f},{0.55f,0.45f,0.0f,1.0f},{0.4f,0.0f,0.55f,1.0f},
         {0.0f,0.5f,0.2f,1.0f},{0.55f,0.05f,0.15f,1.0f},{0.0f,0.2f,0.55f,1.0f},
         {0.55f,0.3f,0.0f,1.0f}},
        {0.92f,0.9f,0.88f,1.0f}, {0.85f,0.82f,0.8f,1.0f},
        {0.7f,0.68f,0.65f,1.0f},
        {0.75f,0.72f,0.7f,0.3f}, {0.5f,0.48f,0.45f,0.5f},
        {0.88f,0.86f,0.84f,0.6f},
        {0.0f,0.4f,0.5f,1.0f}, {0.4f,0.0f,0.5f,1.0f},
        {0.15f,0.12f,0.1f,0.9f},
        0.0f, 0.0f, 0.0f
    },
    /* ─── 5: Solarized Dark ─── */
    {
        "SOLARIZED", "DARK EDITION",
        {{0.16f,0.63f,0.6f,1.0f},{0.71f,0.54f,0.0f,1.0f},{0.42f,0.44f,0.77f,1.0f},
         {0.52f,0.6f,0.0f,1.0f},{0.86f,0.2f,0.18f,1.0f},{0.15f,0.55f,0.82f,1.0f},
         {0.8f,0.29f,0.09f,1.0f}},
        {0.03f,0.21f,0.26f,1.0f}, {0.0f,0.17f,0.21f,1.0f},
        {0.07f,0.24f,0.29f,1.0f},
        {0.07f,0.26f,0.32f,0.4f}, {0.16f,0.63f,0.6f,0.5f},
        {0.0f,0.15f,0.19f,0.7f},
        {0.16f,0.63f,0.6f,1.0f}, {0.42f,0.44f,0.77f,1.0f},
        {0.58f,0.63f,0.63f,0.9f},
        0.4f, 0.2f, 0.3f
    },
    /* ─── 6: Synthwave ─── */
    {
        "SYNTHWAVE", "RETROWAVE EDITION",
        {{1.0f,0.1f,0.6f,1.0f},{1.0f,1.0f,0.2f,1.0f},{0.6f,0.0f,1.0f,1.0f},
         {0.2f,1.0f,0.4f,1.0f},{1.0f,0.0f,0.2f,1.0f},{0.2f,0.4f,1.0f,1.0f},
         {1.0f,0.0f,0.8f,1.0f}},
        {0.08f,0.01f,0.12f,1.0f}, {0.03f,0.0f,0.06f,1.0f},
        {0.15f,0.02f,0.2f,1.0f},
        {0.2f,0.03f,0.3f,0.4f}, {0.6f,0.1f,0.8f,0.7f},
        {0.02f,0.0f,0.04f,0.7f},
        {1.0f,0.1f,0.6f,1.0f}, {0.2f,0.4f,1.0f,1.0f},
        {1.0f,0.9f,1.0f,0.9f},
        1.0f, 0.8f, 0.7f
    },
    /* ─── 7: Ocean ─── */
    {
        "OCEAN", "DEEP SEA EDITION",
        {{0.0f,0.6f,0.9f,1.0f},{0.9f,0.85f,0.5f,1.0f},{0.5f,0.3f,0.75f,1.0f},
         {0.2f,0.8f,0.6f,1.0f},{0.9f,0.3f,0.3f,1.0f},{0.1f,0.3f,0.8f,1.0f},
         {0.9f,0.5f,0.2f,1.0f}},
        {0.02f,0.06f,0.12f,1.0f}, {0.01f,0.03f,0.08f,1.0f},
        {0.03f,0.1f,0.15f,1.0f},
        {0.05f,0.15f,0.25f,0.35f}, {0.1f,0.3f,0.5f,0.6f},
        {0.01f,0.04f,0.08f,0.7f},
        {0.0f,0.7f,0.9f,1.0f}, {0.2f,0.8f,0.6f,1.0f},
        {0.8f,0.9f,1.0f,0.9f},
        0.7f, 0.4f, 0.3f
    },
    /* ─── 8: Sunset ─── */
    {
        "SUNSET", "WARM EDITION",
        {{0.4f,0.7f,1.0f,1.0f},{1.0f,0.85f,0.3f,1.0f},{0.9f,0.2f,0.6f,1.0f},
         {0.7f,0.9f,0.2f,1.0f},{0.9f,0.15f,0.2f,1.0f},{0.3f,0.2f,0.8f,1.0f},
         {1.0f,0.4f,0.1f,1.0f}},
        {0.1f,0.03f,0.02f,1.0f}, {0.06f,0.01f,0.01f,1.0f},
        {0.15f,0.05f,0.02f,1.0f},
        {0.2f,0.08f,0.05f,0.35f}, {0.5f,0.2f,0.1f,0.6f},
        {0.04f,0.01f,0.01f,0.7f},
        {1.0f,0.6f,0.2f,1.0f}, {0.9f,0.2f,0.4f,1.0f},
        {1.0f,0.95f,0.85f,0.9f},
        0.8f, 0.5f, 0.4f
    },
    /* ─── 9: Matrix ─── */
    {
        "MATRIX", "HACKER EDITION",
        {{0.0f,1.0f,0.0f,1.0f},{0.5f,1.0f,0.3f,1.0f},{0.0f,0.8f,0.2f,1.0f},
         {0.3f,1.0f,0.0f,1.0f},{0.2f,0.9f,0.1f,1.0f},{0.0f,0.7f,0.4f,1.0f},
         {0.4f,0.9f,0.0f,1.0f}},
        {0.01f,0.04f,0.01f,1.0f}, {0.0f,0.02f,0.0f,1.0f},
        {0.02f,0.08f,0.02f,1.0f},
        {0.05f,0.15f,0.05f,0.35f}, {0.1f,0.4f,0.1f,0.6f},
        {0.0f,0.02f,0.0f,0.75f},
        {0.0f,1.0f,0.0f,1.0f}, {0.0f,0.7f,0.3f,1.0f},
        {0.6f,1.0f,0.6f,0.9f},
        0.9f, 0.5f, 0.8f
    },
};

/* Helper: get piece color from current theme */
static Color piece_color(int type) {
    return THEMES[g_game.current_theme].pieces[type];
}

/* Helper: get current theme */
static const Theme* current_theme(void) {
    return &THEMES[g_game.current_theme];
}

/* ═══════════════════════════════════════════════════════════
 *  DIFFICULTY SETTINGS
 * ═══════════════════════════════════════════════════════════ */

static const char *DIFF_NAMES[NUM_DIFFICULTIES] = {
    "EASY", "NORMAL", "HARD", "INSANE"
};

/* { initial_fall_interval, speed_step_per_level, min_fall_interval } */
static const float DIFF_FALL_INITIAL[NUM_DIFFICULTIES]  = { 1.0f,  0.8f,  0.5f,  0.3f  };
static const float DIFF_FALL_STEP[NUM_DIFFICULTIES]     = { 0.04f, 0.05f, 0.06f, 0.07f };
static const float DIFF_FALL_MIN[NUM_DIFFICULTIES]      = { 0.10f, 0.05f, 0.03f, 0.02f };

/* Score multiplier per difficulty */
static const float DIFF_SCORE_MULT[NUM_DIFFICULTIES]    = { 0.75f, 1.0f,  1.5f,  2.0f  };

/* ─────────────────── MATRIX UTILITIES ──────────────────── */

void identity_matrix(float *m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void ortho_matrix(float *m, float l, float r, float b, float t, float n, float f) {
    memset(m, 0, 16 * sizeof(float));
    m[0]  =  2.0f / (r - l);
    m[5]  =  2.0f / (t - b);
    m[10] = -2.0f / (f - n);
    m[12] = -(r + l) / (r - l);
    m[13] = -(t + b) / (t - b);
    m[14] = -(f + n) / (f - n);
    m[15] = 1.0f;
}

void translate_matrix(float *m, float x, float y) {
    identity_matrix(m);
    m[12] = x;
    m[13] = y;
}

void scale_matrix(float *m, float sx, float sy) {
    identity_matrix(m);
    m[0] = sx;
    m[5] = sy;
}

/* Combined translate + scale */
static void model_matrix(float *m, float x, float y, float sx, float sy) {
    memset(m, 0, 16 * sizeof(float));
    m[0]  = sx;
    m[5]  = sy;
    m[10] = 1.0f;
    m[12] = x;
    m[13] = y;
    m[15] = 1.0f;
}

/* ─────────────────── SHADER COMPILATION ────────────────── */

GLuint compile_shader(const char *vert_src, const char *frag_src) {
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vert_src, NULL);
    glCompileShader(vert);

    GLint success;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vert, 512, NULL, log);
        fprintf(stderr, "Vertex shader error:\n%s\n", log);
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &frag_src, NULL);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(frag, 512, NULL, log);
        fprintf(stderr, "Fragment shader error:\n%s\n", log);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        fprintf(stderr, "Shader link error:\n%s\n", log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

/* ─────────────────── WINDOW INIT ───────────────────────── */

GLFWwindow* init_window(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                           "◈ NEON TETRIS ◈ — Cyberpunk Edition", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  /* VSync */

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    return window;
}

/* ─────────────────── RENDERER INIT ─────────────────────── */

/* Forward declaration — defined after cleanup_renderer */
static void update_viewport(Renderer *r, GLFWwindow *window);

void init_renderer(Renderer *r, GLFWwindow *window) {
    /* Unit quad: [0,0] to [1,1] */
    float quad[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    glGenVertexArrays(1, &r->quad_vao);
    glGenBuffers(1, &r->quad_vbo);
    glBindVertexArray(r->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /* Compile all shaders */
    r->basic_shader    = compile_shader(basic_vert_src, basic_frag_src);
    r->glow_shader     = compile_shader(glow_vert_src, glow_frag_src);
    r->particle_shader = compile_shader(particle_vert_src, particle_frag_src);
    r->bg_shader       = compile_shader(bg_vert_src, bg_frag_src);

    /* Cache uniform locations */
    r->basic_proj_loc  = glGetUniformLocation(r->basic_shader, "uProjection");
    r->basic_color_loc = glGetUniformLocation(r->basic_shader, "uColor");
    r->basic_model_loc = glGetUniformLocation(r->basic_shader, "uModel");

    r->glow_proj_loc      = glGetUniformLocation(r->glow_shader, "uProjection");
    r->glow_color_loc     = glGetUniformLocation(r->glow_shader, "uColor");
    r->glow_model_loc     = glGetUniformLocation(r->glow_shader, "uModel");
    r->glow_intensity_loc = glGetUniformLocation(r->glow_shader, "uIntensity");
    r->glow_time_loc      = glGetUniformLocation(r->glow_shader, "uTime");

    r->particle_proj_loc  = glGetUniformLocation(r->particle_shader, "uProjection");
    r->particle_color_loc = glGetUniformLocation(r->particle_shader, "uColor");
    r->particle_model_loc = glGetUniformLocation(r->particle_shader, "uModel");
    r->particle_alpha_loc = glGetUniformLocation(r->particle_shader, "uAlpha");

    r->bg_time_loc       = glGetUniformLocation(r->bg_shader, "uTime");
    r->bg_resolution_loc = glGetUniformLocation(r->bg_shader, "uResolution");
    r->bg_top_loc        = glGetUniformLocation(r->bg_shader, "uBgTop");
    r->bg_bot_loc        = glGetUniformLocation(r->bg_shader, "uBgBot");
    r->bg_accent_loc     = glGetUniformLocation(r->bg_shader, "uAccent");
    r->bg_scan_str_loc   = glGetUniformLocation(r->bg_shader, "uScanStr");
    r->bg_piece_colors_loc = glGetUniformLocation(r->bg_shader, "uPieceColors");
    r->bg_mode3d_loc       = glGetUniformLocation(r->bg_shader, "uMode3D");

    /* ─── Bloom shaders ─── */
    r->bright_shader   = compile_shader(bg_vert_src, bright_frag_src);
    r->blur_shader     = compile_shader(bg_vert_src, blur_frag_src);
    r->composite_shader = compile_shader(bg_vert_src, composite_frag_src);

    r->bright_threshold_loc  = glGetUniformLocation(r->bright_shader, "uThreshold");
    r->blur_direction_loc    = glGetUniformLocation(r->blur_shader, "uDirection");
    r->blur_image_loc        = glGetUniformLocation(r->blur_shader, "uImage");
    r->composite_strength_loc = glGetUniformLocation(r->composite_shader, "uBloomStr");

    /* ─── Bloom FBOs — sized to letterboxed viewport ─── */
    update_viewport(r, window);
    int w = r->vp_w > 0 ? r->vp_w : 1;
    int h = r->vp_h > 0 ? r->vp_h : 1;
    int bw = w / 2, bh = h / 2;
    if (bw < 1) bw = 1;
    if (bh < 1) bh = 1;

    /* Scene FBO (full resolution) */
    glGenFramebuffers(1, &r->scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, r->scene_fbo);
    glGenTextures(1, &r->scene_tex);
    glBindTexture(GL_TEXTURE_2D, r->scene_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r->scene_tex, 0);

    /* Blur FBOs (half resolution, ping-pong) */
    for (int i = 0; i < 2; i++) {
        glGenFramebuffers(1, &r->blur_fbo[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[i]);
        glGenTextures(1, &r->blur_tex[i]);
        glBindTexture(GL_TEXTURE_2D, r->blur_tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bw, bh, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r->blur_tex[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Orthographic projection */
    ortho_matrix(r->projection, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
}

void cleanup_renderer(Renderer *r) {
    glDeleteVertexArrays(1, &r->quad_vao);
    glDeleteBuffers(1, &r->quad_vbo);
    glDeleteProgram(r->basic_shader);
    glDeleteProgram(r->glow_shader);
    glDeleteProgram(r->particle_shader);
    glDeleteProgram(r->bg_shader);
    glDeleteProgram(r->bright_shader);
    glDeleteProgram(r->blur_shader);
    glDeleteProgram(r->composite_shader);
    glDeleteFramebuffers(1, &r->scene_fbo);
    glDeleteTextures(1, &r->scene_tex);
    glDeleteFramebuffers(2, r->blur_fbo);
    glDeleteTextures(2, r->blur_tex);
}

/* ═══════════════════════════════════════════════════════════
 *  VIEWPORT & RESIZE — letterbox to maintain aspect ratio
 * ═══════════════════════════════════════════════════════════ */

static void update_viewport(Renderer *r, GLFWwindow *window) {
    glfwGetFramebufferSize(window, &r->fb_width, &r->fb_height);

    if (r->fb_width <= 0 || r->fb_height <= 0) {
        r->vp_x = r->vp_y = 0;
        r->vp_w = r->fb_width;
        r->vp_h = r->fb_height;
        return;
    }

    float target_aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    float fb_aspect = (float)r->fb_width / (float)r->fb_height;

    if (fb_aspect > target_aspect) {
        /* Window is wider — letterbox on sides */
        r->vp_h = r->fb_height;
        r->vp_w = (int)(r->fb_height * target_aspect);
        r->vp_x = (r->fb_width - r->vp_w) / 2;
        r->vp_y = 0;
    } else {
        /* Window is taller — letterbox on top/bottom */
        r->vp_w = r->fb_width;
        r->vp_h = (int)(r->fb_width / target_aspect);
        r->vp_x = 0;
        r->vp_y = (r->fb_height - r->vp_h) / 2;
    }

    /* Compute mouse transform: window coords → virtual coords */
    int win_w, win_h;
    glfwGetWindowSize(window, &win_w, &win_h);
    if (win_w > 0 && win_h > 0) {
        float dpi_x = (float)r->fb_width / (float)win_w;
        float dpi_y = (float)r->fb_height / (float)win_h;
        float win_vp_x = (float)r->vp_x / dpi_x;
        float win_vp_y = (float)r->vp_y / dpi_y;
        float win_vp_w = (float)r->vp_w / dpi_x;
        float win_vp_h = (float)r->vp_h / dpi_y;
        r->mouse_offset_x = win_vp_x;
        r->mouse_offset_y = win_vp_y;
        r->mouse_scale_x = (win_vp_w > 0) ? (float)WINDOW_WIDTH / win_vp_w : 1.0f;
        r->mouse_scale_y = (win_vp_h > 0) ? (float)WINDOW_HEIGHT / win_vp_h : 1.0f;
    }
}

static void rebuild_fbos(Renderer *r) {
    int w = r->vp_w > 0 ? r->vp_w : 1;
    int h = r->vp_h > 0 ? r->vp_h : 1;
    int bw = w / 2, bh = h / 2;
    if (bw < 1) bw = 1;
    if (bh < 1) bh = 1;

    /* Resize scene FBO */
    glBindTexture(GL_TEXTURE_2D, r->scene_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    /* Resize blur FBOs */
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, r->blur_tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bw, bh, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    (void)width; (void)height;
    if (!g_renderer) return;
    update_viewport(g_renderer, window);
    rebuild_fbos(g_renderer);
}

/* ═══════════════════════════════════════════════════════════
 *  PIXEL FONT SYSTEM — 5x7 bitmap font rendered as quads
 * ═══════════════════════════════════════════════════════════ */

/*
 * 59 glyphs covering ASCII 32 (space) through 90 (Z).
 * Each glyph is 7 rows of 5-bit bitmasks.
 * Bit 4 = leftmost pixel, Bit 0 = rightmost pixel.
 */
static const uint8_t PIXEL_FONT[59][7] = {
    /* 32 ' ' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 33 '!' */ {0x04,0x04,0x04,0x04,0x04,0x00,0x04},
    /* 34 '"' */ {0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00},
    /* 35 '#' */ {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A},
    /* 36 '$' */ {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
    /* 37 '%' */ {0x18,0x19,0x02,0x04,0x08,0x13,0x03},
    /* 38 '&' */ {0x08,0x14,0x14,0x08,0x15,0x12,0x0D},
    /* 39 ''' */ {0x04,0x04,0x08,0x00,0x00,0x00,0x00},
    /* 40 '(' */ {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    /* 41 ')' */ {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    /* 42 '*' */ {0x00,0x04,0x15,0x0E,0x15,0x04,0x00},
    /* 43 '+' */ {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    /* 44 ',' */ {0x00,0x00,0x00,0x00,0x00,0x04,0x08},
    /* 45 '-' */ {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    /* 46 '.' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x04},
    /* 47 '/' */ {0x01,0x01,0x02,0x04,0x08,0x10,0x10},
    /* 48 '0' */ {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    /* 49 '1' */ {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    /* 50 '2' */ {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    /* 51 '3' */ {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    /* 52 '4' */ {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    /* 53 '5' */ {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    /* 54 '6' */ {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    /* 55 '7' */ {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    /* 56 '8' */ {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    /* 57 '9' */ {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    /* 58 ':' */ {0x00,0x00,0x04,0x00,0x04,0x00,0x00},
    /* 59 ';' */ {0x00,0x00,0x04,0x00,0x04,0x04,0x08},
    /* 60 '<' */ {0x02,0x04,0x08,0x10,0x08,0x04,0x02},
    /* 61 '=' */ {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
    /* 62 '>' */ {0x08,0x04,0x02,0x01,0x02,0x04,0x08},
    /* 63 '?' */ {0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    /* 64 '@' */ {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E},
    /* 65 'A' */ {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    /* 66 'B' */ {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    /* 67 'C' */ {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    /* 68 'D' */ {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},
    /* 69 'E' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    /* 70 'F' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    /* 71 'G' */ {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    /* 72 'H' */ {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    /* 73 'I' */ {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    /* 74 'J' */ {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    /* 75 'K' */ {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    /* 76 'L' */ {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    /* 77 'M' */ {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    /* 78 'N' */ {0x11,0x11,0x19,0x15,0x13,0x11,0x11},
    /* 79 'O' */ {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    /* 80 'P' */ {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    /* 81 'Q' */ {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    /* 82 'R' */ {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    /* 83 'S' */ {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    /* 84 'T' */ {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    /* 85 'U' */ {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    /* 86 'V' */ {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},
    /* 87 'W' */ {0x11,0x11,0x11,0x15,0x15,0x15,0x0A},
    /* 88 'X' */ {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    /* 89 'Y' */ {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    /* 90 'Z' */ {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

static const uint8_t* get_glyph(char ch) {
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    if (ch < 32 || ch > 90) return PIXEL_FONT[0];
    return PIXEL_FONT[ch - 32];
}

/* Forward declaration needed by font rendering */
static void draw_quad(Renderer *r, GLuint shader, float x, float y,
                      float w, float h, Color color);

static void render_char_px(Renderer *r, float x, float y, char ch, float ps, Color color) {
    const uint8_t *glyph = get_glyph(ch);
    float block = ps > 2.0f ? ps - 1.0f : ps * 0.85f;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row] & (1 << (4 - col))) {
                draw_quad(r, r->basic_shader,
                          x + col * ps, y + row * ps,
                          block, block, color);
            }
        }
    }
}

static void render_text_px(Renderer *r, float x, float y, const char *text, float ps, Color color) {
    float cx = x;
    while (*text) {
        if (*text != ' ') render_char_px(r, cx, y, *text, ps, color);
        cx += 6.0f * ps;
        text++;
    }
}

static float text_width_px(const char *text, float ps) {
    int len = (int)strlen(text);
    if (len == 0) return 0;
    return (len * 6.0f - 1.0f) * ps;
}

static void render_text_centered_px(Renderer *r, float cx, float cy, const char *text, float ps, Color color) {
    float w = text_width_px(text, ps);
    float h = 7.0f * ps;
    render_text_px(r, cx - w * 0.5f, cy - h * 0.5f, text, ps, color);
}

/* ═══════════════════════════════════════════════════════════
 *  MOUSE INPUT
 * ═══════════════════════════════════════════════════════════ */

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    (void)window;
    if (g_renderer) {
        g_game.mouse_x = (xpos - g_renderer->mouse_offset_x) * g_renderer->mouse_scale_x;
        g_game.mouse_y = (ypos - g_renderer->mouse_offset_y) * g_renderer->mouse_scale_y;
    } else {
        g_game.mouse_x = xpos;
        g_game.mouse_y = ypos;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    (void)window; (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        g_game.mouse_clicked = true;
    }
}

/* ═══════════════════════════════════════════════════════════
 *  BUTTON SYSTEM — clickable UI buttons with hover glow
 * ═══════════════════════════════════════════════════════════ */

static bool point_in_rect(double px, double py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

static bool draw_button(Renderer *r, GameState *game,
                          float x, float y, float w, float h,
                          const char *label, float font_size,
                          Color text_color, Color bg_color, Color border_color) {
    bool hovered = point_in_rect(game->mouse_x, game->mouse_y, x, y, w, h);

    /* Outer glow when hovered */
    if (hovered) {
        Color glow = border_color;
        glow.a = 0.2f + 0.1f * sinf((float)game->time * 4.0f);
        float gs = 6.0f;
        draw_quad(r, r->basic_shader, x - gs, y - gs, w + gs * 2, h + gs * 2, glow);
    }

    /* Background */
    Color bg = bg_color;
    if (hovered) { bg.a = fminf(bg.a + 0.15f, 1.0f); }
    draw_quad(r, r->basic_shader, x, y, w, h, bg);

    /* Border */
    float bw = 2.0f;
    Color bc = hovered ? text_color : border_color;
    float pulse = hovered ? (0.8f + 0.2f * sinf((float)game->time * 5.0f)) : 1.0f;
    Color pbc = {bc.r * pulse, bc.g * pulse, bc.b * pulse, bc.a};
    draw_quad(r, r->basic_shader, x, y, w, bw, pbc);
    draw_quad(r, r->basic_shader, x, y + h - bw, w, bw, pbc);
    draw_quad(r, r->basic_shader, x, y, bw, h, pbc);
    draw_quad(r, r->basic_shader, x + w - bw, y, bw, h, pbc);

    /* Text centered in button */
    Color tc = hovered ? (Color){1.0f, 1.0f, 1.0f, 1.0f} : text_color;
    render_text_centered_px(r, x + w * 0.5f, y + h * 0.5f, label, font_size, tc);

    bool clicked = hovered && game->mouse_clicked;
    if (clicked) {
        game->mouse_clicked = false;
        snd_play(SFX_MENU_SELECT);
    }
    return clicked;
}

/* ═══════════════════════════════════════════════════════════
 *  CROSS-PLATFORM HELPERS
 * ═══════════════════════════════════════════════════════════ */

/* Returns writable user directory on all platforms */
static const char *get_home_dir(void) {
    const char *dir;
    /* Unix / macOS / MSYS2 */
    dir = getenv("HOME");
    if (dir) return dir;
#ifdef _WIN32
    /* Windows native: prefer APPDATA (Roaming), fallback to USERPROFILE if APPDATA isn't set */
    dir = getenv("APPDATA");
    if (dir) return dir;
    dir = getenv("USERPROFILE");
    if (dir) return dir;
#endif
    return ".";
}

/* ═══════════════════════════════════════════════════════════
 *  SAVE / LOAD SYSTEM
 * ═══════════════════════════════════════════════════════════ */

static const char *get_save_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/.neon_tetris_save.dat", get_home_dir());
    return path;
}

static const char *get_highscore_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/.neon_tetris_scores.dat", get_home_dir());
    return path;
}

void save_game(GameState *g) {
    if (g->game_over) return; /* Don't save a finished game */

    SaveData sd;
    memset(&sd, 0, sizeof(sd));
    sd.magic = SAVE_MAGIC;
    memcpy(sd.grid, g->grid, sizeof(sd.grid));
    sd.current_type = g->current_type;
    sd.current_rotation = g->current_rotation;
    sd.current_x = g->current_x;
    sd.current_y = g->current_y;
    sd.next_type = g->next_type;
    sd.held_type = g->held_type;
    sd.can_hold = g->can_hold;
    sd.score = g->score;
    sd.lines_cleared = g->lines_cleared;
    sd.level = g->level;
    sd.combo = g->combo;
    sd.fall_interval = g->fall_interval;
    sd.valid = true;

    FILE *f = fopen(get_save_path(), "wb");
    if (f) {
        fwrite(&sd, sizeof(sd), 1, f);
        fclose(f);
    }
}

void load_game(GameState *g) {
    FILE *f = fopen(get_save_path(), "rb");
    if (!f) { g->has_save = false; return; }

    SaveData sd;
    size_t read = fread(&sd, sizeof(sd), 1, f);
    fclose(f);

    if (read != 1 || sd.magic != SAVE_MAGIC || !sd.valid) {
        g->has_save = false;
        return;
    }

    g->has_save = true;
}

static void restore_game(GameState *g) {
    FILE *f = fopen(get_save_path(), "rb");
    if (!f) return;

    SaveData sd;
    size_t read = fread(&sd, sizeof(sd), 1, f);
    fclose(f);

    if (read != 1 || sd.magic != SAVE_MAGIC || !sd.valid) return;

    /* Preserve screen/mouse/theme/3d state */
    GameScreen saved_screen = g->screen;
    double mx = g->mouse_x, my = g->mouse_y;
    HighScoreTable ht = g->high_scores;
    int theme = g->current_theme;
    bool m3d = g->mode_3d;

    memset(g, 0, sizeof(GameState));
    srand((unsigned)time(NULL));

    g->screen = saved_screen;
    g->mouse_x = mx;
    g->mouse_y = my;
    g->high_scores = ht;
    g->current_theme = theme;
    g->mode_3d = m3d;
    g->mode_3d_factor = m3d ? 1.0f : 0.0f;
    g->mode_transition_timer = -1.0f;

    memcpy(g->grid, sd.grid, sizeof(g->grid));
    g->current_type = sd.current_type;
    g->current_rotation = sd.current_rotation;
    g->current_x = sd.current_x;
    g->current_y = sd.current_y;
    g->next_type = sd.next_type;
    g->held_type = sd.held_type;
    g->can_hold = sd.can_hold;
    g->score = sd.score;
    g->lines_cleared = sd.lines_cleared;
    g->level = sd.level;
    g->combo = sd.combo;
    g->fall_interval = sd.fall_interval;
    g->has_save = false;

    update_ghost(g);
    delete_save();
}

void delete_save(void) {
    remove(get_save_path());
}

/* ═══════════════════════════════════════════════════════════
 *  HIGH SCORE SYSTEM
 * ═══════════════════════════════════════════════════════════ */

void save_high_scores(HighScoreTable *ht) {
    FILE *f = fopen(get_highscore_path(), "wb");
    if (f) {
        fwrite(ht, sizeof(HighScoreTable), 1, f);
        fclose(f);
    }
}

void load_high_scores(HighScoreTable *ht) {
    memset(ht, 0, sizeof(HighScoreTable));
    FILE *f = fopen(get_highscore_path(), "rb");
    if (!f) return;
    fread(ht, sizeof(HighScoreTable), 1, f);
    fclose(f);
}

void submit_high_score(GameState *g) {
    HighScoreTable *ht = &g->high_scores;
    HighScoreEntry entry = { g->score, g->lines_cleared, g->level };

    /* Find insertion position (sorted descending) */
    int pos = ht->count;
    for (int i = 0; i < ht->count; i++) {
        if (entry.score > ht->entries[i].score) {
            pos = i;
            break;
        }
    }

    if (pos >= MAX_HIGH_SCORES) return; /* Didn't make the list */

    /* Shift entries down */
    for (int i = (ht->count < MAX_HIGH_SCORES ? ht->count : MAX_HIGH_SCORES - 1) - 1; i >= pos; i--) {
        ht->entries[i + 1] = ht->entries[i];
    }
    ht->entries[pos] = entry;
    if (ht->count < MAX_HIGH_SCORES) ht->count++;

    save_high_scores(ht);
}

/* ═══════════════════════════════════════════════════════════
 *  CONFIG — persist theme preference
 * ═══════════════════════════════════════════════════════════ */

static const char *get_config_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/.neon_tetris_config.dat", get_home_dir());
    return path;
}

void save_config(GameState *g) {
    FILE *f = fopen(get_config_path(), "wb");
    if (f) {
        int theme = g->current_theme;
        int mode3d = g->mode_3d ? 1 : 0;
        int diff = (int)g->difficulty;
        fwrite(&theme, sizeof(int), 1, f);
        fwrite(&mode3d, sizeof(int), 1, f);
        fwrite(&diff, sizeof(int), 1, f);
        fclose(f);
    }
}

void load_config(GameState *g) {
    FILE *f = fopen(get_config_path(), "rb");
    if (!f) return;
    int theme;
    if (fread(&theme, sizeof(int), 1, f) == 1) {
        if (theme >= 0 && theme < NUM_THEMES)
            g->current_theme = theme;
    }
    int mode3d;
    if (fread(&mode3d, sizeof(int), 1, f) == 1) {
        g->mode_3d = (mode3d != 0);
        g->mode_3d_factor = g->mode_3d ? 1.0f : 0.0f;
    }
    int diff;
    if (fread(&diff, sizeof(int), 1, f) == 1) {
        if (diff >= 0 && diff < NUM_DIFFICULTIES)
            g->difficulty = (Difficulty)diff;
    }
    fclose(f);
}

/* ─────────────────── GAME INIT ─────────────────────────── */

static int random_piece(void) {
    return rand() % NUM_TETROMINOS;
}

void init_game(GameState *g) {
    GameScreen saved_screen = g->screen;
    double mx = g->mouse_x, my = g->mouse_y;
    HighScoreTable ht = g->high_scores;
    bool has_save = g->has_save;
    int theme = g->current_theme;
    bool m3d = g->mode_3d;
    Difficulty diff = g->difficulty;

    memset(g, 0, sizeof(GameState));
    srand((unsigned)time(NULL));

    g->screen = saved_screen;
    g->mouse_x = mx;
    g->mouse_y = my;
    g->high_scores = ht;
    g->has_save = has_save;
    g->current_theme = theme;
    g->mode_3d = m3d;
    g->mode_3d_factor = m3d ? 1.0f : 0.0f;
    g->mode_transition_timer = -1.0f;
    g->difficulty = diff;
    g->level = 1;
    g->fall_interval = DIFF_FALL_INITIAL[diff];
    g->held_type = -1;
    g->can_hold = true;
    g->next_type = random_piece();

    spawn_piece(g);
}

/* ─────────────────── PIECE SPAWNING ────────────────────── */

void spawn_piece(GameState *g) {
    g->current_type = g->next_type;
    g->next_type = random_piece();
    g->current_rotation = 0;
    g->current_x = GRID_COLS / 2 - 2;
    g->current_y = -1;
    g->can_hold = true;
    g->lock_delay_active = false;
    g->lock_timer = 0;

    if (check_collision(g, g->current_type, g->current_rotation,
                        g->current_x, g->current_y)) {
        g->game_over = true;
        submit_high_score(g);
        delete_save();
        snd_play(SFX_GAME_OVER);
    }

    update_ghost(g);
}

/* ─────────────────── COLLISION ─────────────────────────── */

bool check_collision(GameState *g, int type, int rotation, int x, int y) {
    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[type][rotation][row][col]) continue;

            int gx = x + col;
            int gy = y + row;

            if (gx < 0 || gx >= GRID_COLS || gy >= GRID_ROWS) return true;
            if (gy >= 0 && g->grid[gy][gx] != 0) return true;
        }
    }
    return false;
}

/* ─────────────────── GHOST PIECE ───────────────────────── */

void update_ghost(GameState *g) {
    g->ghost_y = g->current_y;
    while (!check_collision(g, g->current_type, g->current_rotation,
                            g->current_x, g->ghost_y + 1)) {
        g->ghost_y++;
    }
}

/* ─────────────────── ROTATION (SRS BASIC) ──────────────── */

void rotate_piece(GameState *g, int dir) {
    int new_rot = (g->current_rotation + dir + 4) % 4;

    /* Wall kick offsets to try */
    static const int kicks[][2] = {{0,0},{-1,0},{1,0},{0,-1},{-2,0},{2,0},{0,-2}};

    for (int i = 0; i < 7; i++) {
        int nx = g->current_x + kicks[i][0];
        int ny = g->current_y + kicks[i][1];
        if (!check_collision(g, g->current_type, new_rot, nx, ny)) {
            g->current_x = nx;
            g->current_y = ny;
            g->current_rotation = new_rot;
            update_ghost(g);

            if (g->lock_delay_active) {
                g->lock_timer = 0;
            }
            return;
        }
    }
}

/* ─────────────────── HOLD PIECE ────────────────────────── */

void hold_piece(GameState *g) {
    if (!g->can_hold) return;

    g->can_hold = false;
    if (g->held_type == -1) {
        g->held_type = g->current_type;
        spawn_piece(g);
    } else {
        int temp = g->held_type;
        g->held_type = g->current_type;
        g->current_type = temp;
        g->current_rotation = 0;
        g->current_x = GRID_COLS / 2 - 2;
        g->current_y = -1;
        g->lock_delay_active = false;
        update_ghost(g);
    }
}

/* ─────────────────── LOCK PIECE ────────────────────────── */

void lock_piece(GameState *g) {
    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[g->current_type][g->current_rotation][row][col]) continue;
            int gy = g->current_y + row;
            int gx = g->current_x + col;
            if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS) {
                g->grid[gy][gx] = g->current_type + 1;
            }
        }
    }

    /* Small screen shake on lock */
    g->shake_intensity = 2.0f;
    g->shake_timer = 0.1f;

    snd_play(SFX_LOCK);
    check_lines(g);
    spawn_piece(g);
}

/* ─────────────────── HARD DROP ─────────────────────────── */

void hard_drop(GameState *g) {
    int dropped = 0;
    while (!check_collision(g, g->current_type, g->current_rotation,
                            g->current_x, g->current_y + 1)) {
        g->current_y++;
        dropped++;
    }
    g->score += dropped * 2;

    /* Spawn trail particles along drop path */
    Color c = piece_color(g->current_type);
    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[g->current_type][g->current_rotation][row][col]) continue;
            float px = GRID_OFFSET_X + (g->current_x + col) * CELL_SIZE + CELL_SIZE * 0.5f;
            float py = GRID_OFFSET_Y + (g->current_y + row) * CELL_SIZE + CELL_SIZE * 0.5f;
            spawn_particles(g, px, py, c, 4);
        }
    }

    /* Bigger screen shake on hard drop */
    g->shake_intensity = 5.0f;
    g->shake_timer = 0.15f;

    snd_play(SFX_HARD_DROP);
    lock_piece(g);
}

/* ─────────────────── LINE CLEARING ─────────────────────── */

void check_lines(GameState *g) {
    int lines = 0;

    for (int row = 0; row < GRID_ROWS; row++) {
        bool full = true;
        for (int col = 0; col < GRID_COLS; col++) {
            if (g->grid[row][col] == 0) { full = false; break; }
        }
        if (full) {
            g->line_clearing[row] = true;
            g->line_clear_anim[row] = 1.0f;
            lines++;
            spawn_line_clear_particles(g, row);
        }
    }

    if (lines > 0) {
        g->animating_clear = true;
        g->clear_timer = 0.4;

        /* Sound: tetris vs normal clear */
        if (lines >= 4)
            snd_play(SFX_TETRIS);
        else
            snd_play(SFX_LINE_CLEAR);

        /* Scoring: 100, 300, 500, 800 for 1-4 lines (scaled by difficulty) */
        static const int score_table[] = {0, 100, 300, 500, 800};
        g->combo++;
        int base = score_table[lines > 4 ? 4 : lines];
        float smul = DIFF_SCORE_MULT[g->difficulty];
        g->score += (int)((base * g->level + (g->combo - 1) * 50 * g->level) * smul);
        g->lines_cleared += lines;

        /* Level up every 10 lines — speed governed by difficulty */
        int old_level = g->level;
        g->level = g->lines_cleared / 10 + 1;
        Difficulty d = g->difficulty;
        g->fall_interval = DIFF_FALL_INITIAL[d] - (g->level - 1) * DIFF_FALL_STEP[d];
        if (g->fall_interval < DIFF_FALL_MIN[d]) g->fall_interval = DIFF_FALL_MIN[d];

        if (g->level > old_level)
            snd_play(SFX_LEVEL_UP);

        if (g->combo > 1)
            snd_play_vol(SFX_COMBO, 0.7f);

        /* Big shake for multi-line clear */
        g->shake_intensity = 4.0f + lines * 3.0f;
        g->shake_timer = 0.2f + lines * 0.05f;
    } else {
        g->combo = 0;
    }
}

static void collapse_lines(GameState *g) {
    for (int row = GRID_ROWS - 1; row >= 0; row--) {
        if (!g->line_clearing[row]) continue;

        /* Move everything above down */
        for (int r = row; r > 0; r--) {
            memcpy(g->grid[r], g->grid[r - 1], GRID_COLS * sizeof(int));
            g->line_clearing[r] = g->line_clearing[r - 1];
            g->line_clear_anim[r] = g->line_clear_anim[r - 1];
        }
        memset(g->grid[0], 0, GRID_COLS * sizeof(int));
        g->line_clearing[0] = false;
        g->line_clear_anim[0] = 0;
        row++; /* Re-check this row */
    }
    g->animating_clear = false;
}

/* ─────────────────── PARTICLE SYSTEM ───────────────────── */

void spawn_particles(GameState *g, float x, float y, Color color, int count) {
    for (int i = 0; i < count && g->particle_count < MAX_PARTICLES; i++) {
        Particle *p = &g->particles[g->particle_count++];
        p->x = x;
        p->y = y;
        p->vx = ((float)rand() / (float)RAND_MAX - 0.5f) * 300.0f;
        p->vy = ((float)rand() / (float)RAND_MAX - 0.5f) * 300.0f - 100.0f;
        p->life = 0.5f + (float)rand() / (float)RAND_MAX * 1.0f;
        p->max_life = p->life;
        p->size = 3.0f + (float)rand() / (float)RAND_MAX * 8.0f;
        p->color = color;
        p->active = true;
    }
}

void spawn_line_clear_particles(GameState *g, int row) {
    for (int col = 0; col < GRID_COLS; col++) {
        int type = g->grid[row][col];
        if (type == 0) continue;
        Color c = piece_color(type - 1);

        float px = GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE * 0.5f;
        float py = GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE * 0.5f;

        /* Burst of particles per cell */
        spawn_particles(g, px, py, c, 12);
    }
}

void update_particles(GameState *g, double dt) {
    int alive = 0;
    for (int i = 0; i < g->particle_count; i++) {
        Particle *p = &g->particles[i];
        if (!p->active) continue;

        p->x += p->vx * (float)dt;
        p->y += p->vy * (float)dt;
        p->vy += 200.0f * (float)dt; /* Gravity */
        p->life -= (float)dt;
        p->size *= 0.995f;

        if (p->life <= 0) {
            p->active = false;
        } else {
            if (alive != i) g->particles[alive] = *p;
            alive++;
        }
    }
    g->particle_count = alive;
}

/* ─────────────────── GAME UPDATE ───────────────────────── */

void update_game(GameState *g, double dt) {
    /* Always update time and visuals for animations */
    g->time += dt;
    g->glow_pulse = (float)g->time;

    /* 3D mode transition animation (runs on all screens) */
    if (g->mode_transition_timer >= 0.0f) {
        g->mode_transition_timer += (float)dt * 1.8f;
        if (g->mode_transition_timer >= 1.0f) {
            g->mode_transition_timer = -1.0f;
            g->mode_3d_factor = g->mode_3d ? 1.0f : 0.0f;
            g->fog_alpha = 0.0f;
        } else {
            float tt = g->mode_transition_timer;
            /* Fog: bell curve peaking at midpoint */
            g->fog_alpha = sinf(tt * PI) * 0.85f;
            /* Factor: smoothstep from old state to new */
            float target = g->mode_3d ? 1.0f : 0.0f;
            float start  = g->mode_3d ? 0.0f : 1.0f;
            float s = tt * tt * (3.0f - 2.0f * tt);
            g->mode_3d_factor = start + (target - start) * s;
        }
    }

    /* Particles fade out even when paused/game over */
    update_particles(g, dt);

    /* Screen shake decay always */
    if (g->shake_timer > 0) {
        g->shake_timer -= (float)dt;
        if (g->shake_timer <= 0) {
            g->shake_intensity = 0;
        }
    }

    if (g->screen != SCREEN_PLAYING) return;
    if (g->game_over || g->paused) return;

    /* Line clear animation */
    if (g->animating_clear) {
        g->clear_timer -= dt;
        for (int row = 0; row < GRID_ROWS; row++) {
            if (g->line_clearing[row]) {
                g->line_clear_anim[row] -= (float)dt * 3.0f;
            }
        }
        if (g->clear_timer <= 0) {
            collapse_lines(g);
        }
        return; /* Don't move pieces during clear animation */
    }

    /* DAS (Delayed Auto Shift) for left/right */
    if (g->key_left || g->key_right) {
        if (!g->das_active) {
            g->das_timer += dt;
            if (g->das_timer >= 0.17) { /* DAS delay */
                g->das_active = true;
                g->arr_timer = 0;
            }
        }
        if (g->das_active) {
            g->arr_timer += dt;
            while (g->arr_timer >= 0.033) { /* ARR rate ~30Hz */
                g->arr_timer -= 0.033;
                int dir = g->key_left ? -1 : 1;
                if (!check_collision(g, g->current_type, g->current_rotation,
                                     g->current_x + dir, g->current_y)) {
                    g->current_x += dir;
                    update_ghost(g);
                    if (g->lock_delay_active) g->lock_timer = 0;
                }
            }
        }
    }

    /* Soft drop */
    double effective_interval = g->key_down ? g->fall_interval * 0.1 : g->fall_interval;

    g->fall_timer += dt;
    while (g->fall_timer >= effective_interval) {
        g->fall_timer -= effective_interval;

        if (!check_collision(g, g->current_type, g->current_rotation,
                             g->current_x, g->current_y + 1)) {
            g->current_y++;
            g->lock_delay_active = false;
            if (g->key_down) g->score += 1;
        } else {
            /* Lock delay */
            if (!g->lock_delay_active) {
                g->lock_delay_active = true;
                g->lock_timer = 0;
            }
        }
    }

    /* Lock delay countdown */
    if (g->lock_delay_active) {
        g->lock_timer += dt;
        if (g->lock_timer >= 0.5) {
            lock_piece(g);
        }
    }
}

/* ─────────────────── INPUT ─────────────────────────────── */

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    GameState *g = &g_game;

    /* F11 or Cmd+F (macOS) toggles fullscreen */
    if (action == GLFW_PRESS && (key == GLFW_KEY_F11 ||
        (key == GLFW_KEY_F && (mods & GLFW_MOD_SUPER)))) {
        if (g->fullscreen) {
            glfwSetWindowMonitor(window, NULL,
                                 g->saved_win_x, g->saved_win_y,
                                 g->saved_win_w, g->saved_win_h, 0);
            g->fullscreen = false;
        } else {
            glfwGetWindowPos(window, &g->saved_win_x, &g->saved_win_y);
            glfwGetWindowSize(window, &g->saved_win_w, &g->saved_win_h);
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height, mode->refreshRate);
            g->fullscreen = true;
        }
        return;
    }

    /* M key toggles mute from any screen */
    if (action == GLFW_PRESS && key == GLFW_KEY_M) {
        snd_toggle_mute();
        return;
    }

    /* V key toggles 2D/3D mode from any screen */
    if (action == GLFW_PRESS && key == GLFW_KEY_V) {
        g->mode_3d = !g->mode_3d;
        g->mode_transition_timer = 0.0f;
        save_config(g);
        snd_play(SFX_MENU_NAV);
        return;
    }

    /* High scores / About screen — ESC or any key returns to menu */
    if (g->screen == SCREEN_HIGHSCORES || g->screen == SCREEN_ABOUT) {
        if (action == GLFW_PRESS) {
            g->screen = SCREEN_MENU;
            snd_play(SFX_MENU_NAV);
        }
        return;
    }

    /* Menu screen input */
    if (g->screen == SCREEN_MENU) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
                snd_play(SFX_MENU_SELECT);
                g->screen = SCREEN_PLAYING;
                init_game(g);
                g->screen = SCREEN_PLAYING;
            } else if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            } else if (key == GLFW_KEY_T || key == GLFW_KEY_RIGHT) {
                g->current_theme = (g->current_theme + 1) % NUM_THEMES;
                save_config(g);
                snd_play(SFX_MENU_NAV);
            } else if (key == GLFW_KEY_LEFT) {
                g->current_theme = (g->current_theme - 1 + NUM_THEMES) % NUM_THEMES;
                save_config(g);
                snd_play(SFX_MENU_NAV);
            } else if (key == GLFW_KEY_D) {
                g->difficulty = (Difficulty)((g->difficulty + 1) % NUM_DIFFICULTIES);
                save_config(g);
                snd_play(SFX_MENU_NAV);
            }
        }
        return;
    }

    /* Playing screen input */
    if (action == GLFW_PRESS) {
        /* Game over state — only restart, menu, or quit */
        if (g->game_over) {
            switch (key) {
                case GLFW_KEY_R:
                    snd_play(SFX_MENU_SELECT);
                    delete_save();
                    init_game(g);
                    g->screen = SCREEN_PLAYING;
                    break;
                case GLFW_KEY_ESCAPE:
                    snd_play(SFX_MENU_NAV);
                    delete_save();
                    g->screen = SCREEN_MENU;
                    break;
            }
            return;
        }

        /* Paused state */
        if (g->paused) {
            switch (key) {
                case GLFW_KEY_P:
                    g->paused = false;
                    snd_play(SFX_PAUSE);
                    break;
                case GLFW_KEY_ESCAPE:
                    snd_play(SFX_MENU_NAV);
                    save_game(g);
                    g->screen = SCREEN_MENU;
                    load_game(g);
                    break;
            }
            return;
        }

        /* Normal gameplay */
        switch (key) {
            case GLFW_KEY_LEFT:
                if (!check_collision(g, g->current_type, g->current_rotation,
                                     g->current_x - 1, g->current_y)) {
                    g->current_x--;
                    update_ghost(g);
                    if (g->lock_delay_active) g->lock_timer = 0;
                    snd_play_vol(SFX_MOVE, 0.5f);
                }
                g->key_left = true;
                g->das_timer = 0;
                g->das_active = false;
                break;

            case GLFW_KEY_RIGHT:
                if (!check_collision(g, g->current_type, g->current_rotation,
                                     g->current_x + 1, g->current_y)) {
                    g->current_x++;
                    update_ghost(g);
                    if (g->lock_delay_active) g->lock_timer = 0;
                    snd_play_vol(SFX_MOVE, 0.5f);
                }
                g->key_right = true;
                g->das_timer = 0;
                g->das_active = false;
                break;

            case GLFW_KEY_DOWN:
                g->key_down = true;
                break;

            case GLFW_KEY_UP:
            case GLFW_KEY_X:
                rotate_piece(g, 1);
                snd_play(SFX_ROTATE);
                break;

            case GLFW_KEY_Z:
                rotate_piece(g, -1);
                snd_play(SFX_ROTATE);
                break;

            case GLFW_KEY_SPACE:
                hard_drop(g);
                break;

            case GLFW_KEY_C:
                hold_piece(g);
                snd_play(SFX_HOLD);
                break;

            case GLFW_KEY_P:
                g->paused = true;
                snd_play(SFX_PAUSE);
                break;

            case GLFW_KEY_ESCAPE:
                g->paused = true;
                snd_play(SFX_PAUSE);
                save_game(g);
                break;
        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_LEFT:
                g->key_left = false;
                g->das_active = false;
                break;
            case GLFW_KEY_RIGHT:
                g->key_right = false;
                g->das_active = false;
                break;
            case GLFW_KEY_DOWN:
                g->key_down = false;
                break;
        }
    }
}

/* ─────────────────── RENDERING HELPERS ─────────────────── */

static void draw_quad(Renderer *r, GLuint shader, float x, float y,
                      float w, float h, Color color) {
    float mdl[16];
    model_matrix(mdl, x, y, w, h);

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, r->projection);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, mdl);
    glUniform4f(glGetUniformLocation(shader, "uColor"), color.r, color.g, color.b, color.a);

    glBindVertexArray(r->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_glow_block(Renderer *r, GameState *g, float x, float y,
                             float size, Color color, float intensity) {
    float mdl[16];
    float gs = current_theme()->glow_strength;

    /* Scale intensity by theme's glow strength */
    float eff_intensity = intensity * gs;

    if (gs > 0.05f) {
        /* Outer glow (larger, additive) — skip if glow is off */
        float glow_pad = size * 0.3f * gs;
        model_matrix(mdl, x - glow_pad, y - glow_pad, size + glow_pad * 2, size + glow_pad * 2);

        glUseProgram(r->glow_shader);
        glUniformMatrix4fv(r->glow_proj_loc, 1, GL_FALSE, r->projection);
        glUniformMatrix4fv(r->glow_model_loc, 1, GL_FALSE, mdl);
        glUniform4f(r->glow_color_loc, color.r, color.g, color.b, color.a * 0.3f * gs);
        glUniform1f(r->glow_intensity_loc, eff_intensity * 0.5f);
        glUniform1f(r->glow_time_loc, g->glow_pulse);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE); /* Additive blending for glow */
        glBindVertexArray(r->quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    /* Inner block */
    model_matrix(mdl, x + 1, y + 1, size - 2, size - 2);

    if (gs > 0.05f) {
        glUseProgram(r->glow_shader);
        glUniformMatrix4fv(r->glow_proj_loc, 1, GL_FALSE, r->projection);
        glUniformMatrix4fv(r->glow_model_loc, 1, GL_FALSE, mdl);
        glUniform4f(r->glow_color_loc, color.r, color.g, color.b, color.a);
        glUniform1f(r->glow_intensity_loc, eff_intensity);
        glUniform1f(r->glow_time_loc, g->glow_pulse);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(r->quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    } else {
        /* Flat block for no-glow themes */
        draw_quad(r, r->basic_shader, x + 1, y + 1, size - 2, size - 2, color);
        /* Subtle border for definition */
        Color border = {color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, color.a * 0.5f};
        float bw = 1.0f;
        draw_quad(r, r->basic_shader, x, y, size, bw, border);
        draw_quad(r, r->basic_shader, x, y + size - bw, size, bw, border);
        draw_quad(r, r->basic_shader, x, y, bw, size, border);
        draw_quad(r, r->basic_shader, x + size - bw, y, bw, size, border);
    }
}

/* ─── 3D BLOCK — isometric depth extrusion + front face ─── */

static void draw_block_3d(Renderer *r, GameState *g, float x, float y,
                           float size, Color color, float intensity) {
    float depth = 7.0f * g->mode_3d_factor;

    if (depth > 0.3f) {
        int layers = (int)ceilf(depth);
        for (int d = layers; d >= 1; d--) {
            float t = (float)d / (float)(layers + 1);
            /* Darker shade for extrusion; slight gradient from back to front */
            float shade = 0.22f + 0.18f * (1.0f - t);
            Color dark = {color.r * shade, color.g * shade,
                          color.b * shade, color.a * 0.92f};
            /* Extrude upper-right for isometric look */
            draw_quad(r, r->basic_shader,
                      x + (float)d, y - (float)d,
                      size - 1.5f, size - 1.5f, dark);
        }

        /* Top highlight edge on the front face — simulates lit top face */
        float hl = fminf(2.0f, depth * 0.4f);
        Color top_hl = {fminf(1.0f, color.r * 1.4f), fminf(1.0f, color.g * 1.4f),
                        fminf(1.0f, color.b * 1.4f), color.a * 0.45f * g->mode_3d_factor};
        draw_quad(r, r->basic_shader, x + 1, y, size - 2, hl, top_hl);

        /* Right shadow edge — simulates shadowed side face */
        Color side_sh = {color.r * 0.15f, color.g * 0.15f,
                         color.b * 0.15f, color.a * 0.35f * g->mode_3d_factor};
        draw_quad(r, r->basic_shader, x + size - hl, y + 1, hl, size - 2, side_sh);
    }

    /* Front face (normal glow block) */
    draw_glow_block(r, g, x, y, size, color, intensity);
}

/* ─────────────────── RENDER BACKGROUND ─────────────────── */

void render_background(Renderer *r, GameState *g) {
    const Theme *t = current_theme();

    glUseProgram(r->bg_shader);
    glUniform1f(r->bg_time_loc, (float)g->time);
    glUniform2f(r->bg_resolution_loc, WINDOW_WIDTH, WINDOW_HEIGHT);
    glUniform3f(r->bg_top_loc, t->bg_top.r, t->bg_top.g, t->bg_top.b);
    glUniform3f(r->bg_bot_loc, t->bg_bot.r, t->bg_bot.g, t->bg_bot.b);
    glUniform3f(r->bg_accent_loc, t->accent.r, t->accent.g, t->accent.b);
    glUniform1f(r->bg_scan_str_loc, t->scanline_strength);
    glUniform1f(r->bg_mode3d_loc, g->mode_3d_factor);

    /* Pass theme piece colors for falling background blocks */
    float pc[21];
    for (int i = 0; i < 7; i++) {
        pc[i*3 + 0] = t->pieces[i].r;
        pc[i*3 + 1] = t->pieces[i].g;
        pc[i*3 + 2] = t->pieces[i].b;
    }
    glUniform3fv(r->bg_piece_colors_loc, 7, pc);

    glBindVertexArray(r->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

/* ─────────────────── RENDER GRID ───────────────────────── */

void render_grid(Renderer *r, GameState *g) {
    (void)g;
    const Theme *t = current_theme();
    Color gridColor = t->grid_line;
    Color borderColor = t->grid_border;

    /* Grid background */
    draw_quad(r, r->basic_shader,
              GRID_OFFSET_X - 2, GRID_OFFSET_Y - 2,
              GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4,
              t->grid_bg);

    /* Grid lines */
    for (int col = 0; col <= GRID_COLS; col++) {
        float x = GRID_OFFSET_X + col * CELL_SIZE;
        draw_quad(r, r->basic_shader, x, GRID_OFFSET_Y,
                  1, GRID_ROWS * CELL_SIZE, gridColor);
    }
    for (int row = 0; row <= GRID_ROWS; row++) {
        float y = GRID_OFFSET_Y + row * CELL_SIZE;
        draw_quad(r, r->basic_shader, GRID_OFFSET_X, y,
                  GRID_COLS * CELL_SIZE, 1, gridColor);
    }

    /* Neon border */
    float bw = 2.0f;
    float gx = GRID_OFFSET_X - bw;
    float gy = GRID_OFFSET_Y - bw;
    float gw = GRID_COLS * CELL_SIZE + bw * 2;
    float gh = GRID_ROWS * CELL_SIZE + bw * 2;

    draw_quad(r, r->basic_shader, gx, gy, gw, bw, borderColor);
    draw_quad(r, r->basic_shader, gx, gy + gh - bw, gw, bw, borderColor);
    draw_quad(r, r->basic_shader, gx, gy, bw, gh, borderColor);
    draw_quad(r, r->basic_shader, gx + gw - bw, gy, bw, gh, borderColor);
}

/* ─────────────────── RENDER LOCKED BLOCKS ──────────────── */

void render_locked_blocks(Renderer *r, GameState *g) {
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int type = g->grid[row][col];
            if (type == 0) continue;

            Color c = piece_color(type - 1);
            float x = GRID_OFFSET_X + col * CELL_SIZE;
            float y = GRID_OFFSET_Y + row * CELL_SIZE;

            /* Flash animation for clearing lines */
            if (g->line_clearing[row]) {
                float anim = g->line_clear_anim[row];
                c.r = c.r * anim + 1.0f * (1.0f - anim);
                c.g = c.g * anim + 1.0f * (1.0f - anim);
                c.b = c.b * anim + 1.0f * (1.0f - anim);
                c.a = anim;
            }

            draw_block_3d(r, g, x, y, CELL_SIZE, c, 1.0f);
        }
    }
}

/* ─────────────────── RENDER GHOST PIECE ────────────────── */

void render_ghost_piece(Renderer *r, GameState *g) {
    if (g->ghost_y == g->current_y) return;

    Color c = piece_color(g->current_type);
    c.a = 0.2f;

    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[g->current_type][g->current_rotation][row][col]) continue;

            float x = GRID_OFFSET_X + (g->current_x + col) * CELL_SIZE;
            float y = GRID_OFFSET_Y + (g->ghost_y + row) * CELL_SIZE;

            /* Ghost outline only */
            float bw = 1.5f;
            draw_quad(r, r->basic_shader, x, y, CELL_SIZE, bw, c);
            draw_quad(r, r->basic_shader, x, y + CELL_SIZE - bw, CELL_SIZE, bw, c);
            draw_quad(r, r->basic_shader, x, y, bw, CELL_SIZE, c);
            draw_quad(r, r->basic_shader, x + CELL_SIZE - bw, y, bw, CELL_SIZE, c);
        }
    }
}

/* ─────────────────── RENDER CURRENT PIECE ──────────────── */

void render_current_piece(Renderer *r, GameState *g) {
    Color c = piece_color(g->current_type);

    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[g->current_type][g->current_rotation][row][col]) continue;

            int gy = g->current_y + row;
            if (gy < 0) continue;

            float x = GRID_OFFSET_X + (g->current_x + col) * CELL_SIZE;
            float y = GRID_OFFSET_Y + gy * CELL_SIZE;

            draw_block_3d(r, g, x, y, CELL_SIZE, c, 1.2f);
        }
    }
}

/* ─────────────────── RENDER PARTICLES ──────────────────── */

void render_particles(Renderer *r, GameState *g) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); /* Additive blending */

    glUseProgram(r->particle_shader);
    glUniformMatrix4fv(r->particle_proj_loc, 1, GL_FALSE, r->projection);

    for (int i = 0; i < g->particle_count; i++) {
        Particle *p = &g->particles[i];
        if (!p->active) continue;

        float alpha = p->life / p->max_life;
        float mdl[16];
        model_matrix(mdl, p->x - p->size * 0.5f, p->y - p->size * 0.5f, p->size, p->size);

        glUniformMatrix4fv(r->particle_model_loc, 1, GL_FALSE, mdl);
        glUniform4f(r->particle_color_loc, p->color.r, p->color.g, p->color.b, 1.0f);
        glUniform1f(r->particle_alpha_loc, alpha);

        glBindVertexArray(r->quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/* ─────────────────── RENDER MINI PIECE (for UI) ────────── */

static void render_mini_piece(Renderer *r, GameState *g,
                               int type, float ox, float oy, float size) {
    if (type < 0) return;
    Color c = piece_color(type);

    for (int row = 0; row < TETRO_SIZE; row++) {
        for (int col = 0; col < TETRO_SIZE; col++) {
            if (!TETROMINOS[type][0][row][col]) continue;
            float x = ox + col * size;
            float y = oy + row * size;
            draw_glow_block(r, g, x, y, size, c, 0.8f);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
 *  RENDER UI — Score, Next, Hold, Controls, Overlays
 * ═══════════════════════════════════════════════════════════ */

/* 7-segment digit renderer (for score/level numbers — distinctive digital look) */
static void render_digit(Renderer *r, float ox, float oy, int digit, float sc, Color color) {
    static const int segments[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    float w = 12.0f * sc;
    float h = 20.0f * sc;
    float t = 2.5f * sc;
    int seg = segments[digit % 10];

    float segs[7][4] = {
        {ox + t,     oy,           w - 2*t, t},
        {ox + w - t, oy + t,       t,       h/2 - t},
        {ox + w - t, oy + h/2 + t, t,       h/2 - t},
        {ox + t,     oy + h,       w - 2*t, t},
        {ox,         oy + h/2 + t, t,       h/2 - t},
        {ox,         oy + t,       t,       h/2 - t},
        {ox + t,     oy + h/2 - t/2, w - 2*t, t},
    };

    for (int i = 0; i < 7; i++) {
        if (seg & (1 << i)) {
            draw_quad(r, r->basic_shader,
                      segs[i][0], segs[i][1], segs[i][2], segs[i][3], color);
        }
    }
}

static void render_number(Renderer *r, float x, float y, int number, float sc, Color color) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", number);
    int len = (int)strlen(buf);

    for (int i = 0; i < len; i++) {
        render_digit(r, x + i * 16.0f * sc, y, buf[i] - '0', sc, color);
    }
}

/* ─── Controls panel on the left side ─── */
static void render_controls_panel(Renderer *r, GameState *g) {
    float lx = 30;
    float cy = 210;
    float ps = 1.5f;
    float line_h = 22.0f;
    const Theme *t = current_theme();

    Color title_color = {t->ui_primary.r * 0.8f, t->ui_primary.g * 0.8f, t->ui_primary.b * 0.8f, 0.6f};
    Color key_color   = {t->ui_text.r * 0.8f, t->ui_text.g * 0.8f, t->ui_text.b * 0.8f, 0.5f};
    Color desc_color  = {t->ui_secondary.r * 0.6f, t->ui_secondary.g * 0.6f, t->ui_secondary.b * 0.6f, 0.5f};

    render_text_px(r, lx, cy, "CONTROLS", ps, title_color);

    /* Separator line */
    draw_quad(r, r->basic_shader, lx, cy + 14, 100, 1,
              (Color){0.3f, 0.1f, 0.5f, 0.4f});

    cy += 22;
    float desc_x = lx + 70;

    const char *keys[] = {"< >", "DOWN", "SPACE", "UP/X", "Z", "C", "P", "ESC", "F11"};
    const char *descs[] = {"MOVE", "DROP", "HARD", "CW", "CCW", "HOLD", "PAUSE", "MENU", "FULL"};
    int count = 9;

    for (int i = 0; i < count; i++) {
        render_text_px(r, lx, cy + i * line_h, keys[i], ps, key_color);
        render_text_px(r, desc_x, cy + i * line_h, descs[i], ps, desc_color);
    }

    (void)g;
}

void render_ui(Renderer *r, GameState *g) {
    const Theme *t = current_theme();
    Color cyan   = t->ui_primary;
    Color purple = t->ui_secondary;
    Color white  = t->ui_text;

    float right_x = GRID_OFFSET_X + GRID_COLS * CELL_SIZE + 40;
    float left_x = 40;

    /* ─── NEXT PIECE ─── */
    render_text_px(r, right_x, 60, "NEXT", 2.0f, cyan);
    draw_quad(r, r->basic_shader, right_x - 5, 85, 110, 90,
              (Color){0.05f, 0.02f, 0.1f, 0.6f});
    render_mini_piece(r, g, g->next_type, right_x + 15, 100, 18);

    /* ─── HOLD PIECE ─── */
    render_text_px(r, left_x, 60, "HOLD", 2.0f, purple);
    draw_quad(r, r->basic_shader, left_x - 5, 85, 110, 90,
              (Color){0.05f, 0.02f, 0.1f, 0.6f});
    if (g->held_type >= 0) {
        Color hc = piece_color(g->held_type);
        if (!g->can_hold) { hc.a = 0.3f; }
        (void)hc;
        render_mini_piece(r, g, g->held_type, left_x + 15, 100, 18);
    }

    /* ─── SCORE ─── */
    render_text_px(r, right_x, 200, "SCORE", 2.0f, cyan);
    render_number(r, right_x, 228, g->score, 1.5f, white);

    /* ─── LEVEL ─── */
    render_text_px(r, right_x, 285, "LEVEL", 2.0f, purple);
    render_number(r, right_x, 313, g->level, 1.5f, white);

    /* Difficulty badge (small, under level number) */
    {
        Color diff_colors[NUM_DIFFICULTIES] = {
            {0.3f, 1.0f, 0.5f, 0.6f},
            {1.0f, 1.0f, 0.3f, 0.6f},
            {1.0f, 0.5f, 0.1f, 0.6f},
            {1.0f, 0.15f, 0.15f, 0.6f},
        };
        render_text_px(r, right_x, 342, DIFF_NAMES[g->difficulty], 1.2f,
                       diff_colors[g->difficulty]);
    }

    /* ─── LINES ─── */
    render_text_px(r, right_x, 370, "LINES", 2.0f, cyan);
    render_number(r, right_x, 398, g->lines_cleared, 1.5f, white);

    /* ─── COMBO ─── */
    if (g->combo > 1) {
        Color comboColor = {1.0f, 0.8f, 0.0f, 1.0f};
        render_text_px(r, right_x, 455, "COMBO", 2.0f, comboColor);
        render_number(r, right_x, 483, g->combo, 2.0f, comboColor);
    }

    /* ─── CONTROLS PANEL (left side) ─── */
    render_controls_panel(r, g);

    /* ═══ GAME OVER OVERLAY ═══ */
    if (g->game_over) {
        /* Dark overlay */
        draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                  (Color){0.0f, 0.0f, 0.0f, 0.75f});

        /* "GAME OVER" text — large, red, flashing */
        float flash = 0.6f + 0.4f * sinf((float)g->time * 3.0f);
        Color go_color = {1.0f * flash, 0.1f * flash, 0.3f * flash, 1.0f};
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.28f,
                                "GAME OVER", 5.0f, go_color);

        /* Decorative line under title */
        float line_w = 280;
        draw_quad(r, r->basic_shader,
                  WINDOW_WIDTH * 0.5f - line_w * 0.5f, WINDOW_HEIGHT * 0.28f + 25,
                  line_w, 2, (Color){1.0f, 0.2f, 0.4f, flash * 0.5f});

        /* Score label and value */
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.42f,
                                "SCORE", 2.5f, (Color){0.6f, 0.4f, 0.7f, 0.9f});
        char score_buf[16];
        snprintf(score_buf, sizeof(score_buf), "%d", g->score);
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.48f,
                                score_buf, 3.5f, white);

        /* Lines and Level */
        char lines_buf[32], level_buf[32];
        snprintf(lines_buf, sizeof(lines_buf), "LINES: %d", g->lines_cleared);
        snprintf(level_buf, sizeof(level_buf), "LEVEL: %d", g->level);
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.56f,
                                lines_buf, 2.0f, (Color){0.5f, 0.3f, 0.7f, 0.8f});
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.60f,
                                level_buf, 2.0f, (Color){0.5f, 0.3f, 0.7f, 0.8f});

        /* RESTART and MENU buttons */
        float btn_w = 160, btn_h = 48;
        float gap = 24;
        float total_w = btn_w * 2 + gap;
        float start_x = WINDOW_WIDTH * 0.5f - total_w * 0.5f;
        float btn_y = WINDOW_HEIGHT * 0.70f;

        if (draw_button(r, g, start_x, btn_y, btn_w, btn_h, "RESTART", 2.5f,
                         (Color){0.0f, 1.0f, 0.5f, 1.0f},
                         (Color){0.02f, 0.08f, 0.04f, 0.8f},
                         (Color){0.0f, 0.5f, 0.25f, 0.8f})) {
            delete_save();
            init_game(g);
            g->screen = SCREEN_PLAYING;
        }

        if (draw_button(r, g, start_x + btn_w + gap, btn_y, btn_w, btn_h, "MENU", 2.5f,
                         (Color){1.0f, 0.3f, 0.5f, 1.0f},
                         (Color){0.08f, 0.02f, 0.04f, 0.8f},
                         (Color){0.5f, 0.15f, 0.25f, 0.8f})) {
            delete_save();
            g->screen = SCREEN_MENU;
        }

        /* Hint text */
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.82f,
                                "R - RESTART    ESC - MENU", 1.5f,
                                (Color){0.4f, 0.3f, 0.5f, 0.5f});
    }

    /* ═══ PAUSED OVERLAY — rendered separately in render_game for blur ═══ */
}

/* ═══════════════════════════════════════════════════════════
 *  PAUSED OVERLAY — drawn after blur pass in render_game
 * ═══════════════════════════════════════════════════════════ */
static void render_pause_overlay(Renderer *r, GameState *g) {
    /* Slight tinted overlay on top of the blurred background */
    draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
              (Color){0.0f, 0.0f, 0.0f, 0.25f});

    /* "PAUSED" text — large, cyan, pulsing */
    float pulse = 0.7f + 0.3f * sinf((float)g->time * 2.0f);
    Color pause_color = {0.0f, pulse, pulse, 1.0f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.35f,
                            "PAUSED", 5.0f, pause_color);

    /* Decorative line */
    float line_w = 220;
    draw_quad(r, r->basic_shader,
              WINDOW_WIDTH * 0.5f - line_w * 0.5f, WINDOW_HEIGHT * 0.35f + 25,
              line_w, 2, (Color){0.0f, 0.5f, 0.5f, pulse * 0.5f});

    /* RESUME and MENU buttons */
    float btn_w = 160, btn_h = 48;
    float gap = 24;
    float total_w = btn_w * 2 + gap;
    float start_x = WINDOW_WIDTH * 0.5f - total_w * 0.5f;
    float btn_y = WINDOW_HEIGHT * 0.52f;

    if (draw_button(r, g, start_x, btn_y, btn_w, btn_h, "RESUME", 2.5f,
                     (Color){0.0f, 1.0f, 1.0f, 1.0f},
                     (Color){0.02f, 0.05f, 0.08f, 0.8f},
                     (Color){0.0f, 0.4f, 0.5f, 0.8f})) {
        g->paused = false;
    }

    if (draw_button(r, g, start_x + btn_w + gap, btn_y, btn_w, btn_h, "MENU", 2.5f,
                     (Color){0.8f, 0.3f, 1.0f, 1.0f},
                     (Color){0.05f, 0.02f, 0.08f, 0.8f},
                     (Color){0.4f, 0.15f, 0.5f, 0.8f})) {
        save_game(g);
        g->screen = SCREEN_MENU;
        load_game(g);
    }

    /* Hint text */
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.66f,
                            "P - RESUME    ESC - MENU", 1.5f,
                            (Color){0.4f, 0.3f, 0.5f, 0.5f});

    /* Mute & 2D/3D buttons side by side */
    {
        float pair_gap = 16;
        float mute_w = 100, mode_w = 60, pair_h = 34;
        float pair_total = mute_w + pair_gap + mode_w;
        float pair_x = WINDOW_WIDTH * 0.5f - pair_total * 0.5f;
        float pair_y = WINDOW_HEIGHT * 0.74f;

        /* Mute button */
        const char *mute_label = g_snd_muted ? "UNMUTE" : "MUTE";
        Color mt = g_snd_muted
            ? (Color){1.0f, 0.3f, 0.3f, 0.9f}
            : (Color){0.7f, 0.7f, 0.7f, 0.7f};
        Color mbg = {0.05f, 0.05f, 0.05f, 0.7f};
        Color mbr = {mt.r * 0.5f, mt.g * 0.5f, mt.b * 0.5f, 0.5f};
        if (draw_button(r, g, pair_x, pair_y, mute_w, pair_h, mute_label, 2.0f,
                         mt, mbg, mbr)) {
            snd_toggle_mute();
        }

        /* 2D/3D button */
        const char *mode_label = g->mode_3d ? "3D" : "2D";
        Color md_t = g->mode_3d
            ? (Color){0.2f, 1.0f, 0.6f, 0.95f}
            : (Color){0.7f, 0.7f, 0.7f, 0.7f};
        Color md_bg = {0.05f, 0.05f, 0.05f, 0.7f};
        Color md_br = {md_t.r * 0.5f, md_t.g * 0.5f, md_t.b * 0.5f, 0.5f};
        if (draw_button(r, g, pair_x + mute_w + pair_gap, pair_y,
                         mode_w, pair_h, mode_label, 2.2f,
                         md_t, md_bg, md_br)) {
            g->mode_3d = !g->mode_3d;
            g->mode_transition_timer = 0.0f;
            save_config(g);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
 *  MENU SCREEN
 * ═══════════════════════════════════════════════════════════ */

static void render_menu_screen(Renderer *r, GameState *g) {
    const Theme *t = current_theme();
    render_background(r, g);

    /* Semi-transparent overlay for readability */
    draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
              (Color){0.0f, 0.0f, 0.0f, 0.45f});

    /* Title — uses theme primary color */
    float pulse = 0.75f + 0.25f * sinf((float)g->time * 2.0f);
    Color title_color = {t->ui_primary.r * pulse, t->ui_primary.g * pulse,
                         t->ui_primary.b * pulse, 1.0f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 80, "NEON TETRIS", 6.0f, title_color);

    /* Subtitle — theme name */
    Color sub_color = {t->ui_secondary.r * pulse, t->ui_secondary.g * pulse,
                       t->ui_secondary.b * pulse, 0.85f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 142, t->subtitle, 2.5f, sub_color);

    /* Decorative lines */
    float line_w = 350;
    float line_cx = WINDOW_WIDTH * 0.5f;
    Color line_c1 = {t->ui_primary.r * 0.5f, t->ui_primary.g * 0.5f, t->ui_primary.b * 0.5f, 0.3f * pulse};
    Color line_c2 = {t->ui_secondary.r * 0.5f, t->ui_secondary.g * 0.5f, t->ui_secondary.b * 0.5f, 0.3f * pulse};
    draw_quad(r, r->basic_shader, line_cx - line_w * 0.5f, 58, line_w, 1, line_c1);
    draw_quad(r, r->basic_shader, line_cx - line_w * 0.5f, 168, line_w, 1, line_c2);

    /* ─── MENU BUTTONS ─── */
    float btn_w = 220, btn_h = 46;
    float btn_x = WINDOW_WIDTH * 0.5f - btn_w * 0.5f;
    float btn_y = 195;
    float btn_gap = 54;

    /* Derive button colors from theme */
    Color btn_text1 = t->ui_primary;
    Color btn_bg1   = {t->ui_primary.r * 0.08f, t->ui_primary.g * 0.08f, t->ui_primary.b * 0.08f, 0.85f};
    Color btn_brd1  = {t->ui_primary.r * 0.5f, t->ui_primary.g * 0.5f, t->ui_primary.b * 0.5f, 0.8f};
    Color btn_text2 = t->ui_secondary;
    Color btn_bg2   = {t->ui_secondary.r * 0.08f, t->ui_secondary.g * 0.08f, t->ui_secondary.b * 0.08f, 0.85f};
    Color btn_brd2  = {t->ui_secondary.r * 0.5f, t->ui_secondary.g * 0.5f, t->ui_secondary.b * 0.5f, 0.8f};

    /* CONTINUE button (only if save exists) */
    if (g->has_save) {
        Color green = {0.0f, 1.0f, 0.5f, 1.0f};
        if (draw_button(r, g, btn_x, btn_y, btn_w, btn_h, "CONTINUE", 3.0f,
                         green,
                         (Color){0.02f, 0.08f, 0.04f, 0.85f},
                         (Color){0.0f, 0.5f, 0.25f, 0.8f})) {
            g->screen = SCREEN_PLAYING;
            restore_game(g);
            g->screen = SCREEN_PLAYING;
        }
        btn_y += btn_gap;
    }

    /* NEW GAME */
    if (draw_button(r, g, btn_x, btn_y, btn_w, btn_h, "NEW GAME", 3.0f,
                     btn_text1, btn_bg1, btn_brd1)) {
        delete_save();
        g->screen = SCREEN_PLAYING;
        init_game(g);
        g->screen = SCREEN_PLAYING;
    }
    btn_y += btn_gap;

    /* ─── DIFFICULTY SELECTOR ─── */
    btn_y += 10;  /* extra spacing to clear button hover glow */
    {
        Color diff_section_c = {t->ui_primary.r * 0.8f, t->ui_primary.g * 0.8f,
                                t->ui_primary.b * 0.8f, 0.6f};
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, btn_y, "DIFFICULTY", 2.0f, diff_section_c);

        float da_w = 40, da_h = 36;
        float dname_area = 160;
        float dtotal = da_w + dname_area + da_w;
        float dsx = WINDOW_WIDTH * 0.5f - dtotal * 0.5f;
        float dty = btn_y + 22;

        /* Left arrow */
        if (draw_button(r, g, dsx, dty, da_w, da_h, "<", 3.0f,
                         t->ui_text, btn_bg1, btn_brd1)) {
            g->difficulty = (Difficulty)((g->difficulty - 1 + NUM_DIFFICULTIES) % NUM_DIFFICULTIES);
            save_config(g);
        }

        /* Difficulty name — color-coded */
        Color diff_colors[NUM_DIFFICULTIES] = {
            {0.3f, 1.0f, 0.5f, 1.0f},   /* EASY   — green  */
            {1.0f, 1.0f, 0.3f, 1.0f},   /* NORMAL — yellow */
            {1.0f, 0.5f, 0.1f, 1.0f},   /* HARD   — orange */
            {1.0f, 0.15f, 0.15f, 1.0f}, /* INSANE — red    */
        };
        render_text_centered_px(r, dsx + da_w + dname_area * 0.5f, dty + da_h * 0.5f,
                                DIFF_NAMES[g->difficulty], 2.2f, diff_colors[g->difficulty]);

        /* Right arrow */
        if (draw_button(r, g, dsx + da_w + dname_area, dty, da_w, da_h, ">", 3.0f,
                         t->ui_text, btn_bg1, btn_brd1)) {
            g->difficulty = (Difficulty)((g->difficulty + 1) % NUM_DIFFICULTIES);
            save_config(g);
        }

        btn_y = dty + da_h + 14;
    }

    /* HIGH SCORES */
    Color gold = {1.0f, 0.8f, 0.0f, 1.0f};
    if (draw_button(r, g, btn_x, btn_y, btn_w, btn_h, "HIGH SCORES", 2.5f,
                     gold,
                     (Color){0.08f, 0.06f, 0.02f, 0.85f},
                     (Color){0.5f, 0.4f, 0.0f, 0.8f})) {
        g->screen = SCREEN_HIGHSCORES;
    }
    btn_y += btn_gap;

    /* ABOUT */
    if (draw_button(r, g, btn_x, btn_y, btn_w, btn_h, "ABOUT", 3.0f,
                     btn_text2, btn_bg2, btn_brd2)) {
        g->screen = SCREEN_ABOUT;
    }
    btn_y += btn_gap;

    /* ─── THEME SELECTOR ─── */
    float theme_y = btn_y + 5;
    Color section_color = {t->ui_primary.r * 0.8f, t->ui_primary.g * 0.8f,
                           t->ui_primary.b * 0.8f, 0.6f};

    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, theme_y, "THEME", 2.0f, section_color);

    /* < THEME NAME > with arrow buttons */
    float arrow_w = 40, arrow_h = 36;
    float name_area = 220;
    float total = arrow_w + name_area + arrow_w;
    float sx = WINDOW_WIDTH * 0.5f - total * 0.5f;
    float ty = theme_y + 22;

    /* Left arrow */
    if (draw_button(r, g, sx, ty, arrow_w, arrow_h, "<", 3.0f,
                     t->ui_text, btn_bg1, btn_brd1)) {
        g->current_theme = (g->current_theme - 1 + NUM_THEMES) % NUM_THEMES;
        save_config(g);
    }

    /* Theme name display */
    Color name_c = {t->ui_text.r, t->ui_text.g, t->ui_text.b, 0.9f};
    render_text_centered_px(r, sx + arrow_w + name_area * 0.5f, ty + arrow_h * 0.5f,
                            t->name, 2.0f, name_c);

    /* Right arrow */
    if (draw_button(r, g, sx + arrow_w + name_area, ty, arrow_w, arrow_h, ">", 3.0f,
                     t->ui_text, btn_bg1, btn_brd1)) {
        g->current_theme = (g->current_theme + 1) % NUM_THEMES;
        save_config(g);
    }

    /* Mini preview — show piece color swatches */
    float swatch_size = 12.0f;
    float swatch_gap = 4.0f;
    float swatch_total = NUM_TETROMINOS * (swatch_size + swatch_gap) - swatch_gap;
    float swatch_x = WINDOW_WIDTH * 0.5f - swatch_total * 0.5f;
    float swatch_y = ty + arrow_h + 8;
    for (int i = 0; i < NUM_TETROMINOS; i++) {
        Color pc = t->pieces[i];
        draw_quad(r, r->basic_shader,
                  swatch_x + i * (swatch_size + swatch_gap), swatch_y,
                  swatch_size, swatch_size, pc);
    }

    /* ─── CONTROLS SECTION (compact) ─── */
    float ctrl_top = swatch_y + swatch_size + 16;

    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, ctrl_top, "CONTROLS", 2.0f, section_color);
    Color sep_c = {t->ui_secondary.r * 0.3f, t->ui_secondary.g * 0.3f, t->ui_secondary.b * 0.3f, 0.4f};
    draw_quad(r, r->basic_shader, WINDOW_WIDTH * 0.5f - 100, ctrl_top + 18, 200, 1, sep_c);

    float ctrl_x = WINDOW_WIDTH * 0.5f - 170;
    float desc_x = WINDOW_WIDTH * 0.5f + 10;
    float cy = ctrl_top + 28;
    float line_h = 18.0f;
    float ps = 1.4f;

    Color key_color  = {t->ui_text.r, t->ui_text.g, t->ui_text.b, 0.7f};
    Color desc_color = {t->ui_secondary.r * 0.6f, t->ui_secondary.g * 0.6f,
                        t->ui_secondary.b * 0.6f, 0.6f};

    const char *keys[]  = {"< >",   "DOWN",  "SPACE", "UP / X", "Z",     "C",    "P",   "ESC",  "F11"};
    const char *descs[] = {"MOVE",   "SOFT DROP", "HARD DROP", "ROTATE CW", "ROTATE CCW", "HOLD PIECE", "PAUSE", "MENU", "FULLSCREEN"};

    for (int i = 0; i < 9; i++) {
        render_text_px(r, ctrl_x, cy + i * line_h, keys[i], ps, key_color);
        render_text_px(r, desc_x, cy + i * line_h, descs[i], ps, desc_color);
    }

    /* ─── 2D/3D MODE BUTTON (bottom-left) ─── */
    {
        const char *mode_label = g->mode_3d ? "3D" : "2D";
        float md_w = 60, md_h = 32;
        float md_x = 20;
        float md_y = WINDOW_HEIGHT - md_h - 35;
        Color mode_text = g->mode_3d
            ? (Color){0.2f, 1.0f, 0.6f, 0.95f}
            : (Color){t->ui_text.r * 0.7f, t->ui_text.g * 0.7f, t->ui_text.b * 0.7f, 0.7f};
        Color mode_bg  = {0.05f, 0.05f, 0.05f, 0.7f};
        Color mode_brd = {mode_text.r * 0.5f, mode_text.g * 0.5f, mode_text.b * 0.5f, 0.5f};
        if (draw_button(r, g, md_x, md_y, md_w, md_h, mode_label, 2.2f,
                         mode_text, mode_bg, mode_brd)) {
            g->mode_3d = !g->mode_3d;
            g->mode_transition_timer = 0.0f;
            save_config(g);
        }
    }

    /* ─── MUTE BUTTON (bottom-right) ─── */
    {
        const char *mute_label = g_snd_muted ? "UNMUTE" : "MUTE";
        float mb_w = 90, mb_h = 32;
        float mb_x = WINDOW_WIDTH - mb_w - 20;
        float mb_y = WINDOW_HEIGHT - mb_h - 35;
        Color mute_text = g_snd_muted
            ? (Color){1.0f, 0.3f, 0.3f, 0.9f}
            : (Color){t->ui_text.r * 0.7f, t->ui_text.g * 0.7f, t->ui_text.b * 0.7f, 0.7f};
        Color mute_bg  = {0.05f, 0.05f, 0.05f, 0.7f};
        Color mute_brd = {mute_text.r * 0.5f, mute_text.g * 0.5f, mute_text.b * 0.5f, 0.5f};
        if (draw_button(r, g, mb_x, mb_y, mb_w, mb_h, mute_label, 1.8f,
                         mute_text, mute_bg, mute_brd)) {
            snd_toggle_mute();
        }
    }

    /* Credits */
    Color credit_c = {t->ui_secondary.r * 0.4f, t->ui_secondary.g * 0.4f,
                      t->ui_secondary.b * 0.4f, 0.35f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 20,
                            "IIIT KALYANI - BTECH CSE - 2026", 1.2f, credit_c);
}

/* ═══════════════════════════════════════════════════════════
 *  HIGH SCORES SCREEN
 * ═══════════════════════════════════════════════════════════ */

static void render_highscores_screen(Renderer *r, GameState *g) {
    render_background(r, g);
    draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
              (Color){0.0f, 0.0f, 0.0f, 0.55f});

    float pulse = 0.8f + 0.2f * sinf((float)g->time * 2.0f);
    Color gold = {1.0f * pulse, 0.8f * pulse, 0.0f, 1.0f};

    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 80, "HIGH SCORES", 5.0f, gold);

    /* Decorative line */
    float lw = 300;
    draw_quad(r, r->basic_shader, WINDOW_WIDTH * 0.5f - lw * 0.5f, 110, lw, 2,
              (Color){0.5f, 0.4f, 0.0f, 0.5f * pulse});

    HighScoreTable *ht = &g->high_scores;

    if (ht->count == 0) {
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 300,
                                "NO SCORES YET", 2.5f,
                                (Color){0.5f, 0.4f, 0.6f, 0.6f});
        render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 350,
                                "PLAY A GAME!", 2.0f,
                                (Color){0.4f, 0.3f, 0.5f, 0.5f});
    } else {
        /* Column headers */
        float header_y = 145;
        float rank_x = WINDOW_WIDTH * 0.5f - 230;
        float score_x = WINDOW_WIDTH * 0.5f - 120;
        float lines_x = WINDOW_WIDTH * 0.5f + 50;
        float level_x = WINDOW_WIDTH * 0.5f + 170;

        Color hdr = {0.0f, 0.8f, 0.8f, 0.7f};
        render_text_px(r, rank_x, header_y, "RANK", 2.0f, hdr);
        render_text_px(r, score_x, header_y, "SCORE", 2.0f, hdr);
        render_text_px(r, lines_x, header_y, "LINES", 2.0f, hdr);
        render_text_px(r, level_x, header_y, "LVL", 2.0f, hdr);

        draw_quad(r, r->basic_shader, rank_x, header_y + 18,
                  level_x + 60 - rank_x, 1, (Color){0.2f, 0.1f, 0.4f, 0.4f});

        /* Entries */
        for (int i = 0; i < ht->count; i++) {
            float ey = 175 + i * 42;
            char buf[32];
            float ps = 2.2f;

            /* Rank colors: gold, silver, bronze, then white */
            Color rc;
            if (i == 0)      rc = (Color){1.0f, 0.85f, 0.0f, 1.0f};
            else if (i == 1) rc = (Color){0.8f, 0.8f, 0.85f, 0.9f};
            else if (i == 2) rc = (Color){0.8f, 0.5f, 0.2f, 0.9f};
            else             rc = (Color){0.7f, 0.6f, 0.8f, 0.7f};

            /* Row background for top 3 */
            if (i < 3) {
                draw_quad(r, r->basic_shader, rank_x - 10, ey - 4,
                          level_x + 70 - rank_x, 28,
                          (Color){rc.r * 0.1f, rc.g * 0.1f, rc.b * 0.1f, 0.3f});
            }

            snprintf(buf, sizeof(buf), "%d.", i + 1);
            render_text_px(r, rank_x, ey, buf, ps, rc);

            snprintf(buf, sizeof(buf), "%d", ht->entries[i].score);
            render_text_px(r, score_x, ey, buf, ps, rc);

            snprintf(buf, sizeof(buf), "%d", ht->entries[i].lines);
            render_text_px(r, lines_x, ey, buf, ps, rc);

            snprintf(buf, sizeof(buf), "%d", ht->entries[i].level);
            render_text_px(r, level_x, ey, buf, ps, rc);
        }
    }

    /* BACK button */
    if (draw_button(r, g, WINDOW_WIDTH * 0.5f - 80, WINDOW_HEIGHT - 80, 160, 45, "BACK", 2.5f,
                     (Color){0.0f, 1.0f, 1.0f, 1.0f},
                     (Color){0.02f, 0.05f, 0.08f, 0.8f},
                     (Color){0.0f, 0.4f, 0.5f, 0.8f})) {
        g->screen = SCREEN_MENU;
    }

    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 25,
                            "PRESS ANY KEY TO RETURN", 1.2f,
                            (Color){0.4f, 0.3f, 0.5f, 0.4f});
}

/* ═══════════════════════════════════════════════════════════
 *  ABOUT SCREEN — Team Credits
 * ═══════════════════════════════════════════════════════════ */

static void render_about_screen(Renderer *r, GameState *g) {
    render_background(r, g);
    draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
              (Color){0.0f, 0.0f, 0.0f, 0.6f});

    float pulse = 0.8f + 0.2f * sinf((float)g->time * 2.0f);

    /* Title */
    Color title_c = {0.0f, pulse, pulse, 1.0f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 70, "NEON TETRIS", 5.0f, title_c);

    Color sub_c = {0.7f * pulse, 0.2f * pulse, 0.9f * pulse, 0.8f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 125, "CYBERPUNK EDITION", 2.0f, sub_c);

    /* Decorative line */
    float lw = 350;
    draw_quad(r, r->basic_shader, WINDOW_WIDTH * 0.5f - lw * 0.5f, 150, lw, 2,
              (Color){0.4f, 0.1f, 0.5f, 0.4f * pulse});

    /* Project info */
    Color info_c = {0.6f, 0.5f, 0.8f, 0.8f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 175, "IIIT KALYANI - BTECH CSE - 2026", 2.0f, info_c);

    /* "TEAM" header */
    Color team_header = {0.0f, 1.0f, 1.0f, 0.9f};
    render_text_centered_px(r, WINDOW_WIDTH * 0.5f, 230, "DEVELOPMENT TEAM", 3.0f, team_header);

    draw_quad(r, r->basic_shader, WINDOW_WIDTH * 0.5f - 140, 255, 280, 1,
              (Color){0.0f, 0.5f, 0.5f, 0.5f});

    /* Team members */
    float cx = WINDOW_WIDTH * 0.5f;
    float my = 285;
    float member_gap = 100;
    float name_ps = 2.8f;
    float role_ps = 1.5f;

    /* Rishi Saha — Lead Developer */
    Color lead_name  = {0.0f, 1.0f, 1.0f, 1.0f};
    Color lead_role  = {0.0f, 0.7f, 0.7f, 0.7f};
    Color lead_desc  = {0.4f, 0.5f, 0.6f, 0.6f};
    render_text_centered_px(r, cx, my, "RISHI SAHA", name_ps, lead_name);
    render_text_centered_px(r, cx, my + 26, "LEAD DEVELOPER", role_ps, lead_role);
    render_text_centered_px(r, cx, my + 42, "GRAPHICS OPS + FOUNDATION CODEBASE", role_ps, lead_desc);
    /* Star indicator for lead */
    draw_quad(r, r->basic_shader, cx - 80, my - 8, 4, 4, lead_name);
    draw_quad(r, r->basic_shader, cx + 76, my - 8, 4, 4, lead_name);

    /* Arko Kabiraj */
    my += member_gap;
    Color mem_name = {0.8f, 0.3f, 1.0f, 1.0f};
    Color mem_role = {0.6f, 0.2f, 0.7f, 0.7f};
    render_text_centered_px(r, cx, my, "ARKO KABIRAJ", name_ps, mem_name);
    render_text_centered_px(r, cx, my + 26, "GRAPHIC ELEMENTS DESIGNER", role_ps, mem_role);

    /* Suhaan Das */
    my += member_gap;
    Color mem2_name = {0.0f, 1.0f, 0.5f, 1.0f};
    Color mem2_role = {0.0f, 0.7f, 0.35f, 0.7f};
    render_text_centered_px(r, cx, my, "SUHAAN DAS", name_ps, mem2_name);
    render_text_centered_px(r, cx, my + 26, "GRAPHIC DESIGN COUPLER - OPENGL", role_ps, mem2_role);

    /* Ayush Mukherjee */
    my += member_gap;
    Color mem3_name = {1.0f, 0.5f, 0.0f, 1.0f};
    Color mem3_role = {0.7f, 0.35f, 0.0f, 0.7f};
    render_text_centered_px(r, cx, my, "AYUSH MUKHERJEE", name_ps, mem3_name);
    render_text_centered_px(r, cx, my + 26, "SHADER MATHEMATICS + LOGIC", role_ps, mem3_role);

    /* Decorative bottom line */
    draw_quad(r, r->basic_shader, WINDOW_WIDTH * 0.5f - lw * 0.5f, my + 60, lw, 1,
              (Color){0.4f, 0.1f, 0.5f, 0.3f * pulse});

    /* Built with line */
    render_text_centered_px(r, cx, my + 78,
                            "BUILT WITH C + OPENGL 3.3 + GLFW + GLEW", 1.3f,
                            (Color){0.3f, 0.2f, 0.4f, 0.4f});

    /* BACK button */
    if (draw_button(r, g, cx - 80, WINDOW_HEIGHT - 70, 160, 45, "BACK", 2.5f,
                     (Color){0.0f, 1.0f, 1.0f, 1.0f},
                     (Color){0.02f, 0.05f, 0.08f, 0.8f},
                     (Color){0.0f, 0.4f, 0.5f, 0.8f})) {
        g->screen = SCREEN_MENU;
    }

    render_text_centered_px(r, cx, WINDOW_HEIGHT - 15,
                            "PRESS ANY KEY TO RETURN", 1.2f,
                            (Color){0.4f, 0.3f, 0.5f, 0.4f});
}

/* ─────────────────── BLOOM POST-PROCESSING ─────────────── */

static void render_bloom(Renderer *r, GameState *g) {
    const Theme *t = &THEMES[g->current_theme];
    int vw = r->vp_w > 0 ? r->vp_w : 1;
    int vh = r->vp_h > 0 ? r->vp_h : 1;
    int half_w = vw / 2;
    int half_h = vh / 2;
    if (half_w < 1) half_w = 1;
    if (half_h < 1) half_h = 1;

    /* Bright pass: scene → blur[0] at half res */
    glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[0]);
    glViewport(0, 0, half_w, half_h);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(r->bright_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->scene_tex);
    glUniform1i(glGetUniformLocation(r->bright_shader, "uScene"), 0);
    glUniform1f(r->bright_threshold_loc, 0.65f);

    glBindVertexArray(r->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* Ping-pong Gaussian blur (4 passes = nice soft bloom) */
    glUseProgram(r->blur_shader);
    glUniform1i(r->blur_image_loc, 0);

    for (int i = 0; i < 4; i++) {
        /* Horizontal blur: blur[0] → blur[1] */
        glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, r->blur_tex[0]);
        glUniform2f(r->blur_direction_loc, 1.0f / (float)half_w, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        /* Vertical blur: blur[1] → blur[0] */
        glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[0]);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, r->blur_tex[1]);
        glUniform2f(r->blur_direction_loc, 0.0f, 1.0f / (float)half_h);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    /* Composite: scene + bloom → default framebuffer (letterboxed viewport) */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(r->vp_x, r->vp_y, r->vp_w, r->vp_h);

    glUseProgram(r->composite_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->scene_tex);
    glUniform1i(glGetUniformLocation(r->composite_shader, "uScene"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r->blur_tex[0]);
    glUniform1i(glGetUniformLocation(r->composite_shader, "uBloom"), 1);
    glUniform1f(r->composite_strength_loc, t->bloom_strength);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glActiveTexture(GL_TEXTURE0);
}

/* ─────────────────── MAIN RENDER PASS ──────────────────── */

void render_game(Renderer *r, GameState *g) {
    const Theme *t = &THEMES[g->current_theme];
    bool use_bloom = t->bloom_strength > 0.01f;
    bool is_paused = g->paused && !g->game_over && g->screen == SCREEN_PLAYING;

    /* Clear the full framebuffer to black (letterbox bars) */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, r->fb_width, r->fb_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Use FBO when bloom is active OR when paused (for blur effect) */
    bool use_fbo = use_bloom || is_paused;

    if (use_fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, r->scene_fbo);
        glViewport(0, 0, r->vp_w, r->vp_h);
    } else {
        glViewport(r->vp_x, r->vp_y, r->vp_w, r->vp_h);
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Non-gameplay screens */
    if (g->screen != SCREEN_PLAYING) {
        ortho_matrix(r->projection, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        switch (g->screen) {
            case SCREEN_MENU:       render_menu_screen(r, g); break;
            case SCREEN_HIGHSCORES: render_highscores_screen(r, g); break;
            case SCREEN_ABOUT:      render_about_screen(r, g); break;
            default: break;
        }
    } else {
        /* Screen shake offset */
        float shake_x = 0, shake_y = 0;
        if (g->shake_timer > 0) {
            shake_x = ((float)rand() / (float)RAND_MAX - 0.5f) * g->shake_intensity * 2;
            shake_y = ((float)rand() / (float)RAND_MAX - 0.5f) * g->shake_intensity * 2;
            float proj[16];
            ortho_matrix(proj, -shake_x, WINDOW_WIDTH - shake_x,
                         WINDOW_HEIGHT - shake_y, -shake_y, -1, 1);
            memcpy(r->projection, proj, sizeof(proj));
        } else {
            ortho_matrix(r->projection, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        }

        render_background(r, g);
        render_grid(r, g);
        render_ghost_piece(r, g);
        render_locked_blocks(r, g);
        render_current_piece(r, g);
        render_particles(r, g);
        render_ui(r, g);

        /* ─── Pause blur: blur the scene, then draw overlay on top ─── */
        if (is_paused) {
            /* Scene is now in scene_tex — blur it using the ping-pong pipeline */
            int vw = r->vp_w > 0 ? r->vp_w : 1;
            int vh = r->vp_h > 0 ? r->vp_h : 1;
            int half_w = vw / 2, half_h = vh / 2;
            if (half_w < 1) half_w = 1;
            if (half_h < 1) half_h = 1;

            /* Downsample scene → blur[0] (just copy via bright pass with threshold 0) */
            glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[0]);
            glViewport(0, 0, half_w, half_h);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(r->bright_shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r->scene_tex);
            glUniform1i(glGetUniformLocation(r->bright_shader, "uScene"), 0);
            glUniform1f(r->bright_threshold_loc, -1.0f);  /* pass everything through */

            glBindVertexArray(r->quad_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            /* Gaussian blur — 6 passes for a strong, smooth blur */
            glUseProgram(r->blur_shader);
            glUniform1i(r->blur_image_loc, 0);

            for (int i = 0; i < 6; i++) {
                /* Horizontal: blur[0] → blur[1] */
                glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[1]);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, r->blur_tex[0]);
                glUniform2f(r->blur_direction_loc, 1.0f / (float)half_w, 0.0f);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                /* Vertical: blur[1] → blur[0] */
                glBindFramebuffer(GL_FRAMEBUFFER, r->blur_fbo[0]);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, r->blur_tex[1]);
                glUniform2f(r->blur_direction_loc, 0.0f, 1.0f / (float)half_h);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            /* Draw the blurred scene back to scene_fbo as the background */
            glBindFramebuffer(GL_FRAMEBUFFER, r->scene_fbo);
            glViewport(0, 0, vw, vh);
            glClear(GL_COLOR_BUFFER_BIT);

            /* Blit blurred texture as fullscreen quad using composite shader
               (with bloom strength 0 = just show the scene texture) */
            glUseProgram(r->composite_shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r->blur_tex[0]);  /* blurred as "scene" */
            glUniform1i(glGetUniformLocation(r->composite_shader, "uScene"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, r->blur_tex[0]);  /* same — no extra bloom */
            glUniform1i(glGetUniformLocation(r->composite_shader, "uBloom"), 1);
            glUniform1f(r->composite_strength_loc, 0.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glActiveTexture(GL_TEXTURE0);

            /* Reset projection and draw the pause overlay (sharp, on top of blur) */
            ortho_matrix(r->projection, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
            render_pause_overlay(r, g);
        }
    }

    /* ─── Fog overlay for 2D/3D transition ─── */
    if (g->fog_alpha > 0.01f) {
        ortho_matrix(r->projection, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        const Theme *th = current_theme();
        /* Dark base layer */
        draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                  (Color){0.0f, 0.0f, 0.0f, g->fog_alpha * 0.55f});
        /* Tinted mist layer (theme-colored) */
        float mr = fminf(1.0f, (th->ui_primary.r + 1.0f) * 0.5f);
        float mg = fminf(1.0f, (th->ui_primary.g + 1.0f) * 0.5f);
        float mb = fminf(1.0f, (th->ui_primary.b + 1.0f) * 0.5f);
        draw_quad(r, r->basic_shader, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                  (Color){mr, mg, mb, g->fog_alpha * 0.25f});
    }

    /* Apply bloom post-processing (also handles non-paused FBO blit) */
    if (use_bloom) {
        render_bloom(r, g);
    } else if (is_paused) {
        /* No bloom, but we rendered to FBO for pause blur — blit to screen */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(r->vp_x, r->vp_y, r->vp_w, r->vp_h);

        glUseProgram(r->composite_shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->scene_tex);
        glUniform1i(glGetUniformLocation(r->composite_shader, "uScene"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->scene_tex);
        glUniform1i(glGetUniformLocation(r->composite_shader, "uBloom"), 1);
        glUniform1f(r->composite_strength_loc, 0.0f);

        glBindVertexArray(r->quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glActiveTexture(GL_TEXTURE0);
    }
}

/* ═══════════════════════════════════════════════════════════
 *  TERMINAL BOX — dynamic width calculation for alignment
 * ═══════════════════════════════════════════════════════════ */

static int utf8_display_width(const char *s) {
    int width = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        if (*p < 0x80)       { p++;     width++; }
        else if (*p < 0xC0)  { p++;              }
        else if (*p < 0xE0)  { p += 2;  width++; }
        else if (*p < 0xF0)  { p += 3;  width++; }
        else                 { p += 4;  width++; }
    }
    return width;
}

static void print_terminal_box(void) {
    const char *lines[] = {
        "     \xe2\x97\x88  N E O N   T E T R I S  \xe2\x97\x88     ",   /* ◈ */
        "           Cyberpunk Edition           ",
        NULL,  /* separator */
        "  < >       Move",
        "  DOWN      Soft Drop",
        "  SPACE     Hard Drop",
        "  UP / X    Rotate CW",
        "  Z         Rotate CCW",
        "  C         Hold Piece",
        "  P         Pause",
        "  R         Restart (game over)",
        "  F11       Toggle Fullscreen",
        "  ESC       Quit",
    };
    int n = (int)(sizeof(lines) / sizeof(lines[0]));

    /* Find max display width */
    int max_w = 0;
    for (int i = 0; i < n; i++) {
        if (!lines[i]) continue;
        int w = utf8_display_width(lines[i]);
        if (w > max_w) max_w = w;
    }

    /* Top border */
    printf("\n  \xe2\x95\x94");                          /* ╔ */
    for (int i = 0; i < max_w; i++) printf("\xe2\x95\x90");  /* ═ */
    printf("\xe2\x95\x97\n");                            /* ╗ */

    /* Lines */
    for (int i = 0; i < n; i++) {
        if (!lines[i]) {
            /* Separator */
            printf("  \xe2\x95\xa0");                    /* ╠ */
            for (int j = 0; j < max_w; j++) printf("\xe2\x95\x90");
            printf("\xe2\x95\xa3\n");                    /* ╣ */
        } else {
            int w = utf8_display_width(lines[i]);
            int pad = max_w - w;
            printf("  \xe2\x95\x91%s", lines[i]);       /* ║ */
            for (int j = 0; j < pad; j++) putchar(' ');
            printf("\xe2\x95\x91\n");                    /* ║ */
        }
    }

    /* Bottom border */
    printf("  \xe2\x95\x9a");                            /* ╚ */
    for (int i = 0; i < max_w; i++) printf("\xe2\x95\x90");
    printf("\xe2\x95\x9d\n\n");                          /* ╝ */
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */

int main(void) {
    GLFWwindow *window = init_window();

    /* Register all input callbacks */
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    Renderer renderer;
    init_renderer(&renderer, window);
    g_renderer = &renderer;

    /* Register resize callback (after renderer is initialized) */
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Start on menu screen */
    memset(&g_game, 0, sizeof(GameState));
    g_game.screen = SCREEN_MENU;
    g_game.held_type = -1;
    g_game.mode_transition_timer = -1.0f;

    /* Initialize 8-bit sound engine */
    if (snd_init() == 0) {
        snd_generate_all();
    } else {
        fprintf(stderr, "Warning: Audio init failed — continuing without sound\n");
    }

    /* Load persistent data */
    load_high_scores(&g_game.high_scores);
    load_game(&g_game);
    load_config(&g_game);

    double last_time = glfwGetTime();

    /* Print controls to terminal */
    print_terminal_box();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double dt = now - last_time;
        last_time = now;

        /* Cap delta time to avoid spiral of death */
        if (dt > 0.05) dt = 0.05;

        glfwPollEvents();
        update_game(&g_game, dt);
        render_game(&renderer, &g_game);
        glfwSwapBuffers(window);

        /* Reset mouse click after frame — ensures one-frame consumption */
        g_game.mouse_clicked = false;
    }

    /* Auto-save in-progress game on quit */
    if (g_game.screen == SCREEN_PLAYING && !g_game.game_over) {
        save_game(&g_game);
    }

    snd_cleanup();
    cleanup_renderer(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
