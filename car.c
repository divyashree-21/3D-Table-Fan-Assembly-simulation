// truck_full.c
// Full 3D Pickup Truck — movable parts + DSA (undo) + CSV save/load + PNG frames
// MSYS2 MinGW64 compile:
// gcc truck_full.c -o truck_full.exe -lglfw3 -lglu32 -lopengl32 -lgdi32 -lm
//
// Requires: stb_image_write.h in same folder
// Screenshot frames saved to OUTDIR. Assembly saved/loaded from CSV file in DATADIR.

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

// ---------------- CONFIG ----------------
const char *OUTDIR = "C:\\Users\\Divya gowda\\OneDrive\\Documents\\project.c\\render_out";
const char *DATADIR = "C:\\Users\\Divya gowda\\OneDrive\\Documents\\project.c\\data";
const char *CSVFILE = "C:\\Users\\Divya gowda\\OneDrive\\Documents\\project.c\\data\\truck_saved.csv";
const int WINW = 1280, WINH = 720;
const float truck_center_z = 0.5f; // approximate center height for camera focal point

// ---------------- PART DATA STRUCT ----------------
typedef struct {
    char name[32];
    float x, y, z;   // world translate
    float rot;       // rotation around Z (deg)
    float sx, sy, sz; // scale for drawing
    float pickRadius; // picking radius for click selection
} Part;

enum PART_IDS {
    PART_BODY = 1,
    PART_CARGO,
    PART_WFL, // wheel front left
    PART_WFR,
    PART_WRL,
    PART_WRR,
    PART_LDOOR,
    PART_RDOOR,
    PART_BONNET,
    PART_TAILGATE,
    PART_COUNT_PLUS1
};

static Part parts[PART_COUNT_PLUS1];
static int selected = PART_BODY;

// ---------------- Interaction state ----------------
static double last_x, last_y;
static int left_down = 0, right_down = 0, middle_down = 0;
static int dragging_part = 0, rotating_part = 0, orbiting = 0;
static float orbit_yaw = 0, orbit_pitch = -10.0f;
static float cam_dist = 4.0f;
static float wheel_spin = 0.0f;
static int driving = 0;

// frame saving
static int saving_frames = 0;
static int next_frame_num = 1;

// Undo stack node
typedef struct UndoNode {
    int id;
    float x,y,z,rot;
    struct UndoNode* next;
} UndoNode;
static UndoNode *undoTop = NULL;

// ---------------- Utility ----------------
static void push_undo(int id) {
    UndoNode *n = (UndoNode*)malloc(sizeof(UndoNode));
    n->id = id;
    n->x = parts[id].x; n->y = parts[id].y; n->z = parts[id].z; n->rot = parts[id].rot;
    n->next = undoTop; undoTop = n;
}
static void pop_undo() {
    if (!undoTop) { printf("Nothing to undo\n"); return; }
    UndoNode *n = undoTop; undoTop = undoTop->next;
    parts[n->id].x = n->x; parts[n->id].y = n->y; parts[n->id].z = n->z; parts[n->id].rot = n->rot;
    free(n);
}

