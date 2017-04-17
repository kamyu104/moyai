#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define MAX_FACES	10

static FT_Library	library;
static FT_Face		faces[MAX_FACES];	// 0 & 1 reserved for OCR-A/B
static char*		names[MAX_FACES];	// strdup'd in main() for OCR-A/B
static int			mults[MAX_FACES] = { 90, 90 };	
static FT_GlyphSlot	slot;

static unsigned char*	monomap;		// monochrome bitmap
static int				monomode;		// monochrome mode

static int debug_code;

// Unpack a monochrome bitmap into anti-aliased bitmap form
static int mono_unpack(FT_Bitmap* bitmap) {
	if (monomap)
		free(monomap);
	monomap = (unsigned char*)malloc(bitmap->width * bitmap->rows);
	if (!monomap)
		return -1;
	unsigned char* bytes = monomap;
	for (int y = 0; y < bitmap->rows; y++) {
		unsigned char* bits  = &bitmap->buffer[y * bitmap->pitch];
		for (int x = 0; x < bitmap->width; x++) {
			*bytes++ = (bits[x >> 3] & (1 << (7 - (x & 07)))) ? 255 : 0;
		}
	}
	return 0;
}

int monochrome(int enable) {
	monomode = enable ? 1 : 0;
	return 0;
}

// The multiplier allows globally adjusting the font size by mult-%.
int load_font(const char* path, const char* name, int mult) {
	FT_Error error;
	int i;

	// First look for a font with the same name
	for (i = 0; i < MAX_FACES; i++) {
		if (names[i] && strcasecmp(names[i], name) == 0) {
			FT_Done_Face(faces[i]);
			free(names[i]);
			names[i] = 0;
			faces[i] = 0;
			break;
		}
	}
	if (i == MAX_FACES) {
		for (i = 0; i < MAX_FACES && names[i]; i++)
			;
	}
	if (i == MAX_FACES) {
		printf("load_font(%s,%s): too many fonts!\n", path, name);
		return -1;
	}

	error = FT_New_Face(library, path, 0, &faces[i]);
	if (error) {
		printf("New_Face(%s,%s) Error! %d\n", path, name, error);
		return error;
	}
	names[i] = strdup(name);
	mults[i] = mult;
	return 0;
}
int load_mem_font(const char *data, int datalen, const char *name, int mult ) {
	FT_Error error;
	int i;
    
	// First look for a font with the same name
	for (i = 0; i < MAX_FACES; i++) {
		if (names[i] && strcasecmp(names[i], name) == 0) {
			FT_Done_Face(faces[i]);
			free(names[i]);
			names[i] = 0;
			faces[i] = 0;
			break;
		}
	}
	if (i == MAX_FACES) {
		for (i = 0; i < MAX_FACES && names[i]; i++)
			;
	}
	if (i == MAX_FACES) {
		printf("load_mem_font(len:%d,%s): too many fonts!\n", (int)datalen, name);
		return -1;
	}

	error = FT_New_Memory_Face(library, (FT_Byte*)data, datalen, 0, &faces[i]);
	if (error) {
		printf("New_Memory_Face(len:%d, %s) Error! %d\n", (int) datalen, name, error);
		return error;
	}
	names[i] = strdup(name);
	mults[i] = mult;
	return 0;
    
}

int close_font(const char* name) {
	for (int i = 0; i < MAX_FACES; i++) {
		if (names[i] && strcasecmp(names[i], name) == 0) {
			FT_Done_Face(faces[i]);
			free(names[i]);
			names[i] = 0;
			faces[i] = 0;
			break;
		}
	}
	return 0;
}

int find_font(const char* name) {
	for (int i = 0; i < MAX_FACES && names[i]; i++) {
		if (strcasecmp(name, names[i]) == 0) {
			return i;
		}
	}
	// If not found, default is OCR-B
	return 1;
}
int get_debug_code() { return debug_code; }
int g_get_bitmap_retcode=0;
int get_bitmap_opt_retcode() { return g_get_bitmap_retcode;}
unsigned char* get_bitmap(int font, int ch, int width, int height) {
	FT_Error	error;
	FT_Face		face;
    g_get_bitmap_retcode = 0;
    debug_code = 1010;
	if (font < 0 || font >= MAX_FACES || !names[font]) {
        return 0;
    }

	face   = faces[font];
	width  = (width * mults[font] * 64) / 100;
	height = (height * mults[font] * 64) / 100;

	if (face == faces[1]) {
		// The A - Z characters in OCR-B are rendered a tad short, by design.
		// But it looks like hell when used in mixed alpha-numeric strings.
		// Make them the same size as the digits.
		if (ch >= 'A' && ch <= 'Z')
			height = (height * 108) / 100;
	}
	/* 1pt == 1px == 72dpi */
	error = FT_Set_Char_Size(face, width, height, 72, 0 );
	if (error) {
		printf("Set_Char_Size Error! %d\n", error);
        debug_code = 11;
		return 0;
	}

	slot = face->glyph;

	if (face == faces[0] || face == faces[1]) {
		// The OCR fonts we use have some quirks in their glyph maps.
		// ^ is mapped as U+02C6 circumflex
		// ~ is mapped as U+02DC tilde
		if (ch == '~')
			ch = 0x02dc;
		else if (ch == '^')
			ch = 0x02c6;
	}

	if (monomode)
		error = FT_Load_Char(face, ch, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
	else
		error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
	if (error) {
		printf("Load_Char Error! %d\n", error);
        debug_code = 12;        
		return 0;
	}

	if (monomode) {
		if (mono_unpack(&slot->bitmap) != 0) {
        debug_code = 13;            
			return 0;
        }
		return monomap;
	}
    debug_code = 14;
    g_get_bitmap_retcode = 1;
	return slot->bitmap.buffer;
}

unsigned get_left() {
	return slot->bitmap_left;
}
unsigned get_top() {
	return slot->bitmap_top;
}
unsigned get_width() {
	return slot->bitmap.width;
}
unsigned get_height() {
	return slot->bitmap.rows;
}
unsigned get_pitch() {
	return slot->bitmap.pitch;
}
unsigned get_advance() {
	return slot->advance.x >> 6;
}

int main() {

	FT_Error	  error;

	error = FT_Init_FreeType(&library);
	if (error) {
		printf("Init Error! %d\n", error);
		return 1;
	}
	return 0;
}

