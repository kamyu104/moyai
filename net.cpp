// Moyai network : moynet

#if defined(WIN32)
#include <winsock2.h>
typedef int ssize_t;
#define MSG_DONTWAIT 0 // no support in windows send func
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#endif




#include "ConvertUTF.h"

#include "cumino.h"
#include "common.h"
#include "net.h"



//////////////



/////////////

Buffer::Buffer() : buf(0), size(0), used(0) {
}
void Buffer::ensureMemory( size_t sz ) {
    buf = (char*) MALLOC(sz);
    assert(buf);
    size = sz;
    used = 0;    
}

Buffer::~Buffer() {
    assert(buf);
    free(buf);
    size = used = 0;
}


//int Conn::idgen = 1;

#if 0
Conn::Conn( Network *nw, uv_stream fd ) : stream(0), connecting(false), userptr(0), parent_listener(0), parent_nw(nw) {
    id = idgen++;
    fprintf(stderr, "Conn::Conn id:%d fd:%d",id,fd);

    sendbuf.ensureMemory( SENDBUF_SIZE );
    recvbuf.ensureMemory(  RECVBUF_SIZE );
    
    write_watcher = (struct ev_io*) MALLOC( sizeof(struct ev_io) );
    ev_io_init( write_watcher, write_callback, fd, EV_WRITE );
    ev_io_start( nw->evloop, write_watcher );
    write_watcher->data  = this;
    
    read_watcher = (struct ev_io*) MALLOC( sizeof(struct ev_io) );    
    ev_io_init( read_watcher, read_callback, fd, EV_READ );
    ev_io_start( nw->evloop, read_watcher );
    read_watcher->data = this;

}

Conn::~Conn() {
    print("~Conn called");
    if(parent_listener) parent_listener->delConn(this);
    ev_io_stop( parent_nw->evloop, read_watcher );
    ev_io_stop( parent_nw->evloop, write_watcher );
    free( read_watcher );
    free( write_watcher );
    ::close( fd );
}
void Conn::notifyError( NET_ERROR he, int eno ) {
    onError( he, eno);
    onClose();    
}
#endif

// ALL or NOTHING. never push part of the given data.
// return true if all data is pushed.
//bool Conn::push( const char *data, size_t datasz ) {
//    return sendbuf.push( data, datasz );
//}
//size_t Conn::getSendbufRoom() {
//    return sendbuf.getRoom();
//}


///////////////

#if 0
// returns negative if error
static void write_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    Conn *c = (Conn*) watcher->data;
    assert(c);
    //    fprintf(stderr, "write_callback. fd:%d connecting:%d\n", c->fd, c->connecting );
    if( c->connecting ) {
        c->onConnect();
        c->connecting = false;        
    }

    if( c->sendbuf.used == 0 ) {
        ev_io_stop( c->parent_nw->evloop, c->write_watcher );
        return;
    }
    
    ssize_t ss = send( watcher->fd, c->sendbuf.buf, c->sendbuf.used, MSG_DONTWAIT );
    if( c->parent_nw->syscall_log ) fprintf(stderr, "send( %d, %p, %d, MSG_DONTWAIT ) => %d\n", watcher->fd, c->sendbuf.buf, (int)c->sendbuf.used, (int)ss );
    if(ss==-1) {
        c->notifyError( NET_ERROR_WRITE, errno );
        delete c;
    } else {
        c->parent_nw->total_sent_bytes += ss;
        c->sendbuf.shift( ss );
        if( c->sendbuf.used == 0 ) {
            ev_io_stop( c->parent_nw->evloop, c->write_watcher );
            //            fprintf(stderr, "stop write watcher on fd:%d\n", c->fd );
        }
    }
}
#endif

static bool is_would_block_error() {
#if WIN32
    return( GetLastError() == WSAEWOULDBLOCK );
#else
    return( errno == EWOULDBLOCK || errno == EINPROGRESS );
#endif
}
static bool is_any_error() {
#if WIN32    
    return( GetLastError() != 0 );
#else
    return( errno != 0 );
#endif    
}
static int get_error_number() {
#if WIN32    
    return GetLastError();
#else
    return errno;
#endif    

}

