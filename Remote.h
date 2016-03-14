#ifndef _REMOTE_H_
#define _REMOTE_H_

#if defined(WIN32)
#include <stdint.h>
#endif

#include <uv.h>

#include "Pool.h"

// basic buffering

extern inline unsigned int get_u32(const char *buf){ return *((unsigned int*)(buf)); }
extern inline unsigned short get_u16(const char *buf){ return *((unsigned short*)(buf)); }
extern inline unsigned char get_u8(const char *buf){ return *((unsigned char*)(buf)); }
extern inline void set_u32(char *buf, unsigned int v){ (*((unsigned int*)(buf))) = (unsigned int)(v) ; }
extern inline void set_u16(char *buf, unsigned short v){ (*((unsigned short*)(buf))) = (unsigned short)(v); }
extern inline void set_u8( char *buf, unsigned char v){ (*((unsigned char*)(buf))) = (unsigned char)(v); }
extern inline float get_f32(const char *buf) { return *((float*)(buf)); }


// packet structures
typedef struct {
    float x,y;
} PacketVec2;
typedef struct {
    float x,y,z;
} PacketVec3;
typedef struct {
    float r,g,b,a;
} PacketColor;
typedef struct  {
    uint32_t prop_id; // non-zero
    uint32_t layer_id; // non-zero for layer, zero for child props
    uint32_t parent_prop_id; // non-zero for child props, zero for layer props
    PacketVec2 loc;
    PacketVec2 scl;
    int32_t index;
    uint32_t tiledeck_id; // non-zero
    uint32_t grid_id; // 0 for nothing
    int32_t debug;
    float rot;
    uint32_t xflip; // TODO:smaller size
    uint32_t yflip;
    uint32_t uvrot; 
    PacketColor color;
    uint32_t shader_id;
    uint32_t optbits; 
} PacketProp2DSnapshot;

#define PROP2D_OPTBIT_ADDITIVE_BLEND 0x00000001

typedef struct {
    uint32_t shader_id;
    float epsilon;
    PacketColor from_color;
    PacketColor to_color;
} PacketColorReplacerShaderSnapshot;

inline void copyColorToPacketColor( PacketColor *dest, Color *src ) {
    dest->r = src->r;
    dest->g = src->g;
    dest->b = src->b;
    dest->a = src->a;    
}
inline void copyPacketColorToColor( Color *dest, PacketColor *src ) {
    dest->r = src->r;
    dest->g = src->g;
    dest->b = src->b;
    dest->a = src->a;        
}

typedef struct {
    uint32_t prim_id;
    uint8_t prim_type; // from PRIMTYPE
    PacketVec2 a;
    PacketVec2 b;
    PacketColor color;
    float line_width;
} PacketPrim;

#define MAX_PACKET_SIZE (1024*8)

///////
// HMP: Headless Moyai Protocol
#if 0
class RemoteHead;
class HMPListener : public Listener {
public:
    RemoteHead *remote_head;
    HMPListener(Network *nw, RemoteHead *rh) : Listener(nw), remote_head(rh) {};
    virtual ~HMPListener(){};
    virtual void onAccept( int newfd );    
};
class HMPConn : public Conn {
public:
    RemoteHead *remote_head;
    HMPConn( RemoteHead *rh, Network *nw, int newfd ) : Conn(nw,newfd), remote_head(rh) {};
    virtual ~HMPConn() {};
    virtual void onError( NET_ERROR e, int eno );
    virtual void onClose();
    virtual void onConnect();
    virtual void onPacket( uint16_t funcid, char *argdata, size_t argdatalen );

    // send
    void sendFile( const char *filename );
};
#endif

class Prop2D;
class MoyaiClient;
class Grid;
class ColorReplacerShader;
class PrimDrawer;
class SoundSystem;
class Keyboard;
class Mouse;

typedef std::unordered_map<unsigned int,uv_stream_t*>::iterator UvStreamIteratorType;

class RemoteHead {
public:
    int tcp_port;
    uv_tcp_t listener;
    MoyaiClient *target_moyai;
    SoundSystem *target_soundsystem;
    Keyboard *target_keyboard;
    Mouse *target_mouse;
    ObjectPool<uv_stream_t> stream_pool;
    
