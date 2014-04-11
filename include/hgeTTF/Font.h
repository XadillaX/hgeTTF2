#ifndef HGETTF_FONT_H
#define HGETTF_FONT_H

#include <map>
#include <hge.h>
#include <hgeSprite.h>

#ifndef FT_FREETYPE_H
	struct FT_LibraryRec_;
	typedef FT_LibraryRec_* FT_Library;

	struct FT_FaceRec_;
	typedef FT_FaceRec_* FT_Face;

	struct FT_Bitmap_;
	typedef FT_Bitmap_ FT_Bitmap;
#endif

namespace hgeTTF
{

class Font
{
public:
	typedef unsigned int uint32_t;
	typedef int int32_t;

	Font(FT_Library Library);
	~Font();

	bool Load(const std::string& Filename);
	void Unload();

	const std::string getFilename() {return mFilename;}

	bool cacheChar(wchar_t charCode);
	bool cacheChars(const std::wstring& s);
	bool cacheChars(wchar_t startCode, wchar_t endCode);
	bool isCharCached(wchar_t charCode);
	void clearCache();

	bool drawChar(wchar_t charCode, float x, float y);
	bool drawString(const std::wstring& s, float x, float y);
	bool printf(float x, float y, const wchar_t* format, ...);

	float getCharWidth(wchar_t charCode);
	float getCharHeight(wchar_t charCode);
	float getStringWidth(const std::wstring& s);
	float getStringHeight(const std::wstring& s);

	void setColor(uint32_t Color, int i = -1) {mSprite->SetColor(Color, i);}
	void setBlendMode(int Mode) {mSprite->SetBlendMode(Mode);}
	void setHotSpot(float relX, float relY) {mHotSpotX = relX; mHotSpotY = relY;}
	void setXScale(float Scale) {mXScale = Scale;}
	void setYScale(float Scale) {mYScale = Scale;}
	void setScale(float Scale) {mXScale = mYScale = Scale;}
	void setRotation(float Rotation) {mRotation = Rotation;}

	void setWidth(uint32_t Width) {mWidth = Width;}
	void setHeight(uint32_t Height) {mHeight = Height;}
	void setSize(uint32_t Size) {mWidth = mHeight = Size;}

	uint32_t getColor(int i = -1) {return mSprite->GetColor(i);}
	int getBlendMode() {return mSprite->GetBlendMode();}
	void getHotSpot(float& hotX, float& hotY) {hotX = mHotSpotX; hotY = mHotSpotY;}
	float getRotation() {return mRotation;}

	void autoCache(bool AutoCache) {mAutoCache = AutoCache;}
	bool autoCache() {return mAutoCache;}

	void useKerning(bool Kerning) {mUseKerning = Kerning;}
	bool usingKerning() {return mUseKerning;}
	bool hasKerning() {return mHaveKerning;}

private:
	struct CacheEntry
	{
		wchar_t charCode;
		uint32_t glyphIndex;
		uint32_t Advance;
		/* Settings this glyph was rendered with */
		uint32_t Width, Height;

		/* The pixel width/height of the glyph */
		uint32_t glyphWidth, glyphHeight;
		uint32_t Left, Top;

		HTEXTURE Texture;
		uint32_t textureWidth, textureHeight;
	};
	typedef std::multimap<wchar_t, CacheEntry*> CacheList;
	typedef CacheList::iterator CacheListIterator;

	//Number of spaces in a tab
	static const unsigned int TabSize = 4;

	Font(const Font&);
	const Font& operator=(const Font&);
	bool setupSize();
	CacheEntry* getCacheEntry(wchar_t charCode);
	void renderGlyph(FT_Bitmap* Glyph, HTEXTURE& Texture, uint32_t& textureWidth, uint32_t& textureHeight);
	uint32_t spaceAdvance();

	HGE* mHGE;
	std::string mFilename;
	FT_Library mLibrary;
	FT_Face mFace;
	uint32_t mWidth, mHeight;
	bool mAutoCache;
	bool mHaveKerning;
	bool mUseKerning;

	hgeSprite* mSprite;
	float mHotSpotX, mHotSpotY;
	float mXScale, mYScale;
	float mRotation;

	uint32_t mLastSetWidth, mLastSetHeight;
	std::wstring mWBuffer;

	CacheList mCache;
};

}

#endif
