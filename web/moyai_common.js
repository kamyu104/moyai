
DIR4_NONE=-1;
DIR4_UP=0;
DIR4_RIGHT=1;
DIR4_DOWN=2;
DIR4_LEFT=3;


randomDir = function() {
    return irange(DIR4_UP,DIR4_LEFT+1);
}
randomLRDir = function() {
    return [DIR4_LEFT,DIR4_RIGHT][irange(0,2)];
}
reverseDir = function(d){
    switch(d){
    case DIR4_UP: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_UP;
    case DIR4_RIGHT: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_RIGHT;
    default:
        return DIR4_NONE;
    }
}
turnDir = function(origdir,turndir) {
    switch(turndir) {
    case DIR4_LEFT: return leftDir(origdir);
    case DIR4_RIGHT: return rightDir(origdir);
    case DIR4_DOWN: return reverseDir(origdir);
    case DIR4_UP: return origdir;
    default:
        assert(false,"invalid dir");
    }
}
rightDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_RIGHT; 
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_LEFT: return DIR4_UP;
    default:
        return DIR4_NONE;
    }
}
leftDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_LEFT;
    case DIR4_DOWN: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_UP;
    case DIR4_LEFT: return DIR4_DOWN;
    default:
        return DIR4_NONE;
    }    
}

// 4方向のみ
dxdyToDir = function(dx,dy){
    if(dx>0&&dy==0){
        return DIR4_RIGHT;
    } else if(dx<0&&dy==0){
        return DIR4_LEFT;
    } else if(dy>0&&dx==0){
        return DIR4_UP;
    }else if(dy<0&&dx==0){
        return DIR4_DOWN;
    } else {
        return DIR4_NONE;
    }
}
clockDir = function(d) {
    switch(d) {
    case DIR4_NONE: return DIR4_NONE;
    case DIR4_UP: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_UP;
    default: console.assert( "clockDir: invalid direction:", d);
    }
    return DIR4_NONE;
}

dirToDXDY = function(d) {
    switch(d){
    case DIR4_NONE: return null;
    case DIR4_RIGHT: return {x:1,y:0};
    case DIR4_LEFT: return {x:-1,y:0};
    case DIR4_UP: return {x:0,y:1};
    case DIR4_DOWN: return {x:0,y:-1};
    default:
        console.assert("dirToDXDY: invalid direction:",d);
        return null;
    }
}




////

var moyai_rng_w = 123456789;
var moyai_rng_z = 987654321;
var moyai_rng_mask = 0xffffffff;

// Takes any integer
function moyai_rng_seed(i) {
    moyai_rng_w = i;
    moyai_rng_z = 987654321;
}

// Returns number between 0 (inclusive) and 1.0 (exclusive),
// just like Math.random().
function moyai_rng_random()
{
    moyai_rng_z = (36969 * (moyai_rng_z & 65535) + (moyai_rng_z >> 16)) & moyai_rng_mask;
    moyai_rng_w = (18000 * (moyai_rng_w & 65535) + (moyai_rng_w >> 16)) & moyai_rng_mask;
    var result = ((moyai_rng_z << 16) + moyai_rng_w) & moyai_rng_mask;
    result /= 4294967296;
    return result + 0.5;
}

irange = function(a,b) {
    return Math.floor(range(a,b));
}
range = function(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    var out=(small + (big-small)*moyai_rng_random());
    if(out==b)return a; // in very rare case, out==b
    return out;
}
choose = function(ary) {
    return ary[ irange(0,ary.length) ];
}

sign = function(f) {
    if(f>0) return 1; else if(f<0)return -1; else return 0;
}
now = function() {
    var t = new Date().getTime();
    return t / 1000.0;
}
hrnow = function() {
    var t=process.hrtime();
    return t[0] + t[1]/1000000000.0;
}
lengthf = function(x0,y0,x1,y1) {
    return Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
}



//////////////

