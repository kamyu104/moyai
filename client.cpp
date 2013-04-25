
#include "cumino.h"
#include "client.h"






bool Prop2D::propPoll(double dt) {
    if( prop2DPoll(dt) == false ) return false;
    
    // animation of index
    if(anim_curve){
        int previndex = index;
        index = anim_curve->getIndex( accum_time - anim_start_at );
        if( index != previndex ){
            onIndexChanged(previndex);
        }
    }
    // animation of scale
    if( seek_scl_time != 0 ){
        double elt = accum_time - seek_scl_started_at;
        if( elt > seek_scl_time ){
            scl = seek_scl_target;
            seek_scl_time = 0;
        } else {
            double rate = elt / seek_scl_time;
            scl.x = seek_scl_orig.x + ( seek_scl_target.x - seek_scl_orig.x ) * rate;
            scl.y = seek_scl_orig.y + ( seek_scl_target.y - seek_scl_orig.y ) * rate;
        }
    }
    // animation of rotation
    if( seek_rot_time != 0 ){
        double elt = accum_time - seek_rot_started_at;
        if( elt > seek_rot_time ){
            rot = seek_rot_target;
            seek_rot_time = 0;
        } else {
            double rate = elt / seek_rot_time;
            rot = seek_rot_orig + ( seek_rot_target - seek_rot_orig ) * rate;
        }
    }
    // animation of color
    if( seek_color_time != 0 ){
        double elt = accum_time - seek_color_started_at;
        if( elt > seek_color_time ){
            color = seek_color_target;
            seek_color_time = 0;
        } else {
            double rate = elt / seek_color_time;
            color = Color( seek_color_orig.r + ( seek_color_target.r - seek_color_orig.r ) * rate,
                           seek_color_orig.g + ( seek_color_target.g - seek_color_orig.g ) * rate,
                           seek_color_orig.b + ( seek_color_target.b - seek_color_orig.b ) * rate,
                           seek_color_orig.a + ( seek_color_target.a - seek_color_orig.a ) * rate );
        }
    }

    // children
    for(int i=0;i<children_num;i++){
        Prop2D *p = children[i];
        p->loc = loc;
        p->basePoll(dt);
    }
    
    return true;
}

int MoyaiClient::renderAll(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    int cnt=0;
    for(int i=0;i<elementof(groups);i++){
        Layer *l = (Layer*) groups[i];
        if(l) cnt += l->renderAllProps();
    }

    glfwSwapBuffers();
    glFlush();
    return cnt;
}
inline void Layer::drawBillboard(int billboard_index, TileDeck *deck, Vec3 *loc, Vec3 *scl  ) {
    if(billboard_index <0)return;    
    assert(deck);
    glEnable(GL_TEXTURE_2D);
    if( deck->tex->tex != last_tex_gl_id ) {
        glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
        last_tex_gl_id = deck->tex->tex;
    }

    glLoadIdentity();    
    Vec3 diff_camera = *loc - camera->loc;
    Vec3 up_v(0.0f, 1.0f, 0.0f);

    Vec3 cross_a = diff_camera.cross(up_v).normalize(1); 
    Vec3 cross_b = diff_camera.cross(cross_a).normalize(1); 

    // now you can use CrossA and CrossB and the billboard position to calculate the positions of the edges of the billboard-rectangle
    cross_a.x *= scl->x * 0.5;        
    cross_a.y *= scl->y * 0.5;
    cross_a.z *= scl->z * 0.5;
    cross_b.x *= scl->x * 0.5;        
    cross_b.y *= scl->y * 0.5;
    cross_b.z *= scl->z * 0.5;

    Vec3 p1 = *loc + cross_a + cross_b;
    Vec3 p2 = *loc - cross_a + cross_b;
    Vec3 p3 = *loc - cross_a - cross_b;
    Vec3 p4 = *loc + cross_a - cross_b;

#if 0
        print("loc:%f %f %f a:%.1f %.1f %.1f b:%.1f %.1f %.1f  p1:%.1f %.1f %.1f p2:%.1f %.1f %.1f p3:%.1f %.1f %.1f p4:%.1f %.1f %.1f",
              loc->x, loc->y, loc->z,
              cross_a.x, cross_a.y, cross_a.z,
              cross_b.x, cross_b.y, cross_b.z,
              p1.x - loc->x, p1.y - loc->y, p1.z - loc->z,
              p2.x - loc->x, p2.y - loc->y, p2.z - loc->z,
              p3.x - loc->x, p3.y - loc->y, p3.z - loc->z,
              p4.x - loc->x, p4.y - loc->y, p4.z - loc->z                           
              );
#endif

        float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
        if( deck ) deck->getUVFromIndex( billboard_index, &u0, &v0, &u1, &v1, 0, 0 );

        glBegin(GL_TRIANGLES);
        glColor4f( 1,1,1,1 );
        glTexCoord2f(u1,v1); glVertex3f( p1.x, p1.y, p1.z );
        glTexCoord2f(u0,v0); glVertex3f( p3.x, p3.y, p3.z );
        glTexCoord2f(u0,v1); glVertex3f( p2.x, p2.y, p2.z );

        glTexCoord2f(u1,v1); glVertex3f( p1.x, p1.y, p1.z );        
        glTexCoord2f(u1,v0); glVertex3f( p4.x, p4.y, p4.z );
        glTexCoord2f(u0,v0); glVertex3f( p3.x, p3.y, p3.z );        
        glEnd();

}