// ---------------- Init parts ----------------
static void init_parts() {
    memset(parts,0,sizeof(parts));
    // BODY (a box)
    strcpy(parts[PART_BODY].name, "BODY");
    parts[PART_BODY].x = 0; parts[PART_BODY].y = 0; parts[PART_BODY].z = 0.6f;
    parts[PART_BODY].rot = 0;
    parts[PART_BODY].sx = 2.0f; parts[PART_BODY].sy = 1.0f; parts[PART_BODY].sz = 0.4f;
    parts[PART_BODY].pickRadius = 1.0f;

    // CARGO / bed
    strcpy(parts[PART_CARGO].name, "CARGO");
    parts[PART_CARGO].x = -0.9f; parts[PART_CARGO].y = 0; parts[PART_CARGO].z = 0.55f;
    parts[PART_CARGO].sx = 1.0f; parts[PART_CARGO].sy = 1.0f; parts[PART_CARGO].sz = 0.35f;
    parts[PART_CARGO].pickRadius = 0.7f;

    // Wheels (cylinders) positions relative to center
    strcpy(parts[PART_WFL].name, "WHEEL_FL");
    parts[PART_WFL].x = 0.9f; parts[PART_WFL].y = 0.65f; parts[PART_WFL].z = 0.25f;
    parts[PART_WFL].pickRadius = 0.25f;

    strcpy(parts[PART_WFR].name, "WHEEL_FR");
    parts[PART_WFR].x = 0.9f; parts[PART_WFR].y = -0.65f; parts[PART_WFR].z = 0.25f;
    parts[PART_WFR].pickRadius = 0.25f;

    strcpy(parts[PART_WRL].name, "WHEEL_RL");
    parts[PART_WRL].x = -0.9f; parts[PART_WRL].y = 0.65f; parts[PART_WRL].z = 0.25f;
    parts[PART_WRL].pickRadius = 0.25f;

    strcpy(parts[PART_WRR].name, "WHEEL_RR");
    parts[PART_WRR].x = -0.9f; parts[PART_WRR].y = -0.65f; parts[PART_WRR].z = 0.25f;
    parts[PART_WRR].pickRadius = 0.25f;

    // Doors
    strcpy(parts[PART_LDOOR].name, "L_DOOR");
    parts[PART_LDOOR].x = 0.2f; parts[PART_LDOOR].y = 0.55f; parts[PART_LDOOR].z = 0.6f;
    parts[PART_LDOOR].pickRadius = 0.35f;

    strcpy(parts[PART_RDOOR].name, "R_DOOR");
    parts[PART_RDOOR].x = 0.2f; parts[PART_RDOOR].y = -0.55f; parts[PART_RDOOR].z = 0.6f;
    parts[PART_RDOOR].pickRadius = 0.35f;

    // Bonnet
    strcpy(parts[PART_BONNET].name, "BONNET");
    parts[PART_BONNET].x = 1.1f; parts[PART_BONNET].y = 0.0f; parts[PART_BONNET].z = 0.6f;
    parts[PART_BONNET].pickRadius = 0.6f;

    // Tailgate
    strcpy(parts[PART_TAILGATE].name, "TAILGATE");
    parts[PART_TAILGATE].x = -1.4f; parts[PART_TAILGATE].y = 0.0f; parts[PART_TAILGATE].z = 0.55f;
    parts[PART_TAILGATE].pickRadius = 0.6f;
}

