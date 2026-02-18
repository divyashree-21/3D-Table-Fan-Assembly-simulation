// tablefan_assembly.c
// 3D Table Fan — Part-by-part assembly + mouse rotate + keyboard controls
// Works with MSYS2 MinGW64: GLFW + GLU + stb_image_write
//
// Controls:
//  - Left-drag mouse: rotate the fan object (yaw + pitch)
//  - Scroll: zoom in/out
//  - A: toggle automatic assembly playback
//  - 1..6: jump to step 1..6 (Base, Stand, Motor, Blades, Guard, Final/Spin)
//  - R: reset (go to step 0 - nothing assembled)
//  - S: save screenshot (saved in OUTDIR)
//  - ESC or window close: exit
//
// Compile:
// gcc tablefan_assembly.c -o tablefan_assembly.exe -lglfw3 -lglu32 -lopengl32 -lgdi32 -lm
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GLFW/glfw3.h>
#include <GL/glu.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// ---------- CONFIG ----------
const char *OUTDIR = "C:\\Users\\Divya gowda\\OneDrive\\Documents\\project.c\\render_out";
static const float fan_center_z = 0.60f;

// ---------- Interaction state ----------
static int dragging = 0;
static double last_mouse_x = 0.0, last_mouse_y = 0.0;
static float fan_yaw = 0.0f;   // degrees (rotate around Z)
static float fan_pitch = 0.0f; // degrees (rotate around X)
static float camera_distance = 2.0f;

// ---------- Animation / assembly state ----------
enum Step { STEP_NONE=0, STEP_BASE=1, STEP_STAND=2, STEP_MOTOR=3, STEP_BLADES=4, STEP_GUARD=5, STEP_FINAL=6 };
static int current_step = STEP_NONE;   // current completed step (0..6)
static int target_step = STEP_NONE;    // where autoplay will go
static int autoplay = 1;               // 1 = playing, 0 = paused

// Each part has a progress value 0..1 (0 off-screen/unattached, 1 attached)
static float prog_base = 0.0f;
static float prog_stand = 0.0f;

static float prog_motor = 0.0f;
static float prog_blades = 0.0f;
static float prog_guard = 0.0f;

// blade spin (degrees)
static float blade_angle = 0.0f;
static float blade_speed = 360.0f * 1.2f; // deg/sec when spinning

// timing control
static double play_time = 0.0;
static float assemble_speed = 0.9f; // speed factor for interpolation

// ---------- helper: save framebuffer ----------
static void save_frame(const char *path, int w, int h)
{
    unsigned char *buf = malloc(w*h*3);
    unsigned char *flip = malloc(w*h*3);
    if (!buf || !flip) { free(buf); free(flip); return; }

    glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,buf);
    for (int y=0;y<h;y++) memcpy(flip + (h-1-y)*w*3, buf + y*w*3, w*3);
    stbi_write_png(path, w, h, 3, flip, w*3);
    free(buf); free(flip);
}

// ---------- simple geometry primitives ----------
static void draw_blade_geom(float L, float rw, float tw, float tiltDeg)
{
    float t = tiltDeg * (float)M_PI/180.0f;
    glBegin(GL_QUADS);
      glNormal3f(0, sinf(t), cosf(t));
      glVertex3f(-rw/2, 0, 0);
      glVertex3f( rw/2, 0, 0);
      glNormal3f(0, sinf(t), cosf(t));
      glVertex3f( tw/2, L*cosf(t), L*sinf(t));
      glVertex3f(-tw/2, L*cosf(t), L*sinf(t));
    glEnd();
}

static void draw_guard_geom(float r)
{
    for (int ring=0; ring<3; ++ring) {
        float rr = r*(1.0f - 0.1f*ring);
        glBegin(GL_LINE_LOOP);
          for (int a=0;a<360;a++){
              float rad = a*(float)M_PI/180.0f;
              glVertex3f(cosf(rad)*rr, sinf(rad)*rr, 0.02f);
          }
        glEnd();
        
    }
    for (int s=0;s<12;s++){
        float ang = s*(360.0f/12.0f)*(float)M_PI/180.0f;
        glBegin(GL_LINES);
          glVertex3f(0,0,0.02f);
          glVertex3f(cosf(ang)*r, sinf(ang)*r, 0.02f);
        glEnd();
    }
}