// 0 ~ 1
Color={};
Color.fromValues= function(r,g,b,a) {
    var out=new Float32Array(4);
    out[0]=r; out[1]=g; out[2]=b; out[3]=a;
    return out;
}
Color.fromCode = function(code) {
    var r = ((code & 0xff0000)>>16)/255;
    var g = ((code & 0xff00)>>8)/255;
    var b = (code & 0xff)/255;
    var a = 1.0;
    return Color.fromValues(r,g,b,a);
}
Color.toRGBA = function(outary,col) {
    outary[0]=Math.floor(this.r*255);
    outary[1]=Math.floor(this.g*255);
    outary[2]=Math.floor(this.b*255);
    outary[3]=Math.floor(this.a*255);
}
Color.fromRGBA = function(outary,r,g,b,a) {
    outary[0] = r/255.0;
    outary[1] = g/255.0;
    outary[2] = b/255.0;    
    outary[3] = a/255.0;
}
Color.toCode = function(col) {
    return ( Math.floor( col[0] * 255) << 16 ) + ( Math.floor(col[1] * 255) << 8 ) + Math.floor(col[2] * 255);
}
Color.copy = function(out,input) {
    out[0]=input[0]; out[1]=input[1]; out[2]=input[2]; out[3]=input[3];
}
Color.exactEquals = function(out,input) {
    return (out[0]===input[0] && out[1]===input[1] && out[2]===input[2] && out[3]===input[3]);
}
Color.exactEqualsToValues = function(out,r,g,b,a) {
    return (out[0]===r && out[1]===g && out[2]===b && out[3]===a);    
}
Color.set = function(out,r,g,b,a) {
    out[0]=r; out[1]=g; out[2]=b; out[3]=a;
}
///////////////////

var g_moyai_viewport_id_gen=1;
class Viewport {
    constructor() {
        this.id = g_moyai_viewport_id_gen++;
        this.screen_width = null;
        this.screen_height = null;
        this.near_clip = null;
        this.far_clip = null;
        this.dimension = null;
    }
    setSize(sw,sh) {
        this.screen_width = sw;
        this.screen_height = sh;
    }
    setScale2D(sx,sy) {
        this.scl = vec2.fromValues(sx,sy);
        this.dimension = 2;
    }
    setClip3D(near,far) {
        this.near_clip = near;
        this.far_clip = far;
        this.dimension = 3;
    }
    getMinMax(outary) {
        var x0=-this.scl[0]/2, y0=-this.scl[1]/2, x1=this.scl[0]/2, y1=this.scl[1]/2;
        vec2.set(outary[0],x0,y0);
        vec2.set(outary[1],x1,y1);
    }
    getRelativeScale(outvec2) {
        vec2.set(outvec2,this.screen_width/this.scl[0],this.screen_height/this.scl[1]);
    }
}


////////////////////
g_moyai_z_per_layer = 100000;
g_moyai_z_per_prop = 1;
g_moyai_z_per_subprop = 1; // this causes some issue when dense sprites.. but no way to implement correct draw order
g_moyai_max_z = g_moyai_z_per_layer*100; // use z to confirm render order ( renderOrder dont work for line prims..)


g_moyai_layer_id_gen=1;
class Layer {
    constructor() {
        this.id = g_moyai_layer_id_gen++;
        this.props=[];
        this.priority=null;// update when insert to moyai
        this.camera=null;
        this.viewport=null;
        this.light=null;
    }
    setViewport(vp) { this.viewport = vp; }
    setCamera(cam) { this.camera = cam; }
    setLight(lgt) { this.light = lgt; }
    setAmbientLight(lgt) { this.ambient_light = lgt; }
    insertProp(p) {
        if(p.priority==null) {
            var highp = this.getHighestPriority();
            p.priority = highp+1;
            p.parent_layer=this;
        }
        this.props.push(p);
    }
    hasProp(p) {
        for(var i=0;i<this.props.length;i++) {
            if(this.props[i].id==p.id) return true;
        }
        return false;
    }
    delProp(p) {
        for(var i=0;i<this.props.length;i++) {
            if(this.props[i].id==p.id) {
                this.props.splice(i,1);
                return true;
            }
        }
        return false;
    }
    pollAllProps(dt) {
        var keep=[];
        for(var i=0;i<this.props.length;i++) {
            var prop = this.props[i];
            var to_keep = prop.basePoll(dt);
            if(to_keep) {
                keep.push(prop);
            } else {
                if(prop.onDelete) prop.onDelete();
            }
        }
        this.props = keep;
        return this.props.length;
    }
    getHighestPriority() {
        var highp=0;
        for(var i=0;i<this.props.length;i++) {
            if(this.props[i].priority>highp) highp = this.props[i].priority;
        }
        return highp;    
    }
    getLowesetPriority() {
        var lowp=0;
        for(var i=0;i<this.props.length;i++) {
            if(this.props[i].priority<lowp) lowp = this.props[i].priority;
        }
        return lowp;
    }
    getPropById(id) {
        for(var i=0;i<this.props.length;i++) {
            if( this.props[i].id == id ) return this.props[i];
        }
        return null;
    }
    findByKey(keyname,val) {
        for(var i=0;i<this.props.length;i++) {
            var p = this.props[i];
            if( p[keyname] == val ) return p;
        }
        return null;
    }
    scan(cb) {
        for(var i=0;i<this.props.length;i++) {
            cb(this.props[i]);
        }
    }
    clean() {
        this.props=[];
    }
};