void moynet_dump( const char *s, size_t l ) {
    for(int i=0;i<l;i++) {
        fprintf(stderr, "%02x ", s[i] & 0xff );
        if( (i%16) == 15 ) fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
}

#if 0
static void read_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    Conn *c = (Conn*) watcher->data;
    assert(c);
    //    fprintf(stderr, "read_callback. fd:%d\n", c->fd );
    size_t space = c->recvbuf.getRoom();
    if( space == 0 ) return;

    ssize_t rsz = recv( watcher->fd, c->recvbuf.buf + c->recvbuf.used, space, 0 );
    if( c->parent_nw->syscall_log ) fprintf(stderr, "recv( %d, %p, %d ) => %d\n", watcher->fd, c->recvbuf.buf+ c->recvbuf.used, (int)space, (int)rsz );
    if( rsz < 0 ) {
        if( is_would_block_error() ) {
            // again later!
        } else if( is_any_error() ) {
            c->notifyError( NET_ERROR_READ, get_error_number() );
            delete c;
        }
    } else if( rsz == 0 ) {
        // EOF! closed by client.
        if( c->parent_nw->syscall_log ) fprintf(stderr, "recv(fd:%d) returned 0.\n", watcher->fd );
        c->onClose();
        delete c;
    } else {
        c->parent_nw->total_recv_bytes += rsz;
        c->recvbuf.used += rsz;
        // Parse RPC
        //        fprintf(stderr, "recvbuf used:%zu\n", c->recvbuf.used );
        //        moynet_t *h = c->parent_moynet;
        while(true) { // process everything in one poll
            //            print("recvbuf:%d", c->recvbuf.used );
            if( c->recvbuf.used < (4+2) ) return; // need more data from network
            //              <---RECORDLEN------>
            // [RECORDLEN32][FUNCID32][..DATA..]            
            size_t record_len = get_u32( c->recvbuf.buf );
            unsigned int func_id = get_u16( c->recvbuf.buf + 4 );

            if( c->recvbuf.used < (4+record_len) ) {
                //   print("need. used:%d reclen:%d", c->recvbuf.used, record_len );
                prt("_p ");
                return; // need more data from network
            }
            if( record_len < 2 ) {
                fprintf(stderr, "invalid packet format" );
                c->notifyError( NET_ERROR_FORMAT, 0 );
                delete c;
                return;
            }
            //             fprintf(stderr, "dispatching func_id:%d record_len:%lu\n", func_id, record_len );
            // dump( c->recvbuf.buf, record_len-4);
            c->onPacket( func_id, (char*) c->recvbuf.buf +4+2, record_len - 2 );
            c->recvbuf.shift( 4 + record_len );
            //            fprintf(stderr, "after dispatch recv func: buffer used: %zu\n", c->recvbuf.used );
            //            if( c->recvbuf.used > 0 ) dump( c->recvbuf.buf, c->recvbuf.used );
        }
    }
    
}

static void accept_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    fprintf(stderr,"accept callback\n");
    Listener *l = (Listener*) watcher->data;
    assert(l);
    Network *nw = l->parent_nw;
    assert(nw);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int new_fd = accept( l->fd, (struct sockaddr*) &addr, &addrlen );
    if( nw->syscall_log ) fprintf( stderr, "accept( %d, %p, %d ) => %d\n", l->fd, &addr, addrlen, new_fd );

    if( new_fd != -1 ) {
#ifndef WIN32
        int flag = fcntl( new_fd, F_GETFL );
        if( flag < 0 ){
            fprintf(stderr, "socket getfl error\n" );
            close(new_fd);
            return;
        }
        if( fcntl( new_fd, F_SETFL, flag|O_NONBLOCK)<0){
            fprintf(stderr, "socket nonblock setfl error\n" ); 
            close(new_fd);
            return;
        }
#endif
        //
        l->onAccept(new_fd);
    } else {
        // error
    }
}
#endif