// ---------- draw each part using its progress (0..1) ----------
// We move parts from off-screen positions into place using simple lerp.
// base: moves up from z = -0.6 to z=0.02
// stand: slides up along z from z=-0.6 to 0.15 and then rotates into place
// motor: slides along -y to 0 (toward camera) and up into 0.60
// blades: fade/scale into place (we use translate from +0.5 to correct root translation)
// guard: translate from +0.5 forward into center

static void draw_base_part(float p) {
    // p: 0..1
    GLUquadric *q = gluNewQuadric();
    float z = -0.6f * (1.0f - p) + 0.02f * p; // from -0.6 to 0.02
    glPushMatrix();
      glTranslatef(0, 0, z);
      glRotatef(-90,1,0,0);
      glColor3f(0.15f,0.15f,0.15f);
      gluDisk(q, 0.0, 0.28f, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

static void draw_stand_part(float p) {
    // p: 0..1
    GLUquadric *q = gluNewQuadric();
    float z = -0.6f * (1.0f - p) + 0.15f * p; // move up
    glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(-90,1,0,0);
      glColor3f(0.2f,0.3f,0.8f);
      gluCylinder(q, 0.05f, 0.05f, 0.55f * p, 20, 2); // length scales a bit
    glPopMatrix();
    gluDeleteQuadric(q);
}

static void draw_motor_part(float p) {
    // p: 0..1
    float start_y = -1.0f; // far away behind
    float y = start_y * (1.0f - p) + 0.0f * p;
    float z = (0.0f) * (1.0f - p) + fan_center_z * p;
    GLUquadric *q = gluNewQuadric();
    glPushMatrix();
      glTranslatef(0.0f, y, z);
      glColor3f(0.45f,0.45f,0.45f);
      gluSphere(q, 0.10f, 28, 28);
    glPopMatrix();
    gluDeleteQuadric(q);
}

static void draw_blades_part(float p, float blades_spin) {
    // p: 0..1. blades slide radially in from offset to actual position
    float slide = (1.0f - p) * 0.6f; // large offset when p=0
    glPushMatrix();
      glTranslatef(0,0,fan_center_z);
      glRotatef(blades_spin, 0,0,1);
      int n = 3;
      for (int i=0;i<n;i++){
          glPushMatrix();
            glRotatef(i*(360.0f/n),0,0,1);
            glTranslatef(0.12f + slide, 0, 0);
            glColor3f(0.92f,0.92f,0.92f);
            draw_blade_geom(0.22f, 0.06f, 0.03f, 10.0f);
          glPopMatrix();
      }
    glPopMatrix();
}

static void draw_guard_part(float p) {
    // p: 0..1 slide forward toward camera (z offset)
    float off = (1.0f - p) * 0.8f;
    glPushMatrix();
      glTranslatef(0, 0, fan_center_z + off);
      glColor3f(0.1f,0.1f,0.1f);
      draw_guard_geom(0.25f);
    glPopMatrix();
}

// ---------- full fan draw using current parts' progresses ----------
static void draw_fan_with_parts(float blades_spin)
{
    // base
    if (prog_base > 0.0f) draw_base_part(prog_base);

    // stand (draw independent from base for animation)
    if (prog_stand > 0.0f) draw_stand_part(prog_stand);

    // motor (draw as moving in)
    if (prog_motor > 0.0f) draw_motor_part(prog_motor);

    // blades
    if (prog_blades > 0.0f) draw_blades_part(prog_blades, blades_spin);

    // guard
    if (prog_guard > 0.0f) draw_guard_part(prog_guard);
}

// ---------- GLFW callbacks and interaction ----------
static void cursor_cb(GLFWwindow *w, double x, double y) {
    if (!dragging) { last_mouse_x = x; last_mouse_y = y; return; }
    double dx = x - last_mouse_x;
    double dy = y - last_mouse_y;
    last_mouse_x = x; last_mouse_y = y;

    float sensX = 0.45f;
    float sensY = 0.35f;
    fan_yaw += dx * sensX;
    fan_pitch += dy * sensY;
    if (fan_pitch > 85.0f) fan_pitch = 85.0f;
    if (fan_pitch < -85.0f) fan_pitch = -85.0f;
}

static void mouse_button_cb(GLFWwindow *w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = 1;
            glfwGetCursorPos(w, &last_mouse_x, &last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            dragging = 0;
        }
    }
}

static void scroll_cb(GLFWwindow *w, double xoff, double yoff) {
    camera_distance -= (float)yoff * 0.15f;
    if (camera_distance < 0.6f) camera_distance = 0.6f;
    if (camera_distance > 6.0f) camera_distance = 6.0f;
}

static void key_cb(GLFWwindow *w, int key, int sc, int act, int mods) {
    if (act != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, 1);
    else if (key == GLFW_KEY_A) autoplay = !autoplay;
    else if (key == GLFW_KEY_R) {
        // reset everything
        prog_base = prog_stand = prog_motor = prog_blades = prog_guard = 0.0f;
        current_step = target_step = STEP_NONE;
        play_time = 0.0;
    } else if (key == GLFW_KEY_S) {
        int W,H; glfwGetFramebufferSize(w,&W,&H);
        static int shot=0; ++shot;
        char p[512]; sprintf(p,"%s\\shot_%03d.png", OUTDIR, shot);
        save_frame(p, W, H);
        printf("Saved %s\n", p);
    } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6) {
        int step = key - GLFW_KEY_0; // 1..6
        target_step = step;
        autoplay = 0; // pause autoplay when user jumps
    }
}