// ---------------- Drawing primitives ----------------
static void draw_box_unit() {
    // draws a unit cube centered at origin (size 1) scaled by caller
    glBegin(GL_QUADS);
    // top (+z)
    glNormal3f(0,0,1);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f( 0.5f, -0.5f, 0.5f);
    glVertex3f( 0.5f,  0.5f, 0.5f);
    glVertex3f(-0.5f,  0.5f, 0.5f);
    // bottom
    glNormal3f(0,0,-1);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    // front (+y)
    glNormal3f(0,1,0);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f( 0.5f, 0.5f, -0.5f);
    glVertex3f( 0.5f, 0.5f,  0.5f);
    glVertex3f(-0.5f, 0.5f,  0.5f);
    // back (-y)
    glNormal3f(0,-1,0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    // right (+x)
    glNormal3f(1,0,0);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f,  0.5f);
    glVertex3f(0.5f,  0.5f,  0.5f);
    glVertex3f(0.5f,  0.5f, -0.5f);
    // left (-x)
    glNormal3f(-1,0,0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

static void draw_cylinder(float radius, float length) {
    GLUquadric *q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    glPushMatrix();
      glRotatef(90,0,1,0); // cylinder along X
      gluCylinder(q, radius, radius, length, 24, 2);
    glPopMatrix();
    gluDeleteQuadric(q);
}

// --------------- Part drawing ----------------
static void draw_part(int id) {
    if (id == PART_BODY) {
        glPushMatrix();
          glScalef(parts[id].sx, parts[id].sy, parts[id].sz);
          glColor3f(0.8f, 0.15f, 0.15f);
          draw_box_unit();
        glPopMatrix();
    } else if (id == PART_CARGO) {
        glPushMatrix();
          glScalef(parts[id].sx, parts[id].sy, parts[id].sz);
          glColor3f(0.6f, 0.35f, 0.1f);
          draw_box_unit();
        glPopMatrix();
    } else if (id >= PART_WFL && id <= PART_WRR) {
        glPushMatrix();
          glColor3f(0.05f,0.05f,0.05f);
          // draw wheel as cylinder + rim
          glRotatef(90,0,1,0);
          gluDisk(gluNewQuadric(), 0.0, 0.18, 20, 1);
          glTranslatef(0,0,0); // no-op
          // cylinder
          draw_cylinder(0.12f, 0.05f);
        glPopMatrix();
    } else if (id == PART_LDOOR || id == PART_RDOOR) {
        glPushMatrix();
          glScalef(0.7f, 0.05f, 0.45f);
          glColor3f(0.75f,0.75f,0.75f);
          draw_box_unit();
        glPopMatrix();
    } else if (id == PART_BONNET || id == PART_TAILGATE) {
        glPushMatrix();
          glScalef(0.7f, 0.9f, 0.12f);
          glColor3f(0.7f,0.2f,0.2f);
          draw_box_unit();
        glPopMatrix();
    }
}

// ---------------- Scene drawing ----------------
static void draw_scene() {
    // draw ground
    glDisable(GL_LIGHTING);
    glColor3f(0.90f,0.95f,1.0f);
    glBegin(GL_QUADS);
      glVertex3f(-10,-10,0);
      glVertex3f( 10,-10,0);
      glVertex3f( 10, 10,0);
      glVertex3f(-10, 10,0);
    glEnd();
    glEnable(GL_LIGHTING);

    // draw each part with its transform
    for (int i = PART_BODY; i <= PART_TAILGATE; ++i) {
        glPushMatrix();
          glTranslatef(parts[i].x, parts[i].y, parts[i].z);
          glRotatef(parts[i].rot, 0,0,1);

          // wheels rotate visually (spin) about their local axis
          if (i >= PART_WFL && i <= PART_WRR) {
              // rotate wheel orientation and spin
              // wheel spin axis: around Y in local box coords, but we drew cylinder along X earlier; adapt
              glRotatef(90,0,1,0);
              glRotatef(wheel_spin, 1,0,0);
              // draw actual wheel geometry
              // we already rotate in draw_part for wheels; keep simple
          }

          draw_part(i);
        glPopMatrix();
    }

    // draw part highlight circle for selected
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.8f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    float cx = parts[selected].x, cy = parts[selected].y, cz = parts[selected].z;
    if (selected >= PART_WFL && selected <= PART_WRR) cz = parts[selected].z; // wheel center
    float r = parts[selected].pickRadius;
    for (int a=0;a<48;a++){
        float ang = (float)a/(48.0f)*(2.0f*(float)M_PI);
        glVertex3f(cx + cosf(ang)*r, cy + sinf(ang)*r, cz + 0.05f);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

// ---------------- Picking (ray-sphere) ----------------
static double ray_sphere_intersect(double Ox,double Oy,double Oz,double Dx,double Dy,double Dz,double Cx,double Cy,double Cz,double r) {
    double Lx = Ox - Cx, Ly = Oy - Cy, Lz = Oz - Cz;
    double a = Dx*Dx + Dy*Dy + Dz*Dz;
    double b = 2*(Dx*Lx + Dy*Ly + Dz*Lz);
    double c = Lx*Lx + Ly*Ly + Lz*Lz - r*r;
    double disc = b*b - 4*a*c;
    if (disc < 0) return -1;
    double sdisc = sqrt(disc);
    double t1 = (-b - sdisc)/(2*a);
    double t2 = (-b + sdisc)/(2*a);
    if (t1 >= 0) return t1;
    if (t2 >= 0) return t2;
    return -1;
}

static int pick_at(double sx, double sy, GLFWwindow *win) {
    int vp[4]; double model[16], proj[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, vp);
    double wx, wy, wz;
    gluUnProject(sx, vp[3]-sy, 0.0, model, proj, vp, &wx, &wy, &wz); // near
    double fx, fy, fz;
    gluUnProject(sx, vp[3]-sy, 1.0, model, proj, vp, &fx, &fy, &fz); // far
    double Dx = fx - wx, Dy = fy - wy, Dz = fz - wz;
    int best = 0; double bestt = 1e9;
    for (int i = PART_BODY; i <= PART_TAILGATE; ++i) {
        double cx = parts[i].x, cy = parts[i].y, cz = parts[i].z;
        double t = ray_sphere_intersect(wx,wy,wz, Dx,Dy,Dz, cx,cy,cz, parts[i].pickRadius);
        if (t > 0 && t < bestt) { bestt = t; best = i; }
    }
    return best;
}

// ---------------- Save / Load (CSV) ----------------
static void save_csv(const char *filename) {
    FILE *f = fopen(filename,"w");
    if(!f){ printf("Failed to open CSV for save: %s\n", filename); return; }
    fprintf(f,"id,name,x,y,z,rot\n");
    for(int i=PART_BODY;i<=PART_TAILGATE;i++){
        fprintf(f,"%d,%s,%.6f,%.6f,%.6f,%.6f\n", i, parts[i].name, parts[i].x, parts[i].y, parts[i].z, parts[i].rot);
    }
    fclose(f);
    printf("Saved assembly to %s\n", filename);
}

static void load_csv(const char *filename) {
    FILE *f = fopen(filename,"r");
    if(!f){ printf("No saved CSV found: %s\n", filename); return; }
    char line[512];
    fgets(line,sizeof(line),f); // header
    while(fgets(line,sizeof(line),f)){
        int id; char name[64]; float x,y,z,rot;
        if (sscanf(line,"%d,%63[^,],%f,%f,%f,%f",&id,name,&x,&y,&z,&rot) == 6) {
            if (id>=PART_BODY && id<=PART_TAILGATE){
                parts[id].x = x; parts[id].y = y; parts[id].z = z; parts[id].rot = rot;
            }
        }
    }
    fclose(f);
    printf("Loaded assembly from %s\n", filename);
}

// ---------------- Screenshot ----------------
static void save_frame_as_png(const char *path, int W, int H) {
    unsigned char *pixels = malloc(W*H*3);
    unsigned char *flipped = malloc(W*H*3);
    if(!pixels || !flipped){ free(pixels); free(flipped); return; }
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    for (int y=0;y<H;y++) memcpy(flipped + (H-1-y)*W*3, pixels + y*W*3, W*3);
    stbi_write_png(path, W, H, 3, flipped, W*3);
    free(pixels); free(flipped);
}

// ---------------- Callbacks / Interaction ----------------
static void cursor_cb(GLFWwindow *win,double x,double y) {
    if (dragging_part) {
        // compute ray, project to plane z = targetZ and set part.x/y
        int vp[4]; double model[16], proj[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, vp);
        double nx,ny,nz, fx,fy,fz;
        gluUnProject(x, vp[3]-y, 0.0, model, proj, vp, &nx, &ny, &nz);
        gluUnProject(x, vp[3]-y, 1.0, model, proj, vp, &fx, &fy, &fz);
        double Dx = fx - nx, Dy = fy - ny, Dz = fz - nz;
        double targetZ = parts[selected].z;
        // for wheels we use wheel center z
        if (selected >= PART_WFL && selected <= PART_WRR) targetZ = parts[selected].z;
        if (fabs(Dz) > 1e-6) {
            double t = (targetZ - nz) / Dz;
            double wx = nx + t*Dx;
            double wy = ny + t*Dy;
            // if blade/wheel, set offsets accordingly (here absolute)
            parts[selected].x = (float)wx;
            parts[selected].y = (float)wy;
        }
    } else if (rotating_part) {
        double dx = x - last_x;
        parts[selected].rot += (float)dx * 0.3f;
    } else if (orbiting) {
        orbit_yaw += (float)(x - last_x)*0.2f;
        orbit_pitch += (float)(y - last_y)*0.2f;
        if (orbit_pitch > 89) orbit_pitch = 89;
        if (orbit_pitch < -89) orbit_pitch = -89;
    }
    last_x = x; last_y = y;
}

static void mouse_button_cb(GLFWwindow *win, int button, int action, int mods) {
    double mx,my; glfwGetCursorPos(win,&mx,&my);
    if (button==GLFW_MOUSE_BUTTON_LEFT) {
        if (action==GLFW_PRESS) {
            left_down=1;
            int picked = pick_at(mx,my,win);
            if (picked) {
                selected = picked; printf("Picked: %s\n", parts[selected].name);
                push_undo(selected); // store pre-move state
                dragging_part = 1;
            } else {
                // if clicked empty area -> start orbit if middle not pressed
                dragging_part = 0;
            }
        } else { left_down=0; dragging_part = 0; }
    } else if (button==GLFW_MOUSE_BUTTON_RIGHT) {
        if (action==GLFW_PRESS) { right_down=1; rotating_part = 1; push_undo(selected); }
        else { right_down=0; rotating_part = 0; }
    } else if (button==GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action==GLFW_PRESS) { middle_down=1; orbiting = 1; }
        else { middle_down=0; orbiting = 0; }
    }
}

static void scroll_cb(GLFWwindow *win, double xoff, double yoff) {
    if (selected>=PART_BODY && selected<=PART_TAILGATE) {
        parts[selected].z += (float)yoff * 0.05f;
    } else {
        cam_dist -= (float)yoff * 0.2f;
        if (cam_dist < 1.0f) cam_dist = 1.0f;
    }
}

static void key_cb(GLFWwindow *win, int key, int sc, int action, int mods) {
    if (action!=GLFW_PRESS && action!=GLFW_REPEAT) return;
    float mv = 0.05f;
    if (key>=GLFW_KEY_1 && key<=GLFW_KEY_0) {
        // allow 1..9,0 maps to tailgate
        int idx = (key==GLFW_KEY_0)? PART_TAILGATE : (key - GLFW_KEY_0);
        if (idx>=PART_BODY && idx<=PART_TAILGATE) { selected = idx; printf("Selected: %s\n", parts[selected].name); }
        return;
    }
    switch(key) {
        case GLFW_KEY_W: parts[selected].y += mv; break;
        case GLFW_KEY_S: parts[selected].y -= mv; break;
        case GLFW_KEY_A: parts[selected].x -= mv; break;
        case GLFW_KEY_D: parts[selected].x += mv; break;
        case GLFW_KEY_Q: parts[selected].z -= mv; break;
        case GLFW_KEY_E: parts[selected].z += mv; break;
        case GLFW_KEY_LEFT: parts[selected].rot += 6.0f; break;
        case GLFW_KEY_RIGHT: parts[selected].rot -= 6.0f; break;
        case GLFW_KEY_U: pop_undo(); break;
        case GLFW_KEY_K: save_csv(CSVFILE); break;
        case GLFW_KEY_L: load_csv(CSVFILE); break;
        case GLFW_KEY_P:
            saving_frames = !saving_frames;
            if (saving_frames) { next_frame_num = 1; printf("Started saving frames\n"); }
            else printf("Stopped saving frames\n");
            break;
        case GLFW_KEY_SPACE:
            driving = !driving;
            printf("Driving toggled: %d\n", driving);
            break;
        case GLFW_KEY_R:
            init_parts(); printf("Reset parts\n"); break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(win,1); break;
    }
}

// ---------------- Main ----------------
int main(void) {
    if (!glfwInit()) { fprintf(stderr,"GLFW init failed\n"); return 1; }
    GLFWwindow *win = glfwCreateWindow(WINW, WINH, "Pickup Truck - Move parts separately", NULL, NULL);
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
    GLfloat lp[4] = {1.0f, -1.0f, 2.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lp);

    init_parts();

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        double dt = now - last;
        last = now;

        // driving moves wheels spin and optional translation
        if (driving) {
            wheel_spin += 180.0f * (float)dt * 2.0f; // fast spin
            // optionally move whole truck forward slightly
            for (int i = PART_BODY; i <= PART_TAILGATE; ++i) parts[i].x -= 0.3f * (float)dt;
        } else {
            wheel_spin += 60.0f * (float)dt;
        }
        if (wheel_spin > 360.0f) wheel_spin -= 360.0f;

        int fbW, fbH; glfwGetFramebufferSize(win,&fbW,&fbH);
        glViewport(0,0,fbW,fbH);
        glClearColor(0.8f,0.9f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, (double)fbW/(double)fbH, 0.1, 100.0);
        // camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        float yaw = orbit_yaw * (M_PI/180.0f);
        float pitch = orbit_pitch * (M_PI/180.0f);
        float ex = cam_dist * cosf(pitch) * sinf(yaw);
        float ey = -cam_dist * cosf(pitch) * cosf(yaw);
        float ez = cam_dist * sinf(pitch) + truck_center_z;
        gluLookAt(ex, ey, ez,  0.0,0.0,truck_center_z,  0.0,0.0,1.0);

        draw_scene();

        // saving frames?
        if (saving_frames) {
            char path[512];
            snprintf(path, sizeof(path), "%s\\frame_%03d.png", OUTDIR, next_frame_num++);
            save_frame_as_png(path, fbW, fbH);
            printf("Saved %s\n", path);
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
