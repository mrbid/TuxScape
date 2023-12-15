/*
    James William Fletcher  ( notabug.org/Vandarin )
        November 2023       ( github.com/mrbid     )

    A mythical adventure as Tux!

    Inspired by the famous game Run-Escape.

    I've used a few 'hax' to prevent storing stuff in
    memory like using % with integers and a fixed seed
    with the fast randf() function. This is probably
    slower than using memory access, I just did this
    because it looks cleaner than littering the code
    with un-necessary variables. It's just how I wanted
    to do things I guess, and I just want you the reader
    to know somewhat of why I made that decision.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WEB
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
#endif

#define uint GLuint
#define sint GLint

#include "inc/gl.h"
#define GLFW_INCLUDE_NONE
#include "inc/glfw3.h"
#define fTime() (float)glfwGetTime()

#define MAX_MODELS 128 // hard limit, be aware and increase if needed
#include "inc/esAux5.h"

#include "inc/res.h"
#include "assets/high/knife.h"  //0
#include "assets/high/tux.h"    //1
#include "assets/high/treeman.h"//2
#include "assets/high/flame1.h" //3
#include "assets/high/flame2.h" //4
#include "assets/high/f1.h"     //5
#include "assets/high/f2.h"     //6
#include "assets/high/f3.h"     //7
#include "assets/high/tree1.h"  //8
#include "assets/high/hot.h"    //9
#include "assets/high/box1.h"   //10
#include "assets/high/box2.h"   //11
#include "assets/high/box3.h"   //12
#include "assets/high/box4.h"   //13
#include "assets/high/box5.h"   //14
#include "assets/high/box6.h"   //15
#include "assets/high/hp1.h"    //16
#include "assets/high/hp2.h"    //17
#include "assets/high/hp3.h"    //18
#include "assets/high/hp4.h"    //19
#include "assets/high/hp5.h"    //20
#include "assets/high/hp6.h"    //21
#include "assets/high/hp7.h"    //22
#include "assets/high/hp8.h"    //23
#include "assets/high/hp9.h"    //24
#include "assets/high/hp10.h"   //25
#include "assets/high/hp11.h"   //26
#include "assets/high/hp12.h"   //27
#include "assets/high/hp13.h"   //28
#include "assets/high/hp14.h"   //29
#include "assets/high/shield1.h"//30
#include "assets/high/shield2.h"//31
#include "assets/high/shield3.h"//32
#include "assets/high/shield4.h"//33
#include "assets/high/wep1.h"   //34
#include "assets/high/wep2.h"   //35
#include "assets/high/wep3.h"   //36
#include "assets/high/wep4.h"   //37
#include "assets/high/wep5.h"   //38
#include "assets/high/wep6.h"   //39
#include "assets/high/wep7.h"   //40
#include "assets/high/wep8.h"   //41
#include "assets/high/zombie.h" //42
#include "assets/high/dinofly.h"//43
#include "assets/high/babybluedragon.h"//44
#include "assets/high/skull.h"     //45
#include "assets/high/tornado.h"   //46
#include "assets/high/dragphent.h" //47
#include "assets/high/alien.h"     //48
#include "assets/high/aliendog.h"  //49
#include "assets/high/redknight.h" //50
#include "assets/high/samurai.h"   //51
#include "assets/high/wizard.h"    //52
#include "assets/high/knight.h"    //53
#include "assets/high/ogre.h"      //54
#include "assets/high/golum.h"     //55
#include "assets/high/bear.h"      //56
#include "assets/high/warewolf.h"  //57
#include "assets/high/bluedragon.h"//58
#include "assets/high/reddragon.h" //59
#include "assets/high/intro.h"     //60
#include "assets/high/crossbones.h"//61
#include "assets/high/meshy.h"     //62

//*************************************
// globals
//*************************************
const char appTitle[]="TuxScape";
GLFWwindow* window;
uint winw=1024, winh=768;
float t=0.f, dt=0.f, st=0.f, fc=0.f, lfct=0.f, aspect;
double mx,my,lx,ly,ww,wh;

// render state
mat projection, view, model, modelview;

// game vars
#define FAR_DISTANCE 384.f
#define NEWGAME_SEED 1337
uint keystate[6] = {0};

// camera vars
uint focus_cursor = 0;
double sens = 0.003f;
float xrot = 0.f;
float yrot = d2PI;
float zoom = -0.3f;
vec look_dir, lookx, looky;

// player vars
float MOVE_SPEED = 0.6f;
vec pp, mpp;      // position and inverted(-)position
uint attack = 0;  // atacking
uint swipe = 0;   // swipe animation
uint swipe_phase = 0;
float swipe_lt = 0.f;
uint focus = 0;   // focus mode
uint grounded = 0;// is on ground (not flying)
vec plook;        // 2D look direction (zero Z axis) (not normalised)
int health = 100; // health
int mhealth = 100;// max health
uint xp = 0;      // total xp gained
uint weapon = 0;  // weapon unlock level
uint wield = 0;   // currently wielded weapon
uint sticky = 0;  // sticky/toggle mouse clicking (afking)

// islands
#define MAX_ISLANDS 21
vec islands[MAX_ISLANDS];
float islands_rad[MAX_ISLANDS];
uint islands_type[MAX_ISLANDS];

// enemies
#define MAX_ENEMIES 512
vec enemy_pos[MAX_ENEMIES];
vec enemy_spn[MAX_ENEMIES];
uint enemy_type[MAX_ENEMIES];
uint enemy_island[MAX_ENEMIES];
int enemy_health[MAX_ENEMIES];
int enemy_health_max[MAX_ENEMIES];
float enemy_att[MAX_ENEMIES]; // for attack timings
float enemy_hit[MAX_ENEMIES];
void spawnEnemy(const uint island, const uint type)
{
    for(uint i=0; i < MAX_ENEMIES; i++)
    {
        if(enemy_pos[i].x == 0.f && enemy_pos[i].y == 0.f && enemy_pos[i].z == 0.f)
        {
            vec p = islands[island];
            const float a = esRandFloat(-PI, PI);
            const float r = esRandFloat(0.f, islands_rad[island]*0.42f);
            p.x += r*cosf(a), p.y += r*sinf(a);
            enemy_pos[i] = p;
            enemy_spn[i] = p;
            enemy_type[i] = type;
            enemy_island[i] = island;
            enemy_health[i] = 100 * ((1+type)*(1+type)); // scale heath by enemy difficulty (exponential)
            enemy_health_max[i] = enemy_health[i];
            enemy_att[i] = 0.f;
            enemy_hit[i] = 0.f;
            break;
        }
    }
}

// boxes
#define MAX_BOXES 48
#define MAX_ITEMS 384
vec box_pos[MAX_BOXES] = {0};
vec itm_pos[MAX_ITEMS] = {0};
void addBox()
{
    for(uint i=0; i < MAX_BOXES; i++)
    {
        if(box_pos[i].x == 0.f && box_pos[i].y == 0.f && box_pos[i].z == 0.f)
        {
            const uint j = esRand(1, MAX_ISLANDS);
            vec p = islands[j];
            const float a = esRandFloat(-PI, PI);
            const float r = esRandFloat(0.f, islands_rad[j]*0.4f);
            p.x += r*cosf(a), p.y += r*sinf(a);
            box_pos[i] = p;
            break;
        }
    }
}
void popBox(const vec pos)
{
    const uint max_drop = esRand(1, 6);
    uint mdc = 0;
    for(uint i=0; i < MAX_ITEMS; i++)
    {
        if(itm_pos[i].x == 0.f && itm_pos[i].y == 0.f && itm_pos[i].z == 0.f)
        {
            vec p = pos;
            const float a = esRandFloat(-PI, PI);
            const float r = esRandFloat(0.f, 0.2f);
            p.x += r*cosf(a), p.y += r*sinf(a);
            itm_pos[i] = p;
            mdc++;
            if(mdc >= max_drop){break;}
        }
    }
}

// misc
vec balloon_pos = (vec){0.f, 0.f, 10.f};

//*************************************
// utility functions
//*************************************
void timestamp(char* ts)
{
    const time_t tt = time(0);
    strftime(ts, 16, "%H:%M:%S", localtime(&tt));
}
float vDistSq2(const vec a, const vec b) // this is so pointless when I ignore all the other micro-optimisations
{                                        // its almost as pathetic as for(NULL;NULL;++i)
    const float xm = (a.x - b.x);
    const float ym = (a.y - b.y);
    return xm*xm + ym*ym;
}
void updateModelView()
{
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
}
void updateTitle()
{
    char tmp[256];
    sprintf(tmp, "❤️ %u - 📈 %u - ⚔️ %u/8", health, xp, weapon);
    glfwSetWindowTitle(window, tmp);
}
void doAttack()
{
    for(uint i=0; i < MAX_ENEMIES; i++)
    {
        if(enemy_pos[i].x != 0.f || enemy_pos[i].y != 0.f || enemy_pos[i].z != 0.f)
        {
            if(vDistSq(mpp, enemy_pos[i]) < 0.014f)
            {
                vec n = (vec){enemy_pos[i].x, enemy_pos[i].y, 0.f};
                vSub(&n, n, (vec){-pp.x, -pp.y, 0.f});
                vNorm(&n);
                vec tl = plook;
                vNorm(&tl);
                const float angle = (tl.x * n.x) + (tl.y * n.y);
                //printf("%f\n", angle);
                if(angle > 0.9f) // looking at the enemy?
                {
                    enemy_health[i] -= 88*(1+wield); // scale damage by wield weapon level
                    //printf("%u: %i\n", i, enemy_health[i]);
                    if(enemy_health[i] <= 0) // uh-oh dead
                    {
                        xp += (1+enemy_type[i])*(1+enemy_type[i]);
                        if     (xp > 113422){weapon = 8, mhealth = 500; wield = weapon;}
                        else if(xp > 49314 ){weapon = 7, mhealth = 450; wield = weapon;}
                        else if(xp > 21441 ){weapon = 6, mhealth = 400; wield = weapon;}
                        else if(xp > 9322  ){weapon = 5, mhealth = 350; wield = weapon;}
                        else if(xp > 4053  ){weapon = 4, mhealth = 300; wield = weapon;}
                        else if(xp > 1762  ){weapon = 3, mhealth = 250; wield = weapon;}
                        else if(xp > 766   ){weapon = 2, mhealth = 200; wield = weapon;}
                        else if(xp > 333   ){weapon = 1, mhealth = 150; wield = weapon;}

                        updateTitle();
                        enemy_pos[i] = (vec){0.f, 0.f, 0.f};
                    }
                    break; // one enemy per swipe
                }
            }
        }
    }
}

//*************************************
// render functions
//*************************************
void modelBind(const ESModel* mdl)
{
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vid);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(position_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->iid);
}

//*************************************
// update & render
//*************************************
void main_loop()
{
//*************************************
// core logic
//*************************************
    fc++;
    glfwPollEvents();
    static float lt = 0;
    t = fTime();
    dt = t-lt;
    lt = t;

#ifdef WEB
    EmscriptenPointerlockChangeEvent e;
    if(emscripten_get_pointerlock_status(&e) == EMSCRIPTEN_RESULT_SUCCESS)
    {
        if(focus_cursor == 0 && e.isActive == 1)
        {
            glfwGetCursorPos(window, &lx, &ly);
        }
        focus_cursor = e.isActive;
    }
#endif

//*************************************
// game logic
//*************************************

if(health > 0)
{
    // forward & backward
    if(keystate[2] == 1)
    {
        vec m;
        vMulS(&m, (vec){look_dir.x, look_dir.y, 0.f}, MOVE_SPEED * dt);
        vSub(&pp, pp, m);
    }
    else if(keystate[3] == 1)
    {
        vec m;
        vMulS(&m, (vec){look_dir.x, look_dir.y, 0.f}, MOVE_SPEED * dt);
        vAdd(&pp, pp, m);
    }

    // strafe left & right
    if(keystate[0] == 1)
    {
        mGetViewX(&lookx, view);
        vec m;
        vMulS(&m, (vec){lookx.x, lookx.y, 0.f}, MOVE_SPEED * dt);
        vSub(&pp, pp, m);
    }
    else if(keystate[1] == 1)
    {
        mGetViewX(&lookx, view);
        vec m;
        vMulS(&m, (vec){lookx.x, lookx.y, 0.f}, MOVE_SPEED * dt);
        vAdd(&pp, pp, m);
    }

    // sprint
    if(keystate[4] == 1 || ((keystate[2] == 1 || keystate[5] == 1 || focus == 1) && grounded == 0))
    {
        MOVE_SPEED = 3.20f;
    }
    else
    {
        MOVE_SPEED = 0.60f;
    }
    
    // jet pack
    if(keystate[5] == 1)
    {
        pp.z += MOVE_SPEED * dt;
        grounded = 0;
    }
    else
    {
        // gravity
        {
            uint isfloor = 0;
            const float ftol = 1.35f;

            for(uint i=0; i < MAX_ISLANDS; i++) // could combine this with the render loop but im not that bothered [1]
            {
                float rad = 0.6f;
                if(islands_type[i] == 5){rad = 0.55f;}
                else if(islands_type[i] == 7){rad = 0.58f;}
                const float ar = islands_rad[i]*rad;
                if(vDistSq(mpp, islands[i]) < ar*ar)
                {
                    if(pp.z > islands[i].z-ftol && pp.z <= islands[i].z)
                    {
                        pp.z = islands[i].z;
                        isfloor = 1;
                        grounded = 1;
                        break;
                    }
                }
            }
            
            if(isfloor == 0)
            {
                grounded = 0;
                if(keystate[2] == 1 || keystate[3] == 1 || focus == 1)
                    pp.z -= 0.42f * dt;
                else
                    pp.z -= 2.4f * dt;
            }
        }
    }

    // auto attack
    if(attack == 1)
    {
        if(t > swipe_lt)
        {
            if(swipe_phase == 0)
            {
                doAttack();
                swipe = 1;
            }
            else if(swipe_phase == 1){swipe = 0;}
            swipe_phase = 1 - swipe_phase;
            swipe_lt = t + 0.0625f;
        }
    }

    // spawn new box
    {
        static float lt = 0.f;
        if(t > lt)
        {
            addBox();
            lt = t + esRandFloat(3.f, 8.f);
        }
    }

    // spawn enemies
    {
        static float lt = 0.f;
        if(t > lt)
        {
            const uint ei = esRand(0, 18);
            spawnEnemy(3+ei, ei);
            lt = t + esRandFloat(0.1f, 1.f);
        }
    }
}
else // brown bread view
{
    zoom = -0.3f;
    yrot = 1.f;
}

//*************************************
// camera
//*************************************
    if(focus_cursor == 1)
    {
if(health > 0)
{
        glfwGetCursorPos(window, &mx, &my);
        xrot += (lx-mx)*sens;
        yrot += (ly-my)*sens;
}
        
        if(zoom >= -0.11f)
        {
            if(yrot > 2.5f){yrot = 2.5f;}
        }
        else
        {
            if(yrot > 1.7f){yrot = 1.7f;}
        }
        
        if(yrot < 0.55f)
            yrot = 0.55f;

        lx = mx, ly = my;
    }

    if(zoom >= -0.11f)
    {
        mIdent(&view);
        mRotate(&view, yrot, 1.f, 0.f, 0.f);
        mRotate(&view, xrot, 0.f, 0.f, 1.f);
        mTranslate(&view, pp.x, pp.y, -pp.z-0.09f);
    }
    else
    {
        mIdent(&view);
        mTranslate(&view, 0.f, 0.f, zoom);
        mRotate(&view, yrot, 1.f, 0.f, 0.f);
        mRotate(&view, xrot, 0.f, 0.f, 1.f);
        mTranslate(&view, pp.x, pp.y, -pp.z-0.05f);
    }

    // get look dir/axes
    mGetViewZ(&look_dir, view);
    mGetViewX(&lookx, view);
    mGetViewY(&looky, view);
    mpp = (vec){-pp.x, -pp.y, pp.z};

//*************************************
// render
//*************************************

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render scene
    srandf(1337);
    for(uint i=0; i < MAX_ISLANDS; i++) // [1] here
    {
        esBindModel(islands_type[i]);
        mIdent(&model);
        mRotZ(&model, randf()*PI);
        mScale(&model, islands_rad[i], islands_rad[i], islands_rad[i]);
        mSetPos(&model, islands[i]);
        updateModelView();
        esRenderModel(islands_type[i]);
    }

    // render tree
    esBindModel(8);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&view.m[0][0]);
    esRenderModel(8);

    // render treeman (i just think he looks cool, and dibesfer created the text prompt that created him, and dibesfer is awesome)
    esBindModel(2);
    mIdent(&model);
    mSetPos(&model, (vec){0.f, -0.3f, 0.f});
    if(swipe == 1 && vDistSq(mpp, (vec){0.f, -0.3f, 0.f}) < 0.012f)
    {
        vec n = (vec){0.f, -0.3f, 0.f};
        vSub(&n, n, (vec){-pp.x, -pp.y, 0.f});
        vNorm(&n);
        vec tl = plook;
        vNorm(&tl);
        const float angle = (tl.x * n.x) + (tl.y * n.y);
        if(angle > 0.9f)
        {
            mRotY(&model,  n.y * 0.42f);
            mRotX(&model, -n.x * 0.42f);
        }
    }
    updateModelView();
    esRenderModel(2);

    // render enemies
    for(uint i=0; i < MAX_ENEMIES; i++)
    {
        if(enemy_pos[i].x != 0.f || enemy_pos[i].y != 0.f || enemy_pos[i].z != 0.f)
        {
            const float ftype = 1.f+(float)enemy_type[i];
            vec dir = (vec){0.f, 0.f, 0.f};
            float island_rad_sq = islands_rad[enemy_island[i]]*0.5f;
            island_rad_sq *= island_rad_sq;
            const float edp = vDistSq(mpp, enemy_pos[i]);
            if(edp < 0.1f * ftype) // increase sight range for harder enemies [2]
            {
                if(edp < 0.01f) // stop at enemy feet
                {
                    vec n = mpp;
                    vSub(&n, n, enemy_pos[i]);
                    n.z = 0.f;
                    vNorm(&n);
                    dir = n;

                    // and do damage!
                    if(t > enemy_att[i])
                    {
                        int amt = ((float)(1+enemy_type[i])) * (1.f-(((float)(1+wield))*0.111111111f)); // lol cool fractional part
                        if(amt <= 0){amt=1;}
                        health -= amt;
                        if(health < 0){health = 0;}
                        enemy_hit[i] = t+0.1f;
                        updateTitle();
                        enemy_att[i] = t + ( 3.3f - ( ((float)(1+enemy_type[i])) * 0.157894737f ) );
                    }

                }
                else if(vDistSq2(islands[enemy_island[i]], enemy_pos[i]) < island_rad_sq) // don't leave island bounds
                {
                    vec n = mpp;
                    vSub(&n, n, enemy_pos[i]);
                    n.z = 0.f;
                    vNorm(&n);
                    dir = n;
                    vMulS(&n, n, (0.1f + (ftype*0.042f)) * dt); // and increase speed [2]
                    vAdd(&enemy_pos[i], enemy_pos[i], n);
                }
            }
            else if(vDistSq2(enemy_spn[i], enemy_pos[i]) > 0.03f) // return back to default pos
            {
                vec n = enemy_spn[i];
                vSub(&n, n, enemy_pos[i]);
                n.z = 0.f;
                vNorm(&n);
                dir = n;
                vMulS(&n, n, (0.1f + (ftype*0.042f)) * dt);
                vAdd(&enemy_pos[i], enemy_pos[i], n);
            }
            const uint m = 42+enemy_type[i];
            esBindModel(m);
            mIdent(&model);
            vec epos = enemy_pos[i];
            if(t < enemy_hit[i]){epos.z += 0.01f;} // this shows when the enemy is throwing hits
            mSetPos(&model, epos);
            if(m == 46)
            {
                mRotZ(&model, t * 12.f);
            }
            else
            {
                if(dir.x != 0.f || dir.y != 0.f || dir.z != 0.f)
                {
                    mSetDir(&model, dir);
                }
                else
                {
                    mRotZ(&model, (((float)(i*m))*0.003649635f)*x2PI);
                }
            }
            float zscale = (1.f/((float)enemy_health_max[i]))*((float)enemy_health[i]); // could pre-compute the reciprocal but im past caring it's 2023
            // if(zscale < 0.5f)
            // {
            //     glEnable(GL_BLEND);
            //     glUniform1f(opacity_id, zscale);
            //     mScale(&model, 1.f, 1.f, 0.5f);
            // }
            // else
            // {
                if(zscale < 1.f){mScale(&model, 1.f, 1.f, zscale);}
            // }
            updateModelView();
            esRenderModel(m);
            // if(zscale < 0.5f)
            // {
            //     glDisable(GL_BLEND);
            //     //glUniform1f(opacity_id, 1.f);
            // }
        }
    }

    // render boxes
    for(uint i=0; i < MAX_BOXES; i++)
    {
        if(box_pos[i].x != 0.f || box_pos[i].y != 0.f || box_pos[i].z != 0.f)
        {
            if(swipe == 1)
            {
                if(vDistSq(mpp, box_pos[i]) < 0.014f)
                {
                    popBox(box_pos[i]);
                    box_pos[i] = (vec){0.f, 0.f, 0.f};
                    continue;
                }
            }
            const uint m = 10+(i%6);
            esBindModel(m);
            mIdent(&model);
            mSetPos(&model, box_pos[i]);
            mRotZ(&model, (((float)(i*m))*0.021276596f)*x2PI);
            updateModelView();
            esRenderModel(m);
        }
    }

    // render items
    for(uint i=0; i < MAX_ITEMS; i++)
    {
        if(itm_pos[i].x != 0.f || itm_pos[i].y != 0.f || itm_pos[i].z != 0.f)
        {
            const uint m = 16+(i%14); // lol yes im that frugal on memory today
            if(health < mhealth && vDistSq(mpp, itm_pos[i]) < 0.003f)
            {
                if(m == 16 || m == 17){health += 1;}
                else if(m == 18){health += 2;}
                else if(m == 19){health += 3;}
                else if(m == 20){health += 6;}
                else if(m == 21 || m == 23){health += 8;}
                else if(m == 22 || m == 24){health += 9;}
                else if(m == 25){health += 15;}
                else if(m == 26){health += 16;}
                else if(m == 27){health += 19;}
                else if(m == 28){health += 33;}
                itm_pos[i] = (vec){0.f, 0.f, 0.f};
                updateTitle();
                continue;
            }
            esBindModel(m);
            mIdent(&model);
            mSetPos(&model, itm_pos[i]);
            mRotZ(&model, t * 3.2f);
            updateModelView();
            esRenderModel(m);
        }
    }

    // render hot air balloon
    float bd = vDistSq(mpp, balloon_pos);
    if(bd < 2.56f)
    {
        bd = vDist(mpp, balloon_pos);
        vec v = mpp;
        vSub(&v, v, balloon_pos);
        vNorm(&v);
        vInv(&v);
        vMulS(&v, v, 1.6f - bd);
        vAdd(&balloon_pos, balloon_pos, v);
    }
    esBindModel(9);
    mIdent(&model);
    mSetPos(&model, balloon_pos);
    updateModelView();
    esRenderModel(9);

    // render player
    {
        static uint lrs = 0;
        static float lr = 0.f;
        if(zoom < -0.11f)
        {
            // tux
            mIdent(&model);
            if(keystate[0] == 1 || keystate[1] == 1 || keystate[2] == 1 || keystate[3] == 1 || focus == 1)
            {
                plook = (vec){look_dir.x, look_dir.y, 0.f};
                if(lrs == 1){lrs = 0;}
                mRotZ(&model, -xrot+PI);
            }
            else
            {
                if(lrs == 0)
                {
                    lr = -xrot+PI;
                    lrs = 1;
                }
                mRotZ(&model, lr);
            }
            mSetPos(&model, mpp);
if(health == 0){mScale(&model, 1.f, 1.f, 0.01f);}
            updateModelView();
            esBindModel(1);
            esRenderModel(1);

if(health > 0)
{
            // weapon
            uint wid = 0;
            uint sid = 0;
            if(wield == 1)
            {
                wid = 34;
            }
            else if(wield == 2)
            {
                wid = 35;
            }
            else if(wield == 3)
            {
                wid = 36;
                sid = 30;
            }
            else if(wield == 4)
            {
                wid = 37;
                sid = 31;
            }
            else if(wield == 5)
            {
                wid = 38;
                sid = 31;
            }
            else if(wield == 6)
            {
                wid = 39;
                sid = 32;
            }
            else if(wield == 7)
            {
                wid = 40;
                sid = 32;
            }
            else if(wield == 8)
            {
                wid = 41;
                sid = 33;
            }
            esBindModel(wid);
            mIdent(&model);
            mSetPos(&model, mpp);
            if(keystate[0] == 1 || keystate[1] == 1 || keystate[2] == 1 || keystate[3] == 1 || focus == 1)
                mRotZ(&model, -xrot+PI);
            else
                mRotZ(&model, lr);
            if(swipe == 1){mRotY(&model, -0.420f);}
            mTranslate(&model, 0.025f, 0.f, 0.05f);
            updateModelView();
            esRenderModel(wid);

            // now shield
            if(sid != 0)
            {
                esBindModel(sid);
                mIdent(&model);
                mSetPos(&model, mpp);
                if(keystate[0] == 1 || keystate[1] == 1 || keystate[2] == 1 || keystate[3] == 1 || focus == 1)
                    mRotZ(&model, -xrot+PI);
                else
                    mRotZ(&model, lr);
                mTranslate(&model, -0.020f, -0.016f, 0.05f);
                updateModelView();
                esRenderModel(sid);
            }

            // jet flames
            if((keystate[2] == 1 || keystate[5] == 1 || focus == 1) && grounded == 0)
            {
                glEnable(GL_BLEND);

                glUniform1f(opacity_id, 0.5f);

                esBindModel(4);
                mIdent(&model);
                mRotZ(&model, -t*5.f);
                mSetPos(&model, mpp);
                updateModelView();
                esRenderModel(4);

                glUniform1f(opacity_id, 0.75f);

                esBindModel(3);
                mIdent(&model);
                mRotZ(&model, t*2.f);
                mSetPos(&model, mpp);
                updateModelView();
                esRenderModel(3);

                glDisable(GL_BLEND);
            }
}
        }
        else
        {
            // this is for when your in FPS mode, just to make sure everything remains functional/updated
            plook = (vec){look_dir.x, look_dir.y, 0.f};
            lr = -xrot+PI;
        }
    }

    ///

    // render fps weapon
    if(zoom >= -0.11f)
    {
        mIdent(&model);
        vec ld = look_dir;
        ld.z = 0.f;
        vMulS(&ld, ld, 0.08f); // far from camera
        vec np = (vec){-pp.x, -pp.y, pp.z};
        vAdd(&np, np, ld);

        // x offset
        vec vx = lookx;
        vMulS(&vx, vx, -0.07f);
        vAdd(&np, np, vx);

        // y offset
        vec vy = looky;
        vMulS(&vy, vy, -0.0188f);
        vAdd(&np, np, vy);

        // rotate to camera direction with slight offset and set pos
        mRotZ(&model, -(xrot-PI));
        if(swipe == 1){mRotY(&model, -0.6f); mRotX(&model, -0.3f);}
        mSetPos(&model, np);

        // render it
        uint wid = 33+wield;
        if(wid == 33){wid = 0;}
        updateModelView();
        esBindModel(wid);
        glClear(GL_DEPTH_BUFFER_BIT);
        esRenderModel(wid);
    }

    ///

    // render meshy intro
    if(t < 5.f)
    {
        shadeFullbright1(&position_id, &projection_id, &modelview_id, &color_id, &opacity_id);
        glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);

        if(t < 4.f){glUniform1f(opacity_id, 1.f);}
        else       {glUniform1f(opacity_id, 1.f-(t-4.f));}
        vec ld = look_dir;
        vMulS(&ld, ld, 1.79f);
        vec np = (vec){-pp.x, -pp.y, pp.z};
        vAdd(&np, np, ld);

        glBindBuffer(GL_ARRAY_BUFFER, esModelArray[62].vid);
        glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(position_id);

        glBindBuffer(GL_ARRAY_BUFFER, esModelArray[62].cid);
        glVertexAttribPointer(color_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(color_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, esModelArray[62].iid);

        mIdent(&model);
        //mScale1(&model, 1.3f);
        mSetPos(&model, np);
        mRotZ(&model, -xrot);
        mMul(&modelview, &model, &view);
        updateModelView();
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        esRenderModel(62);
        glDisable(GL_BLEND);

        shadeLambert3(&position_id, &projection_id, &modelview_id, &lightpos_id, &normal_id, &color_id, &opacity_id);
        glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
    }

    // render intro
    if(t > 5.f && t < 10.f)
    {
        if(t < 9.f){glUniform1f(opacity_id, 1.f);}
        else       {glUniform1f(opacity_id, 1.f-(t-9.f)); focus = 0;} // kill smooth drop-in now [3]
        vec ld = look_dir;
        vMulS(&ld, ld, 1.79f);
        vec np = (vec){-pp.x, -pp.y, pp.z};
        vAdd(&np, np, ld);

        esBindModel(60);
        mIdent(&model);
        mScale1(&model, 0.5f);
        mSetPos(&model, np);
        mRotZ(&model, -xrot);
        mMul(&modelview, &model, &view);
        updateModelView();
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        esRenderModel(60);
        glDisable(GL_BLEND);
    }

    // render brown bread for ded, crossbones
    // looks like it's rotating above tux which
    // was unintentional but looks cool so I'ma
    // leave it like that.
    if(health <= 0)
    {
        vec ld = look_dir;
        vMulS(&ld, ld, 1.79f);
        vec np = (vec){-pp.x, -pp.y, pp.z};
        vAdd(&np, np, ld);

        esBindModel(61);
        mIdent(&model);
        mSetPos(&model, np);
        mRotZ(&model, t*2.f);
        mMul(&modelview, &model, &view);
        updateModelView();
        glClear(GL_DEPTH_BUFFER_BIT);
        esRenderModel(61);
    }

    ///

    // display render
    glfwSwapBuffers(window);
}

//*************************************
// input
//*************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        if(     key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { keystate[0] = 1; }
        else if(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { keystate[1] = 1; }
        else if(key == GLFW_KEY_W || key == GLFW_KEY_UP)    { keystate[2] = 1; }
        else if(key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { keystate[3] = 1; }
        else if(key == GLFW_KEY_SPACE)                      { keystate[5] = 1; }
        else if(key == GLFW_KEY_LEFT_SHIFT ||
                key == GLFW_KEY_RIGHT_CONTROL)              { keystate[4] = 1; }
        else if(key == GLFW_KEY_ESCAPE) // toggle mouse focus
        {
            focus_cursor = 0;
#ifndef WEB
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwGetCursorPos(window, &lx, &ly);
#endif
        }
        else if(key == GLFW_KEY_C) // flip between first person and third person
        {
            static float lz = -0.18f;
            if(zoom >= -0.11f)
            {
                zoom = lz;
            }
            else
            {
                lz = zoom;
                zoom = 0.f;
            }
        }
        else if(key == GLFW_KEY_V) // sticky mouse clicks
        {
            sticky = 1 - sticky;
        }
        else if(key == GLFW_KEY_1 && weapon >= 0){wield = 0;}
        else if(key == GLFW_KEY_2 && weapon >= 1){wield = 1;}
        else if(key == GLFW_KEY_3 && weapon >= 2){wield = 2;}
        else if(key == GLFW_KEY_4 && weapon >= 3){wield = 3;}
        else if(key == GLFW_KEY_5 && weapon >= 4){wield = 4;}
        else if(key == GLFW_KEY_6 && weapon >= 5){wield = 5;}
        else if(key == GLFW_KEY_7 && weapon >= 6){wield = 6;}
        else if(key == GLFW_KEY_8 && weapon >= 7){wield = 7;}
        else if(key == GLFW_KEY_9 && weapon >= 8){wield = 8;}
        else if(key == GLFW_KEY_F) // show average fps
        {
            if(t-lfct > 2.0)
            {
                char strts[16];
                timestamp(&strts[0]);
                printf("[%s] FPS: %g\n", strts, fc/(t-lfct));
                lfct = t;
                fc = 0;
            }
        }
    }
    else if(action == GLFW_RELEASE)
    {
        if(     key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { keystate[0] = 0; }
        else if(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { keystate[1] = 0; }
        else if(key == GLFW_KEY_W || key == GLFW_KEY_UP)    { keystate[2] = 0; }
        else if(key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { keystate[3] = 0; }
        else if(key == GLFW_KEY_SPACE)                      { keystate[5] = 0; }
        else if(key == GLFW_KEY_LEFT_SHIFT ||
                key == GLFW_KEY_RIGHT_CONTROL)              { keystate[4] = 0; }
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset < 0.0){zoom += 0.06f * zoom;}else{zoom -= 0.06f * zoom;}
    if(zoom > -0.11f){zoom = -0.11f;}
    else if(zoom < -3.f){zoom = -3.f;}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    static uint left_sticky_state = 0;
    static uint right_sticky_state = 0;
    static float lz = 0.f;
    if(action == GLFW_PRESS)
    {
        if(focus_cursor == 0)
        {
            focus_cursor = 1;
#ifndef WEB
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lx, &ly);
#endif
            return;
        }
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if(sticky == 0)
            {
                swipe = 1;
                swipe_phase = 0;
                swipe_lt = 0.f;
                attack = 1;
            }
            else
            {
                if(left_sticky_state == 0)
                {
                    swipe = 1;
                    swipe_phase = 0;
                    swipe_lt = 0.f;
                    attack = 1;
                }
                else
                {
                    swipe = 0;
                    swipe_phase = 0;
                    swipe_lt = 0.f;
                    attack = 0;
                }
                left_sticky_state = 1 - left_sticky_state;
            }
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if(sticky == 0)
            {
                if(zoom >= -0.11f)
                {
                    lz = -0.10f;
                    zoom = -0.18f;
                }else{lz = 0.f;}
                focus = 1;
            }
            else
            {
                if(right_sticky_state == 0)
                {
                    if(zoom >= -0.11f)
                    {
                        lz = -0.10f;
                        zoom = -0.18f;
                    }else{lz = 0.f;}
                    focus = 1;
                }
                else
                {
                    if(lz != 0.f){zoom = lz;}
                    focus = 0;
                }
                right_sticky_state = 1 - right_sticky_state;
            }
        }
    }
    else if(sticky == 0 && action == GLFW_RELEASE)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            swipe = 0;
            swipe_phase = 0;
            swipe_lt = 0.f;
            attack = 0;
            left_sticky_state = 0;
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if(lz != 0.f){zoom = lz;}
            focus = 0;
            right_sticky_state = 0;
        }
    }
}
void window_size_callback(GLFWwindow* window, int width, int height)
{
    winw = width, winh = height;
    glViewport(0, 0, winw, winh);
    aspect = (float)winw / (float)winh;
    ww = winw, wh = winh;
    mIdent(&projection);
    mPerspective(&projection, 60.0f, aspect, 0.01f, FAR_DISTANCE);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
}
#ifdef WEB
EM_BOOL emscripten_resize_event(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
    winw = uiEvent->documentBodyClientWidth;
    winh = uiEvent->documentBodyClientHeight;
    window_size_callback(window, winw, winh);
    emscripten_set_canvas_element_size("canvas", winw, winh);
    return EM_FALSE;
}
#endif

//*************************************
// process entry point
//*************************************
int main(int argc, char** argv)
{
    // allow custom msaa level & mouse sensitivity
    int msaa = 16;
    if(argc >= 2){msaa = atoi(argv[1]);}
    if(argc >= 3){sens = atof(argv[2]);}

    // help
    printf("----\n");
    printf("James William Fletcher (github.com/mrbid) (notabug.org/Vandarin)\n");
    printf("%s - A mythical adventure as Tux!\nInspired by the famous game Run-Escape.\n", appTitle);
    printf("----\n");
#ifndef WEB
    printf("Two command line arguments, msaa 0-16, mouse sensitivity.\n");
    printf("e.g; ./tuxscape 16 0.003\n");
    printf("----\n");
#endif
    printf("All your stats are shown in the title bar. ;)\n");
    printf("ESCAPE = Unlock Mouse\n");
    printf("Left Click = Attack\n");
    printf("Right Click = Target Weapon / Engage Jet Pack\n");
    printf("W,A,S,D / Arrow Keys = Move\n");
    printf("L-SHIFT / R-CTRL = Sprint/Fast\n");
    printf("Space = Jet Pack\n");
    printf("1-9 = Weapon Change\n");
    printf("C = Toggle between First and Third person\n");
    printf("V = Toggle between stickey/toggle mouse clicks (good for afk)\n");
    printf("----\n");
    printf("Tux made by Andy Cuccaro\n");
    printf("https://sketchfab.com/3d-models/tux-157de95fa4014050a969a8361a83d366\n");
    printf("CC BY 4.0 DEED (Attribution 4.0 International)\n");
    printf("https://creativecommons.org/licenses/by/4.0/\n");
    printf("----\n");
    printf("Font is Plastic Love by Azkarizki\n");
    printf("https://www.fontspace.com/plastic-love-font-f49676\n");
    printf("----\n");
    printf("All other assets where generated using https://Meshy.ai\n");
    printf("----\n");
    printf("🎄 Merry Christmas 2023! and a happy new year 2024! 🎅\n");
    printf("----\n");
    printf("%s\n", glfwGetVersionString());
    printf("----\n");

    // init glfw
    if(!glfwInit()){printf("glfwInit() failed.\n"); exit(EXIT_FAILURE);}
#ifdef WEB
    double width, height;
    emscripten_get_element_css_size("body", &width, &height);
    winw = (uint)width, winh = (uint)height;
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, msaa);
    window = glfwCreateWindow(winw, winh, appTitle, NULL, NULL);
    if(!window)
    {
        printf("glfwCreateWindow() failed.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    const GLFWvidmode* desktop = glfwGetVideoMode(glfwGetPrimaryMonitor());
#ifndef WEB
    glfwSetWindowPos(window, (desktop->width/2)-(winw/2), (desktop->height/2)-(winh/2)); // center window on desktop
#endif
    //if(glfwRawMouseMotionSupported() == GLFW_TRUE){glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);} // raw input
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1); // 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for adaptive vsync

    // set icon
    glfwSetWindowIcon(window, 1, &(GLFWimage){16, 16, (unsigned char*)icon_image});

//*************************************
// bind vertex and index buffers
//*************************************
    register_knife();
    register_tux();
    register_treeman();
    register_flame1();
    register_flame2();
    register_f1();
    register_f2();
    register_f3();
    register_tree1();
    register_hot();
    register_box1();
    register_box2();
    register_box3();
    register_box4();
    register_box5();
    register_box6();
    register_hp1();
    register_hp2();
    register_hp3();
    register_hp4();
    register_hp5();
    register_hp6();
    register_hp7();
    register_hp8();
    register_hp9();
    register_hp10();
    register_hp11();
    register_hp12();
    register_hp13();
    register_hp14();
    register_shield1();
    register_shield2();
    register_shield3();
    register_shield4();
    register_wep1();
    register_wep2();
    register_wep3();
    register_wep4();
    register_wep5();
    register_wep6();
    register_wep7();
    register_wep8();
    register_zombie();
    register_dinofly();
    register_babybluedragon();
    register_skull();
    register_tornado();
    register_dragphent();
    register_alien();
    register_aliendog();
    register_redknight();
    register_samurai();
    register_wizard();
    register_knight();
    register_ogre();
    register_golum();
    register_bear();
    register_warewolf();
    register_bluedragon();
    register_reddragon();
    register_intro();
    register_crossbones();
    register_meshy();

//*************************************
// compile & link shader programs
//*************************************
    makeFullbright1();
    makeLambert3();

//*************************************
// configure render options
//*************************************
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDisable(GL_CULL_FACE);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.745f, 0.8863f, 0.f);

    shadeLambert3(&position_id, &projection_id, &modelview_id, &lightpos_id, &normal_id, &color_id, &opacity_id);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
    window_size_callback(window, winw, winh);

//*************************************
// execute update / render loop
//*************************************

    // init
    srand(1337);
    //
    islands[0]  = (vec){0.f, 0.f, 0.f};
    islands[1]  = (vec){-5.111959457397461, -2.537980794906616, 1.7169551849365234};
    islands[2]  = (vec){-3.8676228523254395, 2.3920435905456543, 0.9446221590042114};
    islands[3]  = (vec){-9.064745903015137, 4.180261611938477, 1.414565086364746};
    islands[4]  = (vec){5.78463888168335, 1.0283396244049072, 1.2410753965377808};
    islands[5]  = (vec){-0.30623170733451843, -6.905810832977295, 0.6128108501434326};
    islands[6]  = (vec){-1.2681341171264648, 8.702009201049805, 2.87101411819458};
    islands[7]  = (vec){10.831393241882324, 8.512404441833496, 4.448702812194824};
    islands[8]  = (vec){-6.922382354736328, 12.545719146728516, 1.4647904634475708};
    islands[9]  = (vec){9.27226734161377, -8.284314155578613, 6.999974727630615};
    islands[10] = (vec){-14.254311561584473, -4.658764839172363, 3.1255948543548584};
    islands[11] = (vec){-6.994725704193115, -10.644046783447266, 5.3075456619262695};
    islands[12] = (vec){2.689924478530884, -12.381268501281738, 6.50068473815918};
    islands[13] = (vec){13.379647254943848, -2.158463716506958, 3.1220195293426514};
    islands[14] = (vec){-14.951984405517578, 10.784103393554688, 4.535021781921387};
    islands[15] = (vec){2.7339773178100586, 15.037919044494629, 5.312806129455566};
    islands[16] = (vec){-5.857306957244873, 23.18818473815918, 3.434933662414551};
    islands[17] = (vec){-23.571313858032227, 2.7101147174835205, 7.687347888946533};
    islands[18] = (vec){15.814525604248047, -15.12582015991211, 12.158363342285156};
    islands[19] = (vec){21.189453125, 16.37828826904297, 11.15870475769043};
    islands[20] = (vec){-18.295467376708984, -24.455711364746094, 9.090770721435547};
    //
    islands_rad[0] = 1.3;
    islands_rad[1] = 2.4513;
    islands_rad[2] = 3.4796;
    islands_rad[3] = 3.3645;
    islands_rad[4] = 2.2584;
    islands_rad[5] = 2.0078;
    islands_rad[6] = 3.2239;
    islands_rad[7] = 4.8083;
    islands_rad[8] = 3.2242;
    islands_rad[9] = 3.2239;
    islands_rad[10] = 5.4668;
    islands_rad[11] = 4.3868;
    islands_rad[12] = 4.8083;
    islands_rad[13] = 5.4668;
    islands_rad[14] = 4.8083;
    islands_rad[15] = 4.3868;
    islands_rad[16] = 7.9776;
    islands_rad[17] = 8.6786;
    islands_rad[18] = 9.8603;
    islands_rad[19] = 12.0499;
    islands_rad[20] = 15.7159;
    //
    islands_type[0] = 5;
    for(uint i=1; i < MAX_ISLANDS; i++){islands_type[i] = esRand(5, 7);}
    pp = (vec){0.f, 0.5f, 0.9f};
    focus = 1; // smooth drop-in [3]
    t = fTime();
    lfct = t;

#ifdef WEB
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, emscripten_resize_event);
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while(!glfwWindowShouldClose(window)){main_loop();}
#endif

    // done
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