/////////////////////
MoyaiImage.prototype.id_gen = 1;
function MoyaiImage() {
    this.id = this.__proto__.id_gen++;
    this.data = null;
    this.png=null;
    this.onload=null;
}
MoyaiImage.prototype.loadPNG = function(url,w,h) {
    if(w===undefined||h===undefined) console.warn("loadPNG require width and height currently");    
    var image = new Image();
    image.width=w;
    image.height=h;
    this.width=w;
    this.height=h;
    var moyai_img=this;
    image.onload = function() {
        //        console.log("loadpng: onload:",texture,image,moyai_tex);
        var canvas=document.createElement("canvas");
        var ctx=canvas.getContext("2d");
        ctx.drawImage(this,0,0);
        var imgdata=ctx.getImageData(0,0,w,h);
        console.log("MoyaiImage onload: imgdata",imgdata);
        moyai_img.data=imgdata.data;
        if(moyai_img.onload) moyai_img.onload();
    }
    image.src=url;
    
}
MoyaiImage.prototype.loadPNGMem = function(u8adata) {
    var b = new Buffer(u8adata);
    this.png = pngParse(b);
    this.width = this.png.width;
    this.height = this.png.height;
    this.data = this.png.data;
}
MoyaiImage.prototype.setSize = function(w,h) {
    this.width = w;
    this.height = h;
    if(!this.data) {
        this.data = new Uint8Array(w*h*4);
    }
}
MoyaiImage.prototype.getSize = function(out) {
    vec2.set(out,this.width,this.height);
}
MoyaiImage.prototype.getPixelRaw = function(x,y) {
// int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a ) {
    var out={};
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        out.r = this.data[index];
        out.g = this.data[index+1];
        out.b = this.data[index+2];
        out.a = this.data[index+3];
    }
    return out;
}
MoyaiImage.prototype.setPixelRaw = function(x,y,r,g,b,a) {
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        this.data[index] = r;
        this.data[index+1] = g;
        this.data[index+2] = b;
        this.data[index+3] = a;
    }    
}
MoyaiImage.prototype.setPixel = function(x,y,c) {
    this.setPixelRaw(x,y,Math.floor(c[0]*255),Math.floor(c[1]*255),Math.floor(c[2]*255),Math.floor(c[3]*255));
}
MoyaiImage.prototype.getBufferSize = function() { return this.width * this.height * 4; }
MoyaiImage.prototype.setAreaRaw = function(x0,y0,w,h, data_u8a, insz ) {
    var reqsize = w*h*4;
    if( insz < reqsize ) {
        console.log("image.prototype.setAreaRaw input size too small required:",reqsize, "got:",insz);
        return;
    }
    for(var dy=0;dy<h;dy++) {
        for(var dx=0;dx<w;dx++) {
            var x = x0+dx;
            var y = y0+dy;
            if(x<0||y<0||x>=this.width||y>=this.height)continue;            
            var out_index = ( x + y * this.width ) * 4;
            var in_index = ( dx + dy * w ) * 4;
            this.data[out_index] = data_u8a[in_index]; // r
            this.data[out_index+1] = data_u8a[in_index+1]; // g
            this.data[out_index+2] = data_u8a[in_index+2]; // b
            this.data[out_index+3] = data_u8a[in_index+3]; // a            
        }
    }      
}