inline void Layer::drawMesh( int dbg, Mesh *mesh, TileDeck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  ) {   
    if( deck ) {
        glEnable(GL_TEXTURE_2D);
        if( deck->tex->tex != last_tex_gl_id ) {
            glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
            last_tex_gl_id = deck->tex->tex;
        }
    } else {
        glDisable(GL_TEXTURE_2D);            
    }

    mesh->vb->bless();
    assert( mesh->vb->gl_name > 0 );
    mesh->ib->bless();
    assert( mesh->ib->gl_name > 0 );
    int vert_sz = mesh->vb->fmt->getNumFloat() * sizeof(float);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->ib->gl_name );
    glBindBuffer( GL_ARRAY_BUFFER, mesh->vb->gl_name );
    if( dbg != 0 ) {
        print("draw mesh! dbg:%d deck:%p mesh:%p vbn:%d ibn:%d coordofs:%d colofs:%d texofs:%d normofs:%d vert_sz:%d array_len:%d loc:%f %f %f",
              dbg,
              deck,
              mesh,
              mesh->vb->gl_name,
              mesh->ib->gl_name,
              mesh->vb->fmt->coord_offset,
              mesh->vb->fmt->color_offset,
              mesh->vb->fmt->texture_offset,
              mesh->vb->fmt->normal_offset,
              vert_sz,
              mesh->vb->array_len,
              loc->x, loc->y, loc->z
              );
    }



    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
        
    if( mesh->vb->fmt->coord_offset >= 0 ){
        glEnableClientState( GL_VERTEX_ARRAY );        
        glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->coord_offset * sizeof(float) );
    }
    if( mesh->vb->fmt->color_offset >= 0 ){
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->color_offset * sizeof(float));
    }
    if( mesh->vb->fmt->texture_offset >= 0 ){
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );                    
        glTexCoordPointer( 2, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->texture_offset * sizeof(float) );
    }
    if( mesh->vb->fmt->normal_offset >= 0 ) {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->normal_offset * sizeof(float) );
    }

    glLoadIdentity();    

    // ライトが設定されてるメッシュの中でも、マテリアルが設定されてないメッシュが混ざってるときはライトつけないことが必要。
    // TODO: すでにライトあててるときは再設定必要ない
    if( light && material ) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        float ambient[4] = { light->ambient.r, light->ambient.g, light->ambient.b, light->ambient.a };
        glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
        float diffuse[4] = { light->diffuse.r, light->diffuse.g, light->diffuse.b, light->diffuse.a };
        glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
        float pos[4] = { light->pos.x, light->pos.y, light->pos.z, 0 };
        glLightfv( GL_LIGHT0, GL_POSITION, pos );
        float specular[4] = { light->specular.r, light->specular.g, light->specular.b, light->specular.a };        
        glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    } else {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }

                    
    glTranslatef( loc->x + locofs->x, loc->y + locofs->y, loc->z + locofs->z );
    if( rot->x != 0 ) glRotatef( rot->x, 1,0,0);     
    if( rot->y != 0 ) glRotatef( rot->y, 0,1,0);     
    if( rot->z != 0 ) glRotatef( rot->z, 0,0,1);
        
    glScalef( scl->x, scl->y, scl->z );


    if( localloc ) {
        glTranslatef( localloc->x, localloc->y, localloc->z );
        if( localrot->x != 0 ) glRotatef( localrot->x, 1,0,0);     
        if( localrot->y != 0 ) glRotatef( localrot->y, 0,1,0);     
        if( localrot->z != 0 ) glRotatef( localrot->z, 0,0,1);
        glScalef( localscl->x, localscl->y, localscl->z );
    }        

    if(material) {
        float diffuse[4] = { material->diffuse.r, material->diffuse.g, material->diffuse.b, material->diffuse.a };
        glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
        float ambient[4] = { material->ambient.r, material->ambient.g, material->ambient.b, material->ambient.a };
        glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
        float specular[4] = { material->specular.r, material->specular.g, material->specular.b, material->specular.a };
        glMaterialfv( GL_FRONT, GL_SPECULAR, specular);
    }
    if( mesh->prim_type == GL_LINES || mesh->prim_type == GL_LINE_STRIP ) {
        glLineWidth(mesh->line_width);
    }
    glDrawElements( mesh->prim_type, mesh->ib->array_len, GL_UNSIGNED_INT, 0);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

}

inline void Prop3D::performRenderOptions() {
    glDepthMask( depth_mask );
    if( alpha_test ) {
        glEnable( GL_ALPHA_TEST );
        glAlphaFunc( GL_GREATER, 0.01 );
    } else {
        glDisable( GL_ALPHA_TEST );
    }
    if( cull_back_face ) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);                    
    } else  {
        glDisable(GL_CULL_FACE);
    }
    if( fragment_shader ){
        glUseProgram( fragment_shader->program );
        fragment_shader->updateUniforms();
    }
}
inline void Prop3D::cleanRenderOptions() {
    if( fragment_shader ) glUseProgram( 0 );
    glDepthMask(true);
}