    static const int DEFAULT_PORT = 22222;
    RemoteHead() : tcp_port(0), target_moyai(0), target_soundsystem(0), target_mouse(0) {
    }
    void track2D();
    bool startServer( int portnum );
    void heartbeat();
    void scanSendAllPrerequisites( uv_stream_t *outstream );
    void scanSendAllProp2DSnapshots( uv_stream_t *outstream );
    void notifyProp2DDeleted( Prop2D *prop_deleted );
    void notifyGridDeleted( Grid *grid_deleted );
    void notifyChildCleared( Prop2D *owner_prop, Prop2D *child_prop );
    void setTargetSoundSystem(SoundSystem*ss) { target_soundsystem = ss; }
    void setTargetMoyaiClient(MoyaiClient*mc) { target_moyai = mc; }
    void setTargetKeyboard(Keyboard*kbd) { target_keyboard = kbd; }
    void setTargetMouse(Mouse*mou) { target_mouse = mou; }
    
    void broadcastUS1Bytes( uint16_t usval, const char *data, size_t datalen );
    void broadcastUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *data, size_t datalen );    
    void broadcastUS1UI1( uint16_t usval, uint32_t uival );
    void broadcastUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1 );
    void broadcastUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 );
    void broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
    void broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 );
    void broadcastUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 );    
    
};


typedef enum {
    // generic
    PACKETTYPE_PING = 1,    
    // client to server 
    PACKETTYPE_C2S_KEYBOARD = 200,
    PACKETTYPE_C2S_MOUSE_BUTTON = 202,
    PACKETTYPE_C2S_CURSOR_POS = 203,
    PACKETTYPE_C2S_TOUCH_BEGIN = 204,
    PACKETTYPE_C2S_TOUCH_MOVE = 205,
    PACKETTYPE_C2S_TOUCH_END = 206,
    PACKETTYPE_C2S_TOUCH_CANCEL = 207,

    // server to client
    PACKETTYPE_S2C_PROP2D_SNAPSHOT = 400, 
    PACKETTYPE_S2C_PROP2D_LOC = 401,
    PACKETTYPE_S2C_PROP2D_GRID = 402,
    PACKETTYPE_S2C_PROP2D_INDEX = 403,
    PACKETTYPE_S2C_PROP2D_SCALE = 404,
    PACKETTYPE_S2C_PROP2D_ROT = 405,
    PACKETTYPE_S2C_PROP2D_XFLIP = 406,
    PACKETTYPE_S2C_PROP2D_YFLIP = 407,
    PACKETTYPE_S2C_PROP2D_COLOR = 408,
    PACKETTYPE_S2C_PROP2D_DELETE = 410,
    PACKETTYPE_S2C_PROP2D_CLEAR_CHILD = 412,
    
    PACKETTYPE_S2C_LAYER_CREATE = 420,
    PACKETTYPE_S2C_LAYER_VIEWPORT = 421,
    PACKETTYPE_S2C_LAYER_CAMERA = 422,
    PACKETTYPE_S2C_VIEWPORT_CREATE = 430,
    PACKETTYPE_S2C_VIEWPORT_SIZE = 431,
    PACKETTYPE_S2C_VIEWPORT_SCALE = 432,    
    PACKETTYPE_S2C_CAMERA_CREATE = 440,
    PACKETTYPE_S2C_CAMERA_LOC = 441,
    
    PACKETTYPE_S2C_TEXTURE_CREATE = 450,
    PACKETTYPE_S2C_TEXTURE_IMAGE = 451,
    PACKETTYPE_S2C_IMAGE_CREATE = 460,
    PACKETTYPE_S2C_IMAGE_LOAD_PNG = 461,
    PACKETTYPE_S2C_IMAGE_ENSURE_SIZE = 464,
    PACKETTYPE_S2C_IMAGE_RAW = 465,
    
    PACKETTYPE_S2C_TILEDECK_CREATE = 470,
    PACKETTYPE_S2C_TILEDECK_TEXTURE = 471,
    PACKETTYPE_S2C_TILEDECK_SIZE = 472,
    PACKETTYPE_S2C_GRID_CREATE = 480, // with its size (id,w,h)
    PACKETTYPE_S2C_GRID_DECK = 481, // with gid,tdid
    PACKETTYPE_S2C_GRID_PROP2D = 482, // with gid,propid    
    PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT = 484, // index table, array of int32_t
    PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT = 485, // xfl|yfl|uvrot bitfield in array of uint8_t
    PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT = 486, //  array of Vec2
    PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT = 487, // color table, array of PacketColor: 4 * float32    
    PACKETTYPE_S2C_GRID_DELETE = 490,

    PACKETTYPE_S2C_TEXTBOX_CREATE = 500, // tb_id, uint32_t
    PACKETTYPE_S2C_TEXTBOX_FONT = 501,    // tb_id, font_id
    PACKETTYPE_S2C_TEXTBOX_STRING = 502,    // tb_id, utf8str
    PACKETTYPE_S2C_TEXTBOX_LOC = 503,    // tb_id, x,y
    PACKETTYPE_S2C_TEXTBOX_SCL = 504,    // tb_id, x,y    
    PACKETTYPE_S2C_TEXTBOX_COLOR = 505,    // tb_id, PacketColor
    PACKETTYPE_S2C_TEXTBOX_LAYER = 510,     // tb_id, l_id
    PACKETTYPE_S2C_FONT_CREATE = 540, // fontid, utf8 string array
    PACKETTYPE_S2C_FONT_CHARCODES = 541, // fontid, utf8str
    PACKETTYPE_S2C_FONT_LOADTTF = 542, // fontid, filepath    

    PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT = 600, //
    PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT = 610, // array of PacketPrim

    PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE = 650,
    PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES = 651,
    PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME = 653,
    PACKETTYPE_S2C_SOUND_PLAY = 660,
    PACKETTYPE_S2C_SOUND_STOP = 661,    
    PACKETTYPE_S2C_SOUND_POSITION = 662,
    
    PACKETTYPE_S2C_FILE = 800, // send file body and path
    
    PACKETTYPE_ERROR = 2000, // error code
} PACKETTYPE;