///////////////////
TileDeck.prototype.id_gen = 1;
function TileDeck() {
    this.id = this.__proto__.id_gen++;
}
TileDeck.prototype.setSize = function(sprw,sprh,cellw,cellh) {
    this.tile_width = sprw;
    this.tile_height = sprh;
    this.cell_width = cellw;
    this.cell_height = cellh;
}
TileDeck.prototype.setTexture = function(tex) {
    this.moyai_tex = tex;
}
TileDeck.prototype.getUVFromIndex = function(outary, ind,uofs,vofs,eps) {
	var uunit = this.cell_width / this.moyai_tex.image.width;
	var vunit = this.cell_height / this.moyai_tex.image.height;
	var start_x = this.cell_width * Math.floor( Math.floor(ind) % Math.floor(this.tile_width) );
	var start_y = this.cell_height * Math.floor( Math.floor(ind) / Math.floor(this.tile_width ) );
    var u0 = start_x / this.moyai_tex.image.width + eps + uofs * uunit;
    var v0 = start_y / this.moyai_tex.image.height + eps + vofs * vunit;
    var u1 = u0 + uunit - eps*2;  // *2 because adding eps once for u0 and v0
	var v1 = v0 + vunit - eps*2;
    outary[0]=u0;
    outary[1]=v0;
    outary[2]=u1;
    outary[3]=v1;
}
TileDeck.prototype.getUVOfPixel = function(outary,ind,x_in_cell,y_in_cell) {
    ind=Math.floor(ind);
    var x0=Math.floor(ind%this.tile_width)*this.cell_width;
    var y0=Math.floor(ind/this.tile_width)*this.cell_height;
    var fin_x=x0+x_in_cell, fin_y=y0+y_in_cell;
    var u_per_pixel = 1.0/this.moyai_tex.image.width;
    var v_per_pixel = 1.0/this.moyai_tex.image.height;
    outary[0]=fin_x*u_per_pixel;
    outary[1]=fin_y*v_per_pixel;
    outary[2]=(fin_x+1)*u_per_pixel;
    outary[3]=(fin_y+1)*v_per_pixel;
}
TileDeck.prototype.getUperCell = function() { return this.cell_width / this.moyai_tex.image.width; }
TileDeck.prototype.getVperCell = function() { return this.cell_height / this.moyai_tex.image.height; }    

TileDeck.prototype.getPixelsFromIndex = function(ind) {
	var start_x = this.cell_width * Math.floor( Math.floor(ind) % Math.floor(this.tile_width) );
	var start_y = this.cell_height * Math.floor( Math.floor(ind) / Math.floor(this.tile_width ) );
    var out=[];
    for(var y=start_y;y<start_y+this.cell_height;y++) {
        for(var x=start_x;x<start_x+this.cell_width;x++) {
            var di=x*4+y*(this.cell_width*this.tile_width)*4;
            for(var i=0;i<4;i++) {
                out.push(this.moyai_tex.image.data[di+i]);
            }
        }
    }
    return out;
}



////////////////////////

try {
    if(global) {
        // classes
        global.Color=Color;
        global.Viewport=Viewport;       
        global.Image=MoyaiImage;
        global.TileDeck = TileDeck;
        global.Layer = Layer;

        // funcs
        global.to_i=Math.floor;
        global.lengthf=lengthf;
        global.now=now;
        global.choose=choose;
        global.sign=sign;
        global.range=range;
        global.irange=irange;
        global.dirToDXDY=dirToDXDY;
        global.clockDir=clockDir;
        global.dxdyToDir=dxdyToDir;
        global.leftDir=leftDir;
        global.rightDir=rightDir;
        global.reverseDir=reverseDir;
        global.randomDir=randomDir;
        global.randomLRDir=randomLRDir;
        global.turnDir=turnDir;        
        // constants
        global.DIR4_NONE=DIR4_NONE;
        global.DIR4_UP=DIR4_UP;
        global.DIR4_RIGHT=DIR4_RIGHT;
        global.DIR4_DOWN=DIR4_DOWN;
        global.DIR4_LEFT=DIR4_LEFT;
    }
} catch(e) {}