int Layer::renderAllProps(){
    if( !to_render ) return 0;
    assertmsg( viewport, "no viewport in a layer id:%d setViewport missed?", id );
    if( viewport->dimension == DIMENSION_2D ) {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_DEPTH_TEST);

    
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho( -viewport->scl.x/2, viewport->scl.x/2, -viewport->scl.y/2, viewport->scl.y/2,-100,100);  // center is always (0,0)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();        

        static SorterEntry tosort[1024*32];
        
        int cnt = 0;
        int drawn = 0;
        Prop *cur = prop_top;

        Vec2 minv, maxv;
        viewport->getMinMax(&minv, &maxv);
        while(cur){
            Prop2D *cur2d = (Prop2D*)cur;
            
            assert( cur2d->dimension == viewport->dimension );

            // culling
            float camx=0,camy=0;
            if(camera){
                camx = camera->loc.x;
                camy = camera->loc.y;
            }

            float scr_maxx = cur2d->loc.x - camx + cur2d->scl.x/2 + cur2d->max_rt_cache.x;
            float scr_minx = cur2d->loc.x - camx - cur2d->scl.x/2 + cur2d->min_lb_cache.x;
            float scr_maxy = cur2d->loc.y - camy + cur2d->scl.y/2 + cur2d->max_rt_cache.y;
            float scr_miny = cur2d->loc.y - camy - cur2d->scl.y/2 + cur2d->min_lb_cache.y;
            
            if( scr_maxx >= minv.x && scr_minx <= maxv.x && scr_maxy >= minv.y && scr_miny <= maxv.y ){
                tosort[cnt].val = cur2d->priority;
                tosort[cnt].ptr = cur2d;
                cnt++;
                if(cnt>= elementof(tosort)){
                    print("WARNING: too many props in a layer" );
                    break;
                }
                drawn ++;
            } 
            cur = cur->next;
        }
    
        quickSortF( tosort, 0, cnt-1 );
        for(int i=cnt-1;i>=0;i--){
            Prop2D *p = (Prop2D*) tosort[i].ptr;
            if(p->visible){
                //                { Prop2D *p2d = (Prop2D*)p; print("prio:%f %d %d", p2d->loc.y, p2d->priority, p2d->id  ); }
                p->render(camera);
            }
        }
        return drawn;
    } else { // 3D
        assertmsg(camera, "3d render need camera.");

        glEnable(GL_DEPTH_TEST);

        setupProjectionMatrix3D();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // もともとライト設定はこの位置にあったが drawmeshに移動した

        int cnt=0;
        last_tex_gl_id = 0;

        Prop *cur = prop_top;
        while(cur){
            assert(cur->id>0);            
            Prop3D *cur3d = (Prop3D*)cur;            
            assert(cur3d->dimension == viewport->dimension );
        

        
            assertmsg( cur3d->mesh || cur3d->children_num > 0 || cur3d->billboard_index>=0, "mesh or children is required for 3d prop %p", cur3d );

            cnt++;

            if( cur3d->visible ) {
                cur3d->performRenderOptions();
                if( cur3d->billboard_index >= 0 ) {
                    drawBillboard( cur3d->billboard_index, cur3d->deck, & cur3d->loc, & cur3d->scl  );
                } else if( cur3d->mesh ) {
                    drawMesh( cur3d->debug_id, cur3d->mesh, cur3d->deck,
                              & cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
                              NULL, NULL, NULL, cur3d->material );
                }
                cur3d->cleanRenderOptions();


                if( cur3d->children_num > 0 ) {
                    int opaque_n=0;
                    int transparent_n=0;
                    for(int i=0;i<cur3d->children_num;i++) {
                        cnt++;
                        Prop3D *child = cur3d->children[i];
                        if(child && child->visible ) {
                            float l = camera->loc.len( cur3d->loc + child->loc + child->sort_center );
                            if( child->mesh->transparent ) {
                                sorter_transparent[transparent_n].ptr = (void*)cur3d->children[i];
                                sorter_transparent[transparent_n].val = l;
                                transparent_n++;
                            } else {
                                sorter_opaque[opaque_n].ptr = (void*)cur3d->children[i];
                                sorter_opaque[opaque_n].val = l;
                                opaque_n++;
                            }
                        }
                    }
                    if( opaque_n > 0 ) quickSortF( sorter_opaque, 0, opaque_n-1 );
                    if( transparent_n > 0 ) quickSortF( sorter_transparent, 0, transparent_n-1 );

                    // draw opaque mesh first
                    for(int i=opaque_n-1;i>=0;i--) {
                        Prop3D *child = (Prop3D*)sorter_opaque[i].ptr;
                        child->performRenderOptions();                        
                        if( child->skip_rot ) {
                            Vec3 fixedrot(0,0,0);
                            drawMesh( child->debug_id, child->mesh, child->deck,
                                      & cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & fixedrot,
                                      & child->loc, & child->scl, & child->rot,
                                      child->material
                                      );
                            
                        } else { 
                            drawMesh( child->debug_id, child->mesh, child->deck,
                                      & cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
                                      & child->loc, & child->scl, & child->rot,
                                      child->material
                                      );
                        }
                        child->cleanRenderOptions();                        
                    }
                    for(int i=transparent_n-1;i>=0;i--){
                        Prop3D *child = (Prop3D*)sorter_transparent[i].ptr;
                        child->performRenderOptions();                                                
                        if( child->skip_rot ) {
                            Vec3 fixedrot(0,0,0);
                            drawMesh( child->debug_id, child->mesh, child->deck,
                                      & cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & fixedrot,
                                      & child->loc, & child->scl, & child->rot,
                                      child->material
                                      );
                            
                        } else {
                            drawMesh( child->debug_id, child->mesh, child->deck,
                                      & cur3d->loc, &cur3d->draw_offset, & cur3d->scl, & cur3d->rot,
                                      & child->loc, & child->scl, & child->rot,
                                      child->material
                                      );
                        }
                        child->cleanRenderOptions();
                    }
                }
            }                                   
            
            cur = cur->next;
        }
        return cnt;        
    }
}

void Prop3D::setMaterialChildren( Material *mat ) {        
    for(int i=0;i<children_num;i++) {
        if( children[i] ) {
            children[i]->setMaterial(mat);
        }
    }
}    

Vec2 Prop3D::getScreenPos() {
    Layer *l = (Layer*) parent_group;
    return l->getScreenPos(loc);
}

void Layer::setupProjectionMatrix3D() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 60, (GLdouble)viewport->screen_width/(GLdouble)viewport->screen_height, viewport->near_clip, viewport->far_clip );
    gluLookAt( camera->loc.x,camera->loc.y,camera->loc.z,
               camera->look_at.x,camera->look_at.y,camera->look_at.z,
               camera->look_up.x,camera->look_up.y,camera->look_up.z );
}

Vec2 Layer::getScreenPos( Vec3 at ) {
    setupProjectionMatrix3D();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    double projection[16], modelview[16];
    double sx,sy,sz;
    int vp[4];
        
    glGetIntegerv( GL_VIEWPORT, vp );    
    glGetDoublev(GL_PROJECTION_MATRIX, projection );
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview );

    gluProject( at.x, at.y, at.z, modelview, projection, vp, &sx, &sy, &sz );
    return Vec2( sx,sy );
}
Vec3 Layer::getWorldPos( Vec2 scrpos ) {
    setupProjectionMatrix3D();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double projection[16], modelview[16];
    int vp[4];
    glGetIntegerv( GL_VIEWPORT, vp );
    glGetDoublev(GL_PROJECTION_MATRIX, projection );
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview );
    
    float z;
    double ox,oy,oz;
    Vec3 out;
    glReadPixels( scrpos.x, scrpos.y, 1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
    gluUnProject( scrpos.x, scrpos.y, z, modelview, projection, vp, &ox, &oy, &oz );
    return Vec3(ox,oy,oz);
}