class Prop2D;
class Tracker2D {
public:
    Prop2D *target_prop2d;
    PacketProp2DSnapshot pktbuf[2]; // flip-flop    
    int cur_buffer_index;
    RemoteHead *parent_rh;
    Tracker2D(RemoteHead *rh, Prop2D *target ) : target_prop2d(target), cur_buffer_index(0), parent_rh(rh) {
        memset( pktbuf, 0, sizeof(pktbuf) );
    }
    ~Tracker2D();
    void scanProp2D( Prop2D *parentprop );
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );
};
typedef enum {
    GTT_INDEX = 1,
    GTT_FLIP = 2,
    GTT_TEXOFS = 3,
    GTT_COLOR = 4,
} GRIDTABLETYPE;

#define GTT_FLIP_BIT_X 0x01
#define GTT_FLIP_BIT_Y 0x02
#define GTT_FLIP_BIT_UVROT 0x04

class TrackerGrid {
public:
    Grid *target_grid;
    int32_t *index_table[2];
    uint8_t *flip_table[2]; // ORing GTT_FLIP_BIT_*
    PacketVec2 *texofs_table[2];
    PacketColor *color_table[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerGrid(RemoteHead *rh, Grid *target);
    ~TrackerGrid();
    void scanGrid();
    bool checkDiff(GRIDTABLETYPE gtt);
    void flipCurrentBuffer();
    void broadcastDiff( Prop2D *owner, bool force );
    void broadcastGridConfs( Prop2D *owner ); // util sendfunc
};

class TextBox;
class TrackerTextBox {
public:
    TextBox *target_tb;
    static const int MAX_STR_LEN = 1024;
    // flip flop diff checker
    uint8_t strbuf[2][MAX_STR_LEN];
    size_t str_bytes[2];
    PacketProp2DSnapshot pktbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;    
    TrackerTextBox(RemoteHead *rh, TextBox *target);
    ~TrackerTextBox();
    void scanTextBox();
    void flipCurrentBuffer();
    bool checkDiff();    
    void broadcastDiff( bool force );
};

class TrackerColorReplacerShader {
public:
    ColorReplacerShader *target_shader;
    PacketColorReplacerShaderSnapshot pktbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerColorReplacerShader(RemoteHead *rh, ColorReplacerShader *target ) : target_shader(target), cur_buffer_index(0), parent_rh(rh) {};
    ~TrackerColorReplacerShader();
    void scanShader();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );    
};

