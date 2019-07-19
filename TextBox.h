#pragma once

#include "Prop2D.h"
#include "Font.h"

#if !defined(__linux__)
#include "freetype-gl/vertex-buffer.h"
#endif

class TextBox : public Prop2D {
public:
	wchar_t *str;
	Font *font;
    int len_str;
    Mesh *mesh;
    TrackerTextBox *tracker;
    bool skip_meshing; // true for repreproxy    
	TextBox() : str(NULL), font(0), len_str(0), mesh(NULL), tracker(0), skip_meshing(false) {
		setScl(1,1);
	}
    ~TextBox() {
        if(mesh) {
            mesh->deleteBuffers();
            delete mesh;
        }
        if(tracker) delete tracker;
    }

	inline void setFont( Font *f ){
		assert(f);
		font = f;
	}

	void render(Camera *cam, DrawBatchList *bl );

	inline void setString( const char *s ){
		setString( (char*) s );
	}
	inline void setString( char *u8s ){
		int l = strlen(u8s);
		wchar_t *out = (wchar_t*)MALLOC((l+1)*sizeof(wchar_t));
		mbstowcs(out, u8s, l+1 );
		setString(out);
		FREE(out);
	}

	inline void setString( const wchar_t *s ){
		setString( (wchar_t*)s );
	}
	inline void setString( wchar_t *s ){
		size_t l = wcslen(s);
		if(str){
			FREE(str);
		}
		str = (wchar_t*)MALLOC( (l+1) * sizeof(wchar_t) );
		wcscpy( str, s );
		assert( wcslen(str) == wcslen(s) );
        len_str = l;
        clearMesh();
        if(!skip_meshing) updateMesh();
	}
    bool compareString( const char *u8s ) {
        int l = strlen(u8s);
		wchar_t *out = (wchar_t*)MALLOC((l+1)*sizeof(wchar_t));
		mbstowcs(out, u8s, l+1 );
        int ret = wcscmp( str, out );
        FREE(out);
        //        print("compareString: %d '%S' '%S'",ret, str, out );
        return ret == 0;
    }
    int getStringLength() { return len_str; }
    void clearMesh() {
        if(mesh) {
            mesh->deleteBuffers();
            delete mesh;
            mesh = NULL;
        }
    }
    void updateMesh();
    virtual void onTrack( RemoteHead *rh, Prop2D *parentprop );
    virtual void onColorChanged() {
        clearMesh();
        if(!skip_meshing) updateMesh();
    }
    static void drawToDBL( Layer *l, DrawBatchList *bl, bool additiveblend, Font *font, const char *s, Color col, Vec2 loc, float scl, float rot );    
};
