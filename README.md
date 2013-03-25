# moyai

An experimental simple 2D/3D game engine for native C++, strongly inspired by MoaiSDK API structure.

Moai SDK: https://github.com/moai/moai-dev

 - Rendering
  - 2D sprite + 3D mesh
  - OpenGL 2.0 (ES2), [GLFW http://www.glfw.org/]
 - Image
  - PNG read/write from file
  - dynamic texture
 - Audio
  - Reads any WAV files
  - FMOD based ( http://fmod.com/ )
 - Font
  - FreeType2 based, reads TTF files, multi-byte chars
  

# Small
Moyai is just combining good existing stable libs so itself has only 2K lines of C++ code.

<pre>
[@Macintosh-2 moyai]$ wc moyai.cpp moyai.h cumino.cpp cumino.h
    1160    3794   38004 moyai.cpp
    1432    5057   41818 moyai.h
     321     857    7495 cumino.cpp
     144     513    3218 cumino.h
    3057   10221   90535 total
</pre>

# Example

All of its class names are from MoaiSDK's, 
and the order of API calls is also completely same as MoaiSDK's.

Some arguments are modified to achieve even simpler usage.



## Sprite basics

To show a sprite on your screen, just:

<pre lang="c++">
<code>
Moyai *g_moyai;

int main() 
{
  g_moyai = new Moyai();

  Viewport *v = new Viewport();
  v->setSize(1024,768);
  v->setScale(1024,768);

  Layer *l = new Layer();
  lay->setViewport(v);
  
  Camera *c = new Camera();
  c->setLoc(0,0);
  lay->setCamera©;

  g_moyai->insertLayer(lay);

  Texture *tex = new Texture();
  tex->load("spritesheet.png");

  TileDeck *deck = new TileDeck();
  deck->setTexture(tex);
  deck->setSize( 16,16, 16,16, 256,256); //16x16 of 16pix x 16pix sprites in a sheet

  Prop *p = new Prop();
  p->setDeck(deck);
  p->setLoc(100,100);
  p->setIndex(13); // index number in the sprite sheet (top-down, left-right)
  p->setScl(32,32);

  lay->insertProp℗;

  // initialize glfw
  glfwInit();
  glfwOpenWindow(1024,768, 0,0,0,0, 0,0, GLFW_WINDOW );

  while(1) {
    static double last_poll_at = 0;
    double t = now();
    double dt = t - last_poll_at;
  
    g_moyai->pollAll(dt); // everything happens in 
    g_moyai->renderAll();
    
    last_poll_at = t;
  }
}

</code>
</pre>

## 3D basics
<pre lang="c++">
<code>
int main() {
  g_moyai = new Moyai();

  glfwInit();
  glfwOpenWindow( SCRW,SCRH, 8,8,8,8, 8,0, GLFW_WINDOW );
  glfwSetWindowTitle( "demo3d");
  glfwEnable( GLFW_STICKY_KEYS );
  glfwSwapInterval(1); // vsync

  glClearColor(0,0,0,1);
  glEnable(GL_DEPTH_TEST);    
  glEnable(GL_DEPTH_BUFFER_BIT);    
  glDepthMask(true );
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
    
  Viewport *v = new Viewport();
  v->setSize(1024,768);
  v->setClip3D( 0.01, 100 ); // near and far clip
  
  Camera *cam = new Camera();
  cam->setLoc( Vec3(0,0,10) );
  
  Layer *layer = new Layer();
  layer->setCamera(cam);
  layer->setViewport(v);

  Texture *t = new Texture();
  t->load("wood_surface.png");
  Prop3d *p = new Prop3D();
  p->setMesh(mesh); // prepare mesh before
  p->setTexture(t);
  p->setScl(Vec3(2,2,2));
  p->setLoc(Vec3(1,0,0));
  layer->insertProp(p);

  while(1){
    static double last_poll_at = now();
    double t = now();
    double dt = t - last_poll_at;
    g_moyai->pollAll(dt);
    g_moyai->renderAll();
    last_poll_at = t;
  }
  
}
</code>
</pre>

## Audio

To use sound effects or background music, just use SoundSystem and Sound.

It is simply based on FMOD: stable, small and lightweight.

<pre lang="c">
<code>
SoundSystem *ss = new SoundSystem();
Sound *s = ss->newSound( "explode.wav" );
s->play();
</code>
</pre>

Fonts
----
To use TrueType fonts, just use Font and TextBox.
Pass wchar_t string to prepare glyphs in texture buffer.
Moyai only supports pre-loaded fonts.


<pre lang="c">
<code>
wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん";    
 
Font *f = new Font();
f->loadFromTTF( "cinecaption227.ttf", charcodes, 12 ); 

TextBox *tb = new TextBox();
tb->setFont(f);
tb->setString("こんにちはABC");

layer->insertProp(tb);

</code>
</pre>

Screen capture
----
<pre lang="c">
<code>
Image *img = new Image();
img->setSize(1024,768);
g_moyai->capture(img);
img->writePNG("captured.png");
delete img;
</code>
</pre>

Build
=====

Tested only on MacOS X 10.8. After installing Xcode and command line tools, just do:
 
<pre>
$ git clone https://github.com/kengonakajima/moyai
$ cd moyai
$ make
$ ./demo2d
$ ./demo3d
</pre>

Classes
=====
List of classes defined in moyai.h :

- Basic types
  - <code>Vec2</code>
  - <code>Color</code>
  - <code>Moyai</code>
  - <code>Layer</code>
  - <code>Viewport</code>
  - <code>Camera</code> 
  - <code>Prop2D</code> : simple sprite
  - <code>Prop3D</code> 
  
- Rendering and Graphics
  - <code>Image</code>  : image in memory
  - <code>Texture</code> : opengl texture
  - <code>TileDeck</code> : sprite sheet
  - <code>ColorReplacerShader</code> : replace pixel color of given texture 
  - <code>Grid</code> : m x n grid
  - <code>CharGrid</code> : for simple ascii bitmap fonts
  - <code>Prim</code> : draw lines and rectangles
- Text and fonts  
  - <code>Font</code> : truetype font
  - <code>TextBox</code>
 
- Animation
  - <code>AnimCurve</code>
  - <code>Animation</code>  

- Audio
  - <code>SoundSystem</code>
  - <code>Sound</code>
 