class TrackerPrimDrawer {
public:
    PrimDrawer *target_pd;
    PacketPrim *pktbuf[2]; // Each points to an array of PacketPrim
    int pktnum[2];
    int pktmax[2]; // malloced size
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerPrimDrawer( RemoteHead *rh, PrimDrawer *target )     : target_pd(target), cur_buffer_index(0), parent_rh(rh) {
        pktbuf[0] = pktbuf[1] = 0;
        pktnum[0] = pktnum[1] = 0;
        pktmax[0] = pktmax[1] = 0;
    }
    ~TrackerPrimDrawer();
    void scanPrimDrawer();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( Prop2D *owner, bool force );
};

class TrackerImage {
public:
    Image *target_image;
    uint8_t *imgbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerImage( RemoteHead *rh, Image *target );
    ~TrackerImage();
    void scanImage();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( TileDeck *owner_dk, bool force );

};
class Camera;
class TrackerCamera {
public:
    Camera *target_camera;
    Vec2 locbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerCamera( RemoteHead *rh, Camera *target );
    ~TrackerCamera();
    void scanCamera();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );    
};


class Buffer {
public:
    char *buf;
    size_t size;
    size_t used;
    Buffer();
    ~Buffer();
    void ensureMemory( size_t sz );
    size_t getRoom() { return size - used; }
    bool shift( size_t toshift );    
    bool pushWithNum32( const char *data, size_t datasz );
    bool push( const char *data, size_t datasz );
    bool pushU32( unsigned int val );
    bool pushU16( unsigned short val );
    bool pushU8( unsigned char val );
            
};

class Client {
public:
    static int idgen;
    int id;
    Buffer recvbuf;
    uv_tcp_t *tcp;
    RemoteHead *parent_rh;
    Client( uv_tcp_t *sk, RemoteHead *rh );
    ~Client();
    bool receiveData( const char *data, size_t datalen );
    void onPacket( uint16_t funcid, char *argdata, size_t argdatalen );    
};



// send funcs
int sendUS1( uv_stream_t *out, uint16_t usval );
int sendUS1Bytes( uv_stream_t *out, uint16_t usval, const char *buf, uint16_t datalen );
int sendUS1UI1Bytes( uv_stream_t *out, uint16_t usval, uint32_t uival, const char *buf, uint32_t datalen );
int sendUS1UI1( uv_stream_t *out, uint16_t usval, uint32_t ui0 );
int sendUS1UI2( uv_stream_t *out, uint16_t usval, uint32_t ui0, uint32_t ui1 );    
int sendUS1UI3( uv_stream_t *out, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 );    
int sendUS1UI5( uv_stream_t *out, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3, uint32_t ui4 );
int sendUS1UI1F1( uv_stream_t *out, uint16_t usval, uint32_t uival, float f0 );    
int sendUS1UI1F2( uv_stream_t *out, uint16_t usval, uint32_t uival, float f0, float f1 );
int sendUS1UI1Str( uv_stream_t *out, uint16_t usval, uint32_t uival, const char *cstr );
int sendUS1UI2Str( uv_stream_t *out, uint16_t usval, uint32_t ui0, uint32_t ui1, const char *cstr );
int sendUS1StrBytes( uv_stream_t *out, uint16_t usval, const char *cstr, const char *data, uint32_t datalen );
int sendUS1UI1Wstr( uv_stream_t *out, uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
int sendUS1F2( uv_stream_t *out, uint16_t usval, float f0, float f1 );
void sendFile( uv_stream_t *outstream, const char *filename );


// parse helpers
void parsePacketStrBytes( char *inptr, char *outcstr, char **outptr, size_t *outsize );




#endif