void Prop2D::drawIndex( TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot ) {

    float u0,v0,u1,v1;
    dk->getUVFromIndex(ind, &u0, &v0, &u1, &v1, uofs, vofs );
    float depth = 10;

    if(debug_id==123) print("UV: ind:%d %f,%f %f,%f uo:%f vo:%f", ind, u0,v0, u1,v1, uofs, vofs );

    if(hrev){
        float tmp = u1;
        u1 = u0;
        u0 = tmp;
    }
    if(vrev){
        float tmp = v1;
        v1 = v0;
        v0 = tmp;
    }

    // if not rot 
    // B-----C       A:min C:max
    // |     |
    // A-----D
    //
    // if rot
    Vec2 a,b,c,d;
    if(rot==0){
        if(uvrot){
            a = Vec2( maxx, miny );
            b = Vec2( minx, miny );
            c = Vec2( minx, maxy );
            d = Vec2( maxx, maxy );
        } else {
            a = Vec2( minx, miny );
            b = Vec2( minx, maxy );
            c = Vec2( maxx, maxy );
            d = Vec2( maxx, miny );
        }
    } else {
        float sn = sin(radrot);
        float cs = cos(radrot);
        float center_x = (minx+maxx)/2.0f;
        float center_y = (miny+maxy)/2.0f;
        minx -= center_x;
        miny -= center_y;
        maxx -= center_x;
        maxy -= center_y;
        
        if(uvrot){
            a = Vec2( maxx * cs - miny * sn, maxx * sn + miny * cs );
            b = Vec2( minx * cs - miny * sn, minx * sn + miny * cs );
            c = Vec2( minx * cs - maxy * sn, minx * sn + maxy * cs );
            d = Vec2( maxx * cs - maxy * sn, maxx * sn + maxy * cs );
        } else {
            a = Vec2( minx * cs - miny * sn, minx * sn + miny * cs );
            b = Vec2( minx * cs - maxy * sn, minx * sn + maxy * cs );
            c = Vec2( maxx * cs - maxy * sn, maxx * sn + maxy * cs );
            d = Vec2( maxx * cs - miny * sn, maxx * sn + miny * cs );
        }
        a = a.add( center_x, center_y );
        b = b.add( center_x, center_y );
        c = c.add( center_x, center_y );
        d = d.add( center_x, center_y );                    
    }

    // counter clockwise
    glTexCoord2f(u0,v1); glVertex3i( a.x, a.y, depth );
    glTexCoord2f(u1,v1); glVertex3i( d.x, d.y, depth );
    glTexCoord2f(u1,v0); glVertex3i( c.x, c.y, depth );
    glTexCoord2f(u0,v0); glVertex3i( b.x, b.y, depth ); 

}


void Prop2D::render(Camera *cam) {
    assertmsg(deck || grid_used_num > 0 || children_num > 0 || prim_drawer , "no deck/grid/prim_drawer is set ");
    float camx=0.0f;
    float camy=0.0f;
    if(cam){
        camx = cam->loc.x * -1;
        camy = cam->loc.y * -1;
    }


    // TODO: use vbo for grids
    if( grid_used_num > 0 ){
        glEnable(GL_TEXTURE_2D);
        glColor4f(color.r,color.g,color.b,color.a);        
        for(int i=0;i<grid_used_num;i++){
            Grid *grid = grids[i];
            if(!grid)break;
            if(!grid->visible)continue;

            TileDeck *draw_deck;
            if( grid->deck ){
                draw_deck = grid->deck;
                glBindTexture( GL_TEXTURE_2D, grid->deck->tex->tex );                
            } else {
                assertmsg( deck, "need deck when grid has no deck" );
                draw_deck = deck;
                glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
            }
            if( grid->fragment_shader ){
                glUseProgram(grid->fragment_shader->program );
                grid->fragment_shader->updateUniforms();
            }
            glBegin(GL_QUADS);
            glColor4f( grid->color.r, grid->color.g, grid->color.b, grid->color.a );

            for(int y=0;y<grid->height;y++){
                for(int x=0;x<grid->width;x++){
                    int ind = grid->get(x,y);
                    if( ind != Grid::GRID_NOT_USED ){
                        int ti = x + y * grid->width;

                        bool yflip=false, xflip=false, uvrot=false;
                        if( grid->xflip_table ) xflip = grid->xflip_table[ti];
                        if( grid->yflip_table ) yflip = grid->yflip_table[ti];
                        if( grid->rot_table ) uvrot = grid->rot_table[ti]; 
                        float texofs_x=0, texofs_y=0;
                        if( grid->texofs_table ){
                            texofs_x = grid->texofs_table[ti].x;
                            texofs_y = grid->texofs_table[ti].y;                            
                        }
                        if( grid->color_table ){
                            glColor4f( grid->color_table[ti].r,
                                       grid->color_table[ti].g,
                                       grid->color_table[ti].b,
                                       grid->color_table[ti].a                                       
                                       );
                        }
                        drawIndex( draw_deck,
                                   ind,
                                   camx + loc.x + x * scl.x + draw_offset.x - enfat_epsilon,
                                   camy + loc.y + y * scl.y + draw_offset.y - enfat_epsilon,
                                   camx + loc.x + (x+1) * scl.x + draw_offset.x + enfat_epsilon,
                                   camy + loc.y + (y+1)*scl.y + draw_offset.y + enfat_epsilon,
                                   xflip,
                                   yflip,
                                   texofs_x,
                                   texofs_y,
                                   uvrot,
                                   0 );
                    }
                }
            }
            glEnd();            
            if(grid->fragment_shader){
                glUseProgram(0);
            }
        }
    }

    if( children_num > 0 ){
        for(int i=0;i<children_num;i++){
            Prop2D *p = (Prop2D*) children[i];
            p->render( cam );
        }
    }
    
    if(deck){
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, deck->tex->tex );
        if( fragment_shader ){
            glUseProgram( fragment_shader->program );
            fragment_shader->updateUniforms();
        }
        glBegin(GL_QUADS);
        glColor4f(color.r,color.g,color.b,color.a);
        
        float minx, miny, maxx, maxy;
        minx = camx + loc.x - scl.x/2 + draw_offset.x;
        miny = camy + loc.y - scl.y/2 + draw_offset.y;
        maxx = camx + loc.x + scl.x/2 + draw_offset.x;
        maxy = camy + loc.y + scl.y/2 + draw_offset.y;

        drawIndex( deck, index, minx, miny, maxx, maxy, xflip, yflip, 0,0, uvrot, rot );
        glEnd();
        if( fragment_shader ){
            glUseProgram(0);
        }
    }

    // primitives should go over image sprites
    if( prim_drawer ){
        prim_drawer->drawAll(loc.add(camx,camy) );
    }
}