// ---------- helper: progress interpolation towards target step ----------
static void update_progresses_toward_target(float dt)
{
    // If autoplay: advance target_step automatically
    if (autoplay) {
        // if we haven't reached final, set next target step at a rate
        if (target_step < STEP_FINAL) {
            // decide when to increment target: based on global play_time
            play_time += dt * assemble_speed;
            // every ~1.0 second advance a step
            float threshold = 1.0f; // seconds per part
            if (play_time > threshold) {
                play_time = 0.0;
                target_step = (target_step < STEP_FINAL) ? (target_step + 1) : STEP_FINAL;
            }
        }
    }

    // target_step tells which parts should be fully attached; we move each prog to 1.0 if step >= its number
    float speed = 1.8f * assemble_speed; // interpolation speed
    // base (step 1)
    float want_base = (target_step >= STEP_BASE) ? 1.0f : 0.0f;
    if (fabsf(prog_base - want_base) > 0.001f) {
        if (prog_base < want_base) prog_base += speed * dt;
        else prog_base -= speed * dt;
        if (prog_base > 1.0f) prog_base = 1.0f;
        if (prog_base < 0.0f) prog_base = 0.0f;
    }

    // stand (step 2)
    float want_stand = (target_step >= STEP_STAND) ? 1.0f : 0.0f;
    if (fabsf(prog_stand - want_stand) > 0.001f) {
        if (prog_stand < want_stand) prog_stand += speed * dt;
        else prog_stand -= speed * dt;
        if (prog_stand > 1.0f) prog_stand = 1.0f;
        if (prog_stand < 0.0f) prog_stand = 0.0f;
    }

    // motor (step 3)
    float want_motor = (target_step >= STEP_MOTOR) ? 1.0f : 0.0f;
    if (fabsf(prog_motor - want_motor) > 0.001f) {
        if (prog_motor < want_motor) prog_motor += speed * dt;
        else prog_motor -= speed * dt;
        if (prog_motor > 1.0f) prog_motor = 1.0f;
        if (prog_motor < 0.0f) prog_motor = 0.0f;
    }

    // blades (step 4)
    float want_blades = (target_step >= STEP_BLADES) ? 1.0f : 0.0f;
    if (fabsf(prog_blades - want_blades) > 0.001f) {
        if (prog_blades < want_blades) prog_blades += speed * dt;
        else prog_blades -= speed * dt;
        if (prog_blades > 1.0f) prog_blades = 1.0f;
        if (prog_blades < 0.0f) prog_blades = 0.0f;
    }

    // guard (step 5)
    float want_guard = (target_step >= STEP_GUARD) ? 1.0f : 0.0f;
    if (fabsf(prog_guard - want_guard) > 0.001f) {
        if (prog_guard < want_guard) prog_guard += speed * dt;
        else prog_guard -= speed * dt;
        if (prog_guard > 1.0f) prog_guard = 1.0f;
        if (prog_guard < 0.0f) prog_guard = 0.0f;
    }

    // if everything attached, set final step
    if (prog_base > 0.999f && prog_stand > 0.999f && prog_motor > 0.999f && prog_blades > 0.999f && prog_guard > 0.999f) {
        current_step = STEP_FINAL;
    } else {
        // set current_step according to fully completed parts
        int cs = 0;
        if (prog_base > 0.95f) cs = 1;
        if (prog_stand > 0.95f) cs = 2;
        if (prog_motor > 0.95f) cs = 3;
        if (prog_blades > 0.95f) cs = 4;
        if (prog_guard > 0.95f) cs = 5;
        current_step = cs;
    }
}