#if 0
// returns true if success
bool Listener::startListen( const char *addr, int tcpport ) {
    if(fd!=-1) {
        print("Listener::startListen: already listening");
        return false;
    }
    struct addrinfo hints, *res;
    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;
    hints.ai_flags = AI_PASSIVE;
    char pstr[32];
    snprintf(pstr,sizeof(pstr),"%d", tcpport );
    getaddrinfo(addr, pstr, &hints, &res );
    fd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );

    int opt = 1;
    if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == -1 ){
        fprintf( stderr, "setsockopt(reuseaddr) error:%s", strerror(errno)  );
        close(fd); // setsockopt error
        return false;
    }

    struct linger lingeropt;
    lingeropt.l_onoff =  1;
    lingeropt.l_linger = 2;

    if( setsockopt( fd, SOL_SOCKET, SO_LINGER,  (const char*)&lingeropt, sizeof(lingeropt)) == -1 ){
        fprintf( stderr, "setsockopt(reuseaddr) error:%s", strerror(errno) );
        close(fd);
        return false;
    }
    
    if( bind( fd, res->ai_addr, res->ai_addrlen ) == -1 ){
        fprintf( stderr, "bind error");
        close(fd); 
        return false;
    }
    if( listen( fd, SOMAXCONN ) == -1 ){
        fprintf( stderr, "listen error");
        close(fd); 
        return false;
    }

#ifdef WIN32
    DWORD  dwNonBlocking = 1;
    ioctlsocket(fd, FIONBIO, &dwNonBlocking);
#endif

    freeaddrinfo(res);

    accept_watcher = (struct ev_io*) NET_MALLOC( sizeof(struct ev_io));
    memset( (void*) accept_watcher, 0, sizeof(struct ev_io));
    accept_watcher->data = this;    
    ev_io_init( accept_watcher, accept_callback, fd, EV_READ );
    ev_io_start( parent_nw->evloop, accept_watcher );
    return true;
}
#endif

#if 0
void Listener::addConn( Conn *c ) {
    Conn *stored = conn_pool.get(c->id);
    if(!stored) {
        c->parent_listener = this;
        conn_pool.set(c->id,c);
    }
}
void Listener::delConn( Conn *c ) {
    conn_pool.del(c->id);
}
void Listener::broadcastUS1Bytes( uint16_t usval, const char *data, size_t datalen ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1Bytes( usval, data, datalen );
    }
}
void Listener::broadcastUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *data, size_t datalen ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI1Bytes( usval, uival, data, datalen );
    }
}
void Listener::broadcastUS1UI1( uint16_t usval, uint32_t uival ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI1( usval, uival );
    }
}
void Listener::broadcastUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1 ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI2( usval, ui0, ui1 );
    }
}
void Listener::broadcastUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI3( usval, ui0, ui1, ui2 );
    }
}
void Listener::broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI1Wstr( usval, uival, wstr, wstr_num_letters );
    }
}
void Listener::broadcastUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI1F2( usval, uival, f0, f1 );
    }
}
void Listener::broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 ) {
    for( ConnIteratorType it = conn_pool.idmap.begin(); it != conn_pool.idmap.end(); ++it ) {
        Conn *c = it->second;
        c->sendUS1UI1F1( usval, uival, f0 );
    }
}
#endif

//////////////////////
Network::Network() : syscall_log(false), total_sent_bytes(0), total_recv_bytes(0), accum_time(0), last_stats_at(0) {
}