int Prop3D::countSpareChildren() {
    int cnt=0;
    for(int i=0;i<children_num;i++) {
        if( children[i] == NULL ) cnt ++;
    }
    return cnt;
}

void Prop3D::reserveChildren( int n ) {
    assert( n <= CHILDREN_ABS_MAX );
    size_t sz = sizeof(Prop3D*) * n;
    if( children ) {
        Prop3D **newptr = (Prop3D**) MALLOC( sz );
        for(int i=0;i<n;i++) newptr[i] = NULL;        
        for(int i=0;i<children_max;i++) {
            newptr[i] = children[i];
        }
        FREE(children);
        children = newptr;
    } else {
        children = (Prop3D**) MALLOC( sz );
        for(int i=0;i<n;i++) children[i] = NULL;
    }
    children_max = n;
}
void Prop3D::addChild( Prop3D *p ) {
    assert(p);
    assert( children_num < children_max );
    children[children_num] = p;
    children_num ++;
}
void Prop3D::deleteChild( Prop3D *p ) {
    assert(p);
    for(int i=0;i<children_num;i++) {
        if( children[i] == p ) {
            for(int j=i+1;j<children_num;j++) {
                children[j-1] = children[j];
            }
            children_num --;
            return;
        }
    }
}

bool Prop3D::propPoll(double dt) {    
    if( prop3DPoll(dt) == false ) return false;
    if( children_num > 0 ) { 
        // children
        for(int i=0;i<children_num;i++){
            Prop3D *p = children[i];
            p->basePoll(dt);
        }
    }
        
    return true;
}


bool Texture::load( const char *path ){
    tex = SOIL_load_OGL_texture( path,
                                 SOIL_LOAD_AUTO,
                                 SOIL_CREATE_NEW_ID,
                                 SOIL_FLAG_MULTIPLY_ALPHA );
    if(tex==0)return false;
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );     
    print("soil_load_ogl_texture: new texid:%d", tex );
    return true;
}
void Texture::setLinearMagFilter(){
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}
void Texture::setLinearMinFilter(){
    glBindTexture( GL_TEXTURE_2D, tex );    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );    
}


void TextBox::render(Camera *cam ) {
    glBindTexture( GL_TEXTURE_2D, font->atlas->id );

    size_t i;
    float x = loc.x;
    float y = loc.y;
    if(cam){
        x -= cam->loc.x;
        y -= cam->loc.y;
    }
    float basex = x;
        
    glBegin(GL_QUADS);
    glColor4f( color.r, color.g, color.b, color.a );
        
    for( i=0; i<wcslen(str); ++i ){
        if( str[i] == L"\n"[0] ){
            x = basex;
            y -= font->pixel_size;
            continue;
        }
        texture_glyph_t *glyph = texture_font_get_glyph( font->font, str[i] );
        if( glyph == NULL ) continue;
            
        int kerning = 0;
        if( i > 0){
            kerning = texture_glyph_get_kerning( glyph, str[i-1] );
        }
        x += kerning;
        int x0  = (int)( x + glyph->offset_x );
        int y0  = (int)( y + glyph->offset_y );
        int x1  = (int)( x0 + glyph->width );
        int y1  = (int)( y0 - glyph->height );
        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;
        float depth = 10;
        glTexCoord2f(s0,t0); glVertex3i( x0,y0, depth );
        glTexCoord2f(s0,t1); glVertex3i( x0,y1, depth );
        glTexCoord2f(s1,t1); glVertex3i( x1,y1, depth );
        glTexCoord2f(s1,t0); glVertex3i( x1,y0, depth );
            
        x += glyph->advance_x;

    }
    glEnd();
}


Prop *Prop2D::getNearestProp(){
    Prop *cur = parent_group->prop_top;
    float minlen = 999999999999999.0f;
    Prop *ans=0;
    while(cur){
        Prop2D *cur2d = (Prop2D*)cur;        
        if( cur2d->dimension == DIMENSION_2D ) {
            float l = cur2d->loc.len(loc);
            if(l<minlen && cur != this ){
                ans = cur;
                minlen = l;
            }
        }
        cur = cur->next;
    }
    return ans;
}

const char replacer_shader[] = 
    "uniform sampler2D texture;\n"
    "uniform vec3 color1;\n"
    "uniform vec3 replace1;\n"
    "uniform float eps;\n"
    "void main() {\n"
    "	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy); \n"
    "	if(pixel.r > color1.r - eps && pixel.r < color1.r + eps && pixel.g > color1.g - eps && pixel.g < color1.g + eps && pixel.b > color1.b - eps && pixel.b < color1.b + eps ){\n"
    "		pixel = vec4(replace1, pixel.a );\n"
    "    }\n"
    "	gl_FragColor = pixel;\n"
    "}\n";