// ---------- main ----------
int main(void)
{
    if (!glfwInit()) return 1;

    int W = 1280, H = 720;
    GLFWwindow *win = glfwCreateWindow(W, H, "Table Fan — Assembly Demo (A toggle auto, 1..6 steps, S save)", NULL, NULL);
    if (!win) { glfwTerminate(); return 1; }

    glfwMakeContextCurrent(win);
    glfwSetCursorPosCallback(win, cursor_cb);
    glfwSetMouseButtonCallback(win, mouse_button_cb);
    glfwSetScrollCallback(win, scroll_cb);
    glfwSetKeyCallback(win, key_cb);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    GLfloat lightpos[] = { 1.0f, -1.0f, 2.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

    // start with autoplay on and step 0
    autoplay = 1;
    target_step = STEP_NONE;
    play_time = 0.0;

    double last = glfwGetTime();

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        double dt = now - last;
        last = now;

        // update assembly progress toward target
        update_progresses_toward_target((float)dt);

        // if blades are attached sufficiently, allow spin; else keep slow spin
        if (prog_blades > 0.95f) blade_angle += blade_speed * (float)dt;
        else blade_angle += blade_speed * 0.35f * (float)dt; // slow while partial

        if (blade_angle > 360.0f) blade_angle -= 360.0f;

        // render
        int fbW, fbH; glfwGetFramebufferSize(win, &fbW, &fbH);
        glViewport(0,0,fbW,fbH);
        glClearColor(0.85f, 0.92f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, (double)fbW/(double)fbH, 0.1, 20.0);

        // modelview - camera fixed front view, position uses camera_distance
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0.0, -camera_distance, fan_center_z,
                  0.0, 0.0, fan_center_z,
                  0.0, 0.0, 1.0);

        // apply user rotation to whole fan object
        glPushMatrix();
          glTranslatef(0.0f, 0.0f, 0.0f);
          // rotate around fan center:
          glTranslatef(0.0f, 0.0f, fan_center_z);
          glRotatef(fan_pitch, 1.0f, 0.0f, 0.0f);
          glRotatef(fan_yaw, 0.0f, 0.0f, 1.0f);
          glTranslatef(0.0f, 0.0f, -fan_center_z);

          // draw parts with current progress values
          draw_fan_with_parts(blade_angle);
        glPopMatrix();

        // optional: draw small on-screen text? (not using text lib here)

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