// returns valid fd when success or -1
int Network::connectToServer( const char *host, int portnum ) {
    struct addrinfo hints, *res;
    memset( &hints, 0, sizeof(hints) );
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;
    char pstr[32];
    snprintf( pstr,sizeof(pstr),"%d", portnum );
    getaddrinfo( host, pstr, &hints, &res );
    if(!res){
        fprintf(stderr, "invalid host address or port? '%s':%d", host, portnum );
        return -1;
    }
    int new_fd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
    if( new_fd == -1 ) {
        fprintf(stderr, "socket() error. errno:%d\n",errno );
        return -1;
    }

#ifndef WIN32
    int flag = fcntl( new_fd, F_GETFL );
    if( flag < 0 ){
        fprintf(stderr, "socket getfl error. errno:%s\n", strerror(errno) );
        close(new_fd);
        return -1;
    }
    if( fcntl( new_fd, F_SETFL, flag|O_NONBLOCK)<0){
        fprintf(stderr, "socket nonblock setfl error. errno:%s\n", strerror(errno) );
        close(new_fd);
        return -1;
    }
#endif
    

    if( ::connect( new_fd, res->ai_addr, res->ai_addrlen ) == -1 ) {
        if( is_would_block_error() == false ) {
            fprintf(stderr, "moynet_connect: connect() failed: errno:%d\n", errno );
            close( new_fd );
            return -1;
        }
    }
#ifdef WIN32
    DWORD  dwNonBlocking = 1;
    ioctlsocket(new_fd, FIONBIO, &dwNonBlocking);
#endif        

    freeaddrinfo(res);

    return new_fd;
}



void Network::heartbeat() {
    double nt = now();
    
    uv_run( uv_default_loop(), UV_RUN_ONCE );

    if( nt > last_stats_at + 1.0f ) {
        last_stats_at = nt;
        ts.recv_bytes_per_sec = total_recv_bytes - ts.total_recv_bytes;
        ts.sent_bytes_per_sec = total_sent_bytes - ts.total_sent_bytes;
        ts.total_recv_bytes = total_recv_bytes;
        ts.total_sent_bytes = total_sent_bytes;
    }
}
void Network::getTrafficStats( TrafficStats *outstats ) {
    memcpy( outstats, &ts, sizeof(ts) );
}

void Network::heartbeatWithTimeoutMicroseconds( int timeout_us ) {
#if defined(WIN32)
    assertmsg( false, "not implemented in windows" );
#else    
    if( timeout_us == 0 ) {
        heartbeat();
    } else {
        double st = now();
        heartbeat();
        double et = now();
        double dt = et - st;
        double timeout = (double)(timeout_us) / 1000000.0f;
        if( dt < timeout ) {
            usleep( (timeout-dt) * 1000000 );
        }
    }
#endif    
}