static bool shaderCompile(GLuint shader, const char *src){
    GLsizei len = strlen(src), size;
    GLint compiled;
 
    glShaderSource(shader, 1, (const GLchar**) & src, (GLint*)&len );

    // シェーダのコンパイル
    glCompileShader(shader);
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

    if ( compiled == GL_FALSE ){
        print( "couldn't compile shader: %s \n ", src );
        glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &size );
        if ( size > 0 ){
            GLchar *buf;
            buf = (char *)MALLOC(size);
            glGetShaderInfoLog( shader, size, &len, buf);
            printf("%s",buf);
            FREE(buf);
        }
        return false;
    }
    return true;
}

static bool link( GLuint prog ){
    GLsizei size, len;
    GLint linked;
    char *infoLog ;

    glLinkProgram( prog );

    glGetProgramiv( prog, GL_LINK_STATUS, &linked );

    if ( linked == GL_FALSE ){
        print("couldnt link shader:\n");
    
        glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &size );
        if ( size > 0 ){
            infoLog = (char *)MALLOC(size);
            glGetProgramInfoLog( prog, size, &len, infoLog );
            printf("%s",infoLog);
            FREE(infoLog);
        }
        return false;
    }
    return true;
}


bool FragmentShader::load( const char *src) {
    GLuint shader = glCreateShader( GL_FRAGMENT_SHADER );
    if(!shaderCompile( shader, src)){
        glDeleteShader(shader);
        return false;
    }
    
    program = glCreateProgram();
    glAttachShader( program, shader );
    glDeleteShader(shader);
    
    if(!link( program ) ){
        return false;
    }
    return true;
}

bool ColorReplacerShader::init(){
    return load( replacer_shader );
}

void ColorReplacerShader::updateUniforms(){
    float fromcol[]={ from_color.r, from_color.g, from_color.b};
    GLuint uniid1=glGetUniformLocation(program,"color1");
    glUniform3fv( uniid1,1,fromcol);
    
    float tocol[]={ to_color.r, to_color.g, to_color.b };
    GLuint uniid2=glGetUniformLocation(program,"replace1");
    glUniform3fv( uniid2,1,tocol);

    GLuint uniid3=glGetUniformLocation(program,"texture");
    glUniform1i(uniid3, 0 );

    GLuint uniid4=glGetUniformLocation(program,"eps");
    glUniform1f(uniid4, epsilon );
    
}

void Camera::screenToGL( int scr_x, int scr_y, int scrw, int scrh, Vec2 *out ) {    
    out->x = scr_x - scrw/2;
    out->y = scr_y - scrh/2;
    out->y *= -1;
}

Vec2 Camera::screenToWorld( int scr_x, int scr_y, int scr_w, int scr_h ) {
    Vec2 glpos;
    screenToGL( scr_x, scr_y, scr_w, scr_h, & glpos );
    return glpos + Vec2(loc.x,loc.y);
}


void CharGrid::printf( int x, int y, Color c, const char *fmt, ...) {
    va_list argptr;
    char dest[1024];
    va_start(argptr, fmt);
    vsnprintf( dest, sizeof(dest), fmt, argptr );
    va_end(argptr);

    int l = strlen(dest);
    for(int i=0;i<l;i++){
        int ind = ascii_offset + dest[i];
        if(x+i>=width)break;
        set(x+i,y,ind);
        setColor(x+i,y,c);
    }    
}

void Grid::clear(){
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            set(x,y,GRID_NOT_USED);
        }
    }
}

void Layer::selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen ){
    assertmsg( viewport->dimension == DIMENSION_2D, "selectCenterInside isn't implemented for 3d viewport" );
    
    Prop *cur = prop_top;
    int out_max = *outlen;
    int cnt=0;
    
    while(cur){
        Prop2D *cur2d = (Prop2D*) cur;
        if( cur2d->dimension == DIMENSION_2D ){
            if( !cur->to_clean && cur2d->isCenterInside(minloc, maxloc) ){
                if( cnt < out_max){
                    out[cnt] = cur;
                    cnt++;
                    if(cnt==out_max)break;
                }
            }
        }
        cur = cur->next;
    }
    *outlen = cnt;
}

void Viewport::setSize(int scrw, int scrh ) {
    screen_width = scrw;
    screen_height = scrh;
    glViewport(0,0,screen_width,screen_height);
}
void Viewport::setScale2D( float sx, float sy ){
    dimension = DIMENSION_2D;
    scl = Vec3(sx,sy,1);

}
void Viewport::setClip3D( float near, float far ) {        
    near_clip = near;
    far_clip = far;
    dimension = DIMENSION_3D;
}

void Viewport::getMinMax( Vec2 *minv, Vec2 *maxv ){
    minv->x = -scl.x/2;
    maxv->x = scl.x/2;
    minv->y = -scl.y/2;
    maxv->y = scl.y/2;
}

void Prop2D::updateMinMaxSizeCache(){
    max_rt_cache.x=0;
    max_rt_cache.y=0;
    min_lb_cache.x=0;
    min_lb_cache.y=0;
    
    float grid_max_x=0, grid_max_y=0;
    if(grid_used_num>0){
        for(int i=0;i<grid_used_num;i++){
            Grid *g = grids[i];
            float maxx = g->width * scl.x;
            if(maxx>grid_max_x )grid_max_x = maxx;
            float maxy = g->height * scl.y;
            if(maxy>grid_max_y )grid_max_y = maxy;
        }
    }
    if( grid_max_x > max_rt_cache.x ) max_rt_cache.x = grid_max_x;
    if( grid_max_y > max_rt_cache.y ) max_rt_cache.y = grid_max_y;

    //

    float child_max_x=0, child_max_y = 0;
    if( children_num > 0){
        for(int i=0;i<children_num;i++){
            Prop *p = children[i];
            float maxx = ((Prop2D*)p)->scl.x/2;
            float maxy = ((Prop2D*)p)->scl.y/2;
            if( maxx > child_max_x ) child_max_x = maxx;
            if( maxy > child_max_y ) child_max_y = maxy;
        }
    }
    if( child_max_x > max_rt_cache.x ) max_rt_cache.x = child_max_x;
    if( child_max_y > max_rt_cache.y ) max_rt_cache.y = child_max_y;
    
    if( prim_drawer ){
        Vec2 minv, maxv;
        prim_drawer->getMinMax( &minv, &maxv );
        if( minv.x < min_lb_cache.x ) min_lb_cache.x = minv.x;
        if( minv.y < min_lb_cache.y ) min_lb_cache.y = minv.y;
        if( maxv.x > max_rt_cache.x ) max_rt_cache.x = maxv.x;
        if( maxv.y > max_rt_cache.y ) max_rt_cache.y = maxv.y;
    }
}

