﻿#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"


MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Camera *g_camera;

GLFWwindow *g_window;


// config

static const int SCRW=966, SCRH=544;



void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}

int main(int argc, char **argv )
{
    

    print("program start");

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    
    
    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        return 1;
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "demo2d", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // vsync
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    g_moyai_client = new MoyaiClient(g_window);

    g_viewport = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    g_viewport->setSize(SCRW*retina,SCRH*retina); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered
    
    Layer *l = new Layer();
    g_moyai_client->insertLayer(l);
    l->setViewport(g_viewport);

    Texture *t = new Texture();
    t->load( "./assets/base.png" );
    
    TileDeck *deck = new TileDeck();
    deck->setTexture(t);
    deck->setSize(32,32,8,8);

    Prop2D *p = new Prop2D();
    p->setDeck(deck);
    p->setIndex(1);
    p->setScl(64,64);
    p->setLoc(0,0);
    l->insertProp(p);

    g_main_layer = new Layer();
    g_moyai_client->insertLayer(g_main_layer);
    g_main_layer->setViewport(g_viewport);

    g_camera = new Camera();
    g_camera->setLoc(0,0);

    g_main_layer->setCamera(g_camera);

    while( !glfwWindowShouldClose(g_window) ){
        int scrw, scrh;
        glfwGetFramebufferSize( g_window, &scrw, &scrh );

        static double last_print_at = 0;
        static int frame_counter = 0;
        static double last_poll_at = now();

        double t = now();
        double dt = t - last_poll_at;
        last_poll_at = t;
        
        frame_counter ++;

        Vec2 at(::sin(t)*100,0);
        p->setLoc(at);
        p->setIndex( irange(0,3));
        static float rot=0;
        rot+=2;
        p->setRot(rot);
        p->setScl( 40 + ::sin(t) * 30 );
        int cnt = g_moyai_client->poll(dt);
        if( frame_counter % 50 == 0 ) {
            float alpha = range(0.2, 1.0f);
            Color col(range(0,1),range(0,1),range(0,1),alpha);
            p->setColor(col);
        }        

        if(last_print_at == 0){
            last_print_at = t;
        } else if( last_print_at < t-1 ){
            fprintf(stderr,"FPS:%d prop:%d\n", frame_counter, cnt  );
            frame_counter = 0;
            last_print_at = t;
        }

        g_moyai_client->render();

        if( glfwGetKey( g_window, 'Q') ) {
            print("Q pressed");
            exit(0);
            break;
        }
        glfwPollEvents();
    }
    glfwTerminate();

    print("program finished");
    

    return 0;
}