/////
#if 0
int Conn::sendUS1( uint16_t usval ) {
    size_t totalsize = 4 + 2;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    return totalsize;    
}
int Conn::sendUS1Bytes( uint16_t usval, const char *buf, uint16_t buflen ) {
    size_t totalsize = 4 + 2 + (4+buflen);
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( buflen );
    sendbuf.push( buf, buflen );
    return totalsize;
}
int Conn::sendUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *buf, uint32_t buflen ) {
    size_t totalsize = 4 + 2 + 4 + (4+buflen);
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( uival );
    sendbuf.pushU32( buflen );
    sendbuf.push( buf, buflen );
    return totalsize;
}
int Conn::sendUS1UI1( uint16_t usval, uint32_t uival ) {
    size_t totalsize = 4 + 2 + 4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( uival );
    return totalsize;
}
int Conn::sendUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1 ) {
    size_t totalsize = 4 + 2 + 4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( ui0 );
    sendbuf.pushU32( ui1 );    
    return totalsize;
}
int Conn::sendUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 ) {
    size_t totalsize = 4 + 2 + 4+4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( ui0 );
    sendbuf.pushU32( ui1 );
    sendbuf.pushU32( ui2 );        
    return totalsize;
}
int Conn::sendUS1UI5( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3, uint32_t ui4 ) {
    size_t totalsize = 4 + 2 + 4+4+4+4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( ui0 );
    sendbuf.pushU32( ui1 );
    sendbuf.pushU32( ui2 );
    sendbuf.pushU32( ui3 );
    sendbuf.pushU32( ui4 );        
    return totalsize;
}
int Conn::sendUS1UI1F1( uint16_t usval, uint32_t uival, float f0 ) {
    size_t totalsize = 4 + 2 + 4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( uival );
    sendbuf.push( (char*)&f0, 4 );
    return totalsize;    
}
int Conn::sendUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 ) {
    size_t totalsize = 4 + 2 + 4+4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( uival );
    sendbuf.push( (char*)&f0, 4 );
    sendbuf.push( (char*)&f1, 4 );
    return totalsize;    
}
int Conn::sendUS1F2( uint16_t usval, float f0, float f1 ) {
    size_t totalsize = 4 + 2 + 4+4;
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.push( (char*)&f0, 4 );
    sendbuf.push( (char*)&f1, 4 );
    return totalsize;
}
int Conn::sendUS1UI1Str( uint16_t usval, uint32_t uival, const char *cstr ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + 4 + (1+cstrlen);
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( uival );
    sendbuf.pushU8( (unsigned char) cstrlen );
    sendbuf.push( cstr, cstrlen );
    return totalsize;
}
int Conn::sendUS1UI2Str( uint16_t usval, uint32_t ui0, uint32_t ui1, const char *cstr ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + 4+4 + (1+cstrlen);
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU32( ui0 );
    sendbuf.pushU32( ui1 );    
    sendbuf.pushU8( (unsigned char) cstrlen );
    sendbuf.push( cstr, cstrlen );
    return totalsize;
}
// [record-len:16][usval:16][cstr-len:8][cstr-body][data-len:32][data-body]
int Conn::sendUS1StrBytes( uint16_t usval, const char *cstr, const char *data, uint32_t datalen ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + (1+cstrlen) + (4+datalen);
    if( getSendbufRoom() < totalsize ) return 0;
    sendbuf.pushU32( totalsize - 4 ); // record-len
    sendbuf.pushU16( usval );
    sendbuf.pushU8( (unsigned char) cstrlen );
    sendbuf.push( cstr, cstrlen );
    sendbuf.pushU32( datalen );
    sendbuf.push( data, datalen );
    //    print("send_packet_str_bytes: cstrlen:%d datalen:%d totallen:%d", cstrlen, datalen, totalsize );
    return totalsize;
}
void Conn::parsePacketStrBytes( char *inptr, char *outcstr, char **outptr, size_t *outsize ) {
    uint8_t slen = get_u8(inptr);
    char *s = inptr + 1;
    uint32_t datalen = get_u32(inptr+1+slen);
    *outptr = inptr + 1 + slen + 4;
    memcpy( outcstr, s, slen );
    outcstr[slen]='\0';
    *outsize = (size_t) datalen;
}

// convert wchar_t to 
int Conn::sendUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters ) {
#if defined(__APPLE__) || defined(__linux__)
    assert( sizeof(wchar_t) == sizeof(int32_t) );
    size_t bufsz = wstr_num_letters * sizeof(int32_t);
    UTF8 *outbuf = (UTF8*) MALLOC( bufsz + 1);
    assert(outbuf);
    UTF8 *orig_outbuf = outbuf;
    const UTF32 *inbuf = (UTF32*) wstr;
    ConversionResult r = ConvertUTF32toUTF8( &inbuf, inbuf+wstr_num_letters, &outbuf, outbuf+bufsz, strictConversion );
    assertmsg(r==conversionOK, "ConvertUTF32toUTF8 failed:%d bufsz:%d", r, bufsz );
    size_t outlen = outbuf - orig_outbuf;
    //    print("ConvertUTF32toUTF8 result utf8 len:%d out:'%s'", outlen, orig_outbuf );
    int ret = sendUS1UI1Bytes( usval, uival, (const char*) orig_outbuf, outlen );
    free(orig_outbuf);
    return ret;    
#else
    assertmsg( false, "not implemented" );
    return 0;
#endif    
}
#endif