void PrimDrawer::getMinMax( Vec2 *minv, Vec2 *maxv ) {
    *minv = Vec2(0,0);
    *maxv = Vec2(0,0);
    
    for(int i=0;i<prim_num;i++){
        Prim *prm = prims[i];
        if( prm->a.x < minv->x ) minv->x = prm->a.x;
        if( prm->a.y < minv->y ) minv->y = prm->a.y;
        if( prm->a.x > maxv->x ) maxv->x = prm->a.x;
        if( prm->a.y > maxv->y ) maxv->y = prm->a.y;
        
        if( prm->b.x < minv->x ) minv->x = prm->b.x;
        if( prm->b.y < minv->y ) minv->y = prm->b.y;
        if( prm->b.x > maxv->x ) maxv->x = prm->b.x;
        if( prm->b.y > maxv->y ) maxv->y = prm->b.y;
    }
}

void Texture::setImage( Image *img ) {
    if(tex==0){
        glGenTextures( 1, &tex );
        assertmsg(tex!=0,"glGenTexture failed");
        glBindTexture( GL_TEXTURE_2D, tex );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 
        
    }
    
    glBindTexture(GL_TEXTURE_2D, tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->buffer );

    image = img;
}




void MoyaiClient::capture( Image *img ) {
    float *buf = (float*)MALLOC( img->width * img->height * 3 * sizeof(float) );
    glReadPixels( 0, 0, img->width, img->height, GL_RGB, GL_FLOAT, buf );
    for(int y=0;y<img->height;y++){
        for(int x=0;x<img->width;x++){
            int ind = (x + y * img->width)*3;
            float r = buf[ind+0], g = buf[ind+1], b = buf[ind+2];
            Color c( r,g,b,1);
            img->setPixel(x,img->height-1-y,c);
        }
    }    
    FREE(buf);
}

void Pad::readGLFW() {
    up = glfwGetKey('W');
    left = glfwGetKey('A');
    down = glfwGetKey('S');    
    right = glfwGetKey('D');
}






void VertexBuffer::reserve(int cnt){
    assertmsg(fmt, "vertex format is not set" );
    array_len = cnt;
    unit_num_float = fmt->getNumFloat();
    total_num_float = array_len * unit_num_float;
    buf = (float*)MALLOC( total_num_float * sizeof(float));
    assert(buf);
}

