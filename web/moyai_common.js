
var DIR4_NONE=-1;
var DIR4_UP=0;
var DIR4_RIGHT=1;
var DIR4_DOWN=2;
var DIR4_LEFT=3;


function randomDir() {
    return irange(DIR4_UP,DIR4_LEFT+1);
}

function reverseDir(d){
    switch(d){
    case DIR4_UP: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_UP;
    case DIR4_RIGHT: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_RIGHT;
    default:
        return DIR4_NONE;
    }
}

function rightDir(d) {
    switch(d){
    case DIR4_UP: return DIR4_RIGHT; 
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_LEFT: return DIR4_UP;
    default:
        return DIR4_NONE;
    }
}
function leftDir(d) {
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
function dxdyToDir(dx,dy){
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
function clockDir(d) {
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

function dirToDXDY(d) {
    switch(d){
    case DIR4_NONE: return null;
    case DIR4_RIGHT: return {x:1,y:0};
    case DIR4_LEFT: return {x:-1,y:0};
    case DIR4_UP: return {x:0,y:1};
    case DIR4_DOWN: return {x:0,y:-1};
    default:
        console.assert("dirToDXDY: invalid direction:",d);
    }
}




////

function irange(a,b) {
    return parseInt(range(a,b));
}
function range(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    return (small + (big-small)*Math.random());
}
function sign(f) {
    if(f>0) return 1; else if(f<0)return -1; else return 0;
}
function now() {
    var t = new Date().getTime();
    return t / 1000.0;
}
function lengthf(x0,y0,x1,y1) {
    return Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
}