void VertexBuffer::copyFromBuffer( float *v, int vert_cnt ) {
    assertmsg( unit_num_float > 0, "call setFormat() before this." );
    assertmsg( vert_cnt <= array_len, "size too big");
    array_len = vert_cnt;
    total_num_float = vert_cnt * unit_num_float;
    memcpy( buf, v, vert_cnt * unit_num_float * sizeof(float) );
}
void VertexBuffer::setCoord( int index, Vec3 v ) {
    assertmsg(fmt, "vertex format is not set" );
    assert( index < array_len );
    int ofs = fmt->coord_offset;
    assertmsg( ofs >= 0, "coord have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    buf[index_in_array] = v.x;
    buf[index_in_array+1] = v.y;
    buf[index_in_array+2] = v.z;
}

Vec3 VertexBuffer::getCoord( int index ) {
    assertmsg(fmt, "vertex format is not set" );
    assert( index < array_len );
    int ofs = fmt->coord_offset;
    assertmsg( ofs >= 0, "coord have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    return Vec3( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2] );
}
void VertexBuffer::setCoordBulk( Vec3 *v, int num ) {
    for(int i=0;i<num;i++) {
        setCoord( i, v[i] );
    }
}
void VertexBuffer::setColor( int index, Color c ) {
    assertmsg(fmt, "vertex format is not set");
    assert( index < array_len );
    int ofs = fmt->color_offset;
    assertmsg( ofs >= 0, "color have not declared in vertex format");
    int index_in_array = index * unit_num_float + ofs;
    buf[index_in_array] = c.r;
    buf[index_in_array+1] = c.g;
    buf[index_in_array+2] = c.b;
    buf[index_in_array+3] = c.a;        
}
Color VertexBuffer::getColor( int index ) {
    assertmsg(fmt, "vertex format is not set" );
    assert( index < array_len );
    int ofs = fmt->color_offset;
    assertmsg( ofs >= 0, "color have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    return Color( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2], buf[index_in_array+3] );    
}
void VertexBuffer::setUV( int index, Vec2 uv ) {
    assertmsg(fmt, "vertex format is not set");
    assert( index < array_len );
    int ofs = fmt->texture_offset;
    assertmsg( ofs >= 0, "texcoord have not declared in vertex format");
    int index_in_array = index * unit_num_float + ofs;
    buf[index_in_array] = uv.x;
    buf[index_in_array+1] = uv.y;
}
Vec2 VertexBuffer::getUV( int index ) {
    assertmsg(fmt, "vertex format is not set" );
    assert( index < array_len );
    int ofs = fmt->texture_offset;
    assertmsg( ofs >= 0, "texuv have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    return Vec2( buf[index_in_array], buf[index_in_array+1] );
}

void VertexBuffer::setUVBulk( Vec2 *uv, int num ) {
    for(int i=0;i<num;i++) {
        setUV( i, uv[i] );
    }
}
void VertexBuffer::setNormal( int index, Vec3 v ) { 
    assertmsg(fmt, "vertex format is not set");
    assert( index < array_len );
    int ofs = fmt->normal_offset;
    assertmsg( ofs >= 0, "normal have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    buf[index_in_array] = v.x;
    buf[index_in_array+1] = v.y;
    buf[index_in_array+2] = v.z;        
}
Vec3 VertexBuffer::getNormal( int index ) {
    assertmsg(fmt, "vertex format is not set" );
    assert( index < array_len );
    int ofs = fmt->normal_offset;
    assertmsg( ofs >= 0, "normal have not declared in vertex format" );
    int index_in_array = index * unit_num_float + ofs;
    return Vec3( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2] );    
}
void VertexBuffer::setNormalBulk( Vec3 *v, int num ) {
    for(int i=0;i<num;i++) {
        setNormal( i, v[i] );
    }
}
void VertexBuffer::bless(){
    assert(fmt);
    if( gl_name == 0 ){
        glGenBuffers(1, &gl_name);
        glBindBuffer( GL_ARRAY_BUFFER, gl_name );
        glBufferData( GL_ARRAY_BUFFER, total_num_float * sizeof(float), buf, GL_STATIC_DRAW );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
}
Vec3 VertexBuffer::calcCenterOfCoords() {
    Vec3 c(0,0,0);
    for(int i=0;i<array_len;i++) {
        c += getCoord(i);
    }
    c /= (float)array_len;
    return c;
}


IndexBuffer::~IndexBuffer() {
    assert(buf);
    FREE(buf);
    glDeleteBuffers(1,&gl_name);
}
void IndexBuffer::reserve( int len ) {
    if(buf) FREE(buf);    
    buf = (int*) MALLOC( sizeof(int) * len );
    assert(buf);
    array_len = len;
}
void IndexBuffer::setIndex( int index_at, int val ) {
    assert(buf);
    assert(index_at >= 0 && index_at < array_len );
    buf[index_at] = val;
}
int IndexBuffer::getIndex( int index_at ) {
    assert(buf);
    assert(index_at >= 0 && index_at < array_len );
    return buf[index_at];
}
    
void IndexBuffer::set( int *in, int l ) {
    reserve(l);
    memcpy(buf, in, sizeof(int)*l);
}

void IndexBuffer::bless(){
    if( gl_name == 0 ){
        glGenBuffers(1, &gl_name);
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl_name );
        // データがよく変わるときは GL_DYNAMIC_DRAWらしいけど、それはコンセプトから外れた使い方だからデフォルトはSTATICにしておく。
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * array_len, buf, GL_STATIC_DRAW );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
}


void VertexFormat::dump() {
    print("vfmt: types_used:%d num_float:%d coord_ofs:%d color_ofs:%d tex_ofs:%d normal_ofs:%d",
          types_used, num_float, coord_offset, color_offset, texture_offset, normal_offset );
    for(int i=0;i<elementof(types);i++) {
        print("type[%d]=%c", i, types[i]);
    }
}
void VertexBuffer::dump() {
    print("vb: len:%d nfloat:%d unitfloat:%d glname:%d", array_len, total_num_float, unit_num_float, gl_name );
    if(fmt) fmt->dump(); else print("vb:nofmt");
    if(buf) {
        for(int i=0;i<array_len;i++){
            for(int j=0;j<unit_num_float;j++){
                print("[%d][%d]=%.10f", i, j, buf[i*unit_num_float+j]);
            }
        }
    }
}
void IndexBuffer::dump() {
    print("ib: len:%d glname:%d", array_len, gl_name );
    if(buf){
        for(int i=0;i<array_len;i++) {
            print("[%d]=%d",i, buf[i] );
        }
    } else {
        print("ib:nobuf");
    }
}
void Mesh::dump() {
    print("mesh: primtype:%d transparent:%d", prim_type, transparent );
    if(vb) vb->dump(); else print("no-vb" );
    if(ib) ib->dump(); else print("no-ib" );
}
/////////

inline void FMOD_ERRCHECK(FMOD_RESULT result){
    if (result != FMOD_OK){
        assertmsg( false, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result) );
    }
}


Sound::Sound( SoundSystem *s) : sound(0), parent(s), ch(0), default_volume(1) { }
void Sound::play(){
    play(default_volume);
}
void Sound::play(float vol){
    if(vol==0)return;
    FMOD_RESULT r;
    if( !this->ch ){
        //            print("free:%p",this); // FMODでは、FREEをつかってchannelを割り当てた後、reuseする。
        r = FMOD_System_PlaySound( parent->sys, FMOD_CHANNEL_FREE, sound, 0, & this->ch );
    } else {
        //            print("reuse:%p",this);            
        r = FMOD_System_PlaySound( parent->sys, FMOD_CHANNEL_REUSE, sound, 0, & this->ch );            
    }
    FMOD_ERRCHECK(r);
    FMOD_Channel_SetVolume(ch, default_volume * vol );
}


SoundSystem::SoundSystem()  : sys(0) {
    FMOD_RESULT r;
    r = FMOD_System_Create(&sys);
    FMOD_ERRCHECK(r);
        
    unsigned int version;
    r = FMOD_System_GetVersion(sys, &version);
    FMOD_ERRCHECK(r);
    if(version < FMOD_VERSION ){
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return;
    }
    r = FMOD_System_Init( sys, 32, FMOD_INIT_NORMAL, NULL );
    FMOD_ERRCHECK(r);
}

Sound *SoundSystem::newSound( const char *path, float vol ) {
    FMOD_RESULT r;
    Sound *out = new Sound(this);
    FMOD_SOUND *s;
    r = FMOD_System_CreateSound(sys, path, FMOD_SOFTWARE, 0, &s );
    FMOD_ERRCHECK(r);
    FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
    out->sound = s;
    out->default_volume = vol;
    return out;
}
Sound *SoundSystem::newSound( const char *path ){
    return newSound( path, 1.0 );
}

    

////////