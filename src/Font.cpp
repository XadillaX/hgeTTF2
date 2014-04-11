#include "hgeTTF/Font.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace hgeTTF
{

Font::Font(FT_Library Library) :	mHGE(hgeCreate(HGE_VERSION)),
									mLibrary(Library),
									mFace(NULL),
									mWidth(32),
									mHeight(32),
									mAutoCache(true),
									mHaveKerning(false),
									mUseKerning(true),
									mSprite(new hgeSprite(NULL, 0.0f, 0.0f, 0.0f, 0.0f)),
									mHotSpotX(0.5f),
									mHotSpotY(0.5f),
									mXScale(1.0f),
									mYScale(1.0f),
									mRotation(0.0f),
									mLastSetWidth(0),
									mLastSetHeight(0)
{

}

Font::~Font()
{
	Unload();
}

bool Font::Load(const std::string& Filename)
{
	if (FT_New_Face(mLibrary, Filename.c_str(), 0, &mFace) != 0)
		return false;

	//We only support scalable Unicode fonts
	if (!mFace->charmap || !FT_IS_SCALABLE(mFace))
	{
		FT_Done_Face(mFace);
		return false;
	}
	mFilename = Filename;
	if (FT_HAS_KERNING(mFace))
		mHaveKerning = true;

	return true;
}

void Font::Unload()
{
	mFilename.clear();
	clearCache();
	delete mSprite;
	mSprite = NULL;
	if (mHGE)
	{
		mHGE->Release();
		mHGE = NULL;
	}
	if (mFace != NULL)
	{
		FT_Done_Face(mFace);
		mFace = NULL;
	}
}

bool Font::cacheChar(wchar_t charCode)
{
	if (isCharCached(charCode))
		return true;

	if (charCode == 0)
		return false;

	if (!setupSize())
		return false;

	CacheEntry* Entry = new CacheEntry;
	Entry->charCode = charCode;
	Entry->glyphIndex = FT_Get_Char_Index(mFace, charCode);
	if (Entry->glyphIndex == 0)
	{
		delete Entry;
		return false;
	}
	if (FT_Load_Glyph(mFace, Entry->glyphIndex, FT_LOAD_RENDER) != 0)
	{
		delete Entry;
		return false;
	}
	Entry->Width = mWidth;
	Entry->Height = mHeight;
	Entry->glyphWidth = mFace->glyph->bitmap.width;
	Entry->glyphHeight = mFace->glyph->bitmap.rows;
	renderGlyph(&mFace->glyph->bitmap, Entry->Texture, Entry->textureWidth, Entry->textureHeight);
	Entry->Left = mFace->glyph->bitmap_left;
	Entry->Top = mFace->glyph->bitmap_top;
	Entry->Advance = mFace->glyph->advance.x / 64;
	mCache.insert(std::pair<wchar_t, CacheEntry*>(charCode, Entry));
	return true;
}

bool Font::cacheChars(const std::wstring& s)
{
	std::size_t Length = s.length();
	bool Ret = true;

	for (std::size_t i = 0; i < Length; ++i)
	{
		if (!cacheChar(s[i]))
			Ret = false;
	}
	return Ret;
}

bool Font::cacheChars(wchar_t startCode, wchar_t endCode)
{
	bool Ret = true;
	for (wchar_t i = startCode; i != endCode; ++i)
	{
		if (!cacheChar(i))
			Ret = false;
	}
	return Ret;
}

bool Font::isCharCached(wchar_t charCode)
{
	return getCacheEntry(charCode) != NULL;
}

void Font::clearCache()
{
	while (!mCache.empty())
	{
		CacheEntry* Entry = mCache.begin()->second;
		mCache.erase(mCache.begin());
		delete Entry;
	}
}

bool Font::drawChar(wchar_t charCode, float x, float y)
{
	if (mAutoCache)
		cacheChar(charCode);

	CacheEntry* theGlyph = getCacheEntry(charCode);
	if (NULL == theGlyph || NULL == theGlyph->Texture)
		return false;

	x += theGlyph->Left;
	y -= theGlyph->Top;

	mSprite->SetTexture(theGlyph->Texture);
	mSprite->SetTextureRect(0.0f, 0.0f, static_cast<float>(theGlyph->textureWidth), static_cast<float>(theGlyph->textureHeight));

	x += mHotSpotX * theGlyph->glyphWidth;
	y += mHotSpotY * theGlyph->glyphHeight;

	mSprite->SetHotSpot(mHotSpotX * theGlyph->glyphWidth, mHotSpotY * theGlyph->glyphHeight);
	mSprite->RenderEx(x, y, mRotation, mXScale, mYScale);
	mSprite->SetHotSpot(0.0f, 0.0f);
	return true;
}

bool Font::drawString(const std::wstring& s, float x, float y)
{
	bool Ret = true;
	float penX = x, penY = y;
	uint32_t previousGlyph = 0;
	FT_Vector Delta;
	CacheEntry* theGlyph = NULL;
	std::size_t Length = s.length();

	for (std::size_t i = 0; i < Length; ++i)
	{
		if (mAutoCache)
			cacheChar(s[i]);

		//Special cases
		switch (s[i])
		{
		case L'\t': penX += spaceAdvance() * TabSize;	previousGlyph = 0; continue;
		case L'\v':	penY += mHeight * TabSize;			previousGlyph = 0; continue;
		case L'\n':	penY += mHeight; penX = x;			previousGlyph = 0; continue;
		}
		theGlyph = getCacheEntry(s[i]);
		if (theGlyph == NULL)
		{
			Ret = false;
			previousGlyph = 0;
			continue;
		}
		//Kerning
		if (previousGlyph != 0 && mHaveKerning && mUseKerning)
		{
			FT_Get_Kerning(mFace, previousGlyph, theGlyph->glyphIndex, FT_KERNING_DEFAULT, &Delta);
			penX += Delta.x >> 6;
			penY += Delta.y >> 6;
		}
		if (!drawChar(s[i], penX, penY))
			Ret = false;

		penX += theGlyph->Advance;
		previousGlyph = theGlyph->glyphIndex;
	}
	return Ret;
}

bool Font::printf(float x, float y, const wchar_t* format, ...)
{
	va_list ArgPtr;

	va_start(ArgPtr, format);
	std::size_t Length = _vscwprintf(format, ArgPtr) + 1;
	mWBuffer.resize(Length);
	vswprintf_s(&mWBuffer[0], Length, format, ArgPtr);
	va_end(ArgPtr);
	return drawString(mWBuffer, x, y);
}

float Font::getCharWidth(wchar_t charCode)
{
	CacheEntry* theGlyph = getCacheEntry(charCode);

	if (theGlyph == NULL)
		return 0;

	return static_cast<float>(theGlyph->glyphWidth);
}

float Font::getCharHeight(wchar_t charCode)
{
	CacheEntry* theGlyph = getCacheEntry(charCode);

	if (theGlyph == NULL)
		return 0;

	return static_cast<float>(theGlyph->glyphHeight);
}

float Font::getStringWidth(const std::wstring& s)
{
	CacheEntry* theGlyph = NULL;
	uint32_t previousGlyph = 0;
	float Width = 0.0f;
	FT_Vector Delta;
	std::size_t Length = s.length();

	/* Note that we don't count the last char's advance */
	for (std::size_t i = 0; i < Length; ++i)
	{
		if (mAutoCache)
			cacheChar(s[i]);

		switch (s[i])
		{
		case L'\t': Width += spaceAdvance() * TabSize;	previousGlyph = 0; continue;
		case L'\v':													previousGlyph = 0; continue;
		case L'\n':													previousGlyph = 0; continue;
		}
		theGlyph = getCacheEntry(s[i]);
		if (theGlyph == NULL)
		{
			previousGlyph = 0;
			continue;
		}
		if (previousGlyph != 0 && mHaveKerning && mUseKerning)
		{
			FT_Get_Kerning(mFace, previousGlyph, theGlyph->glyphIndex, FT_KERNING_DEFAULT, &Delta);
			Width += Delta.x >> 6;
		}
		if (i == Length - 1)
		{
			Width += theGlyph->glyphWidth;
		}
		else
			Width += theGlyph->Advance;

		previousGlyph = theGlyph->glyphIndex;
	}
	return Width;
}

float Font::getStringHeight(const std::wstring& s)
{
	CacheEntry* theGlyph = NULL;
	float Height = 0.0f;
	std::size_t Length = s.length();

	for (std::size_t i = 0; i < Length; ++i)
	{
		switch (s[i])
		{
		case L'\v':	Height += mHeight * TabSize;	continue;
		case L'\n':	Height += mHeight;				continue;
		}
		theGlyph = getCacheEntry(s[i]);
		if (theGlyph == NULL)
			continue;

		if (theGlyph->glyphHeight > Height)
			Height = static_cast<float>(theGlyph->glyphHeight);
	}
	return Height;
}

/************************************************************************/
/*                               Private                                */
/************************************************************************/
bool Font::setupSize()
{
	if (mLastSetWidth != mWidth || mLastSetHeight != mHeight)
	{
		if (FT_Set_Pixel_Sizes(mFace, mWidth, mHeight) != 0)
			return false;

		mLastSetWidth = mWidth;
		mLastSetHeight = mHeight;
	}
	return true;
}

Font::CacheEntry* Font::getCacheEntry(wchar_t charCode)
{
	std::pair<CacheListIterator, CacheListIterator> Range(mCache.equal_range(charCode));
	CacheListIterator itStart(Range.first);
	CacheListIterator itEnd(Range.second);

	for (; itStart != itEnd; ++itStart)
	{
		CacheEntry* theEntry = itStart->second;
		if (theEntry->Width == mWidth && theEntry->Height == mHeight)
			return theEntry;
	}
	return NULL;
}

void Font::renderGlyph(FT_Bitmap* Glyph, HTEXTURE& Texture, uint32_t& textureWidth, uint32_t& textureHeight)
{
	Texture = NULL;
	uint32_t* texturePtr = NULL;
	textureWidth = 0;
	textureHeight = 0;
	uint32_t glyphWidth = Glyph->width;
	uint32_t glyphHeight = Glyph->rows;
	uint32_t Pixel = (0xFF << 16) | (0xFF << 8) | 0xFF;

	if (glyphWidth == 0 || glyphHeight == 0)
		return;

	Texture = mHGE->Texture_Create(glyphWidth, glyphHeight);
	if (Texture == NULL)
		return;

	textureWidth = mHGE->Texture_GetWidth(Texture);
	textureHeight = mHGE->Texture_GetHeight(Texture);
	texturePtr = (uint32_t*)mHGE->Texture_Lock(Texture, false);
	for (unsigned int i = 0; i < textureWidth * textureHeight; i += 4)
		*(texturePtr + i) = Pixel;

	for (unsigned int row = 0; row < glyphHeight; ++row)
	{
		typedef unsigned char uint8_t;
		uint8_t* Src = Glyph->buffer + row * Glyph->pitch;
		for (unsigned int col = 0; col < glyphWidth; ++col)
		{
			uint32_t Alpha = *Src++;
			texturePtr[row * textureWidth + col] = Pixel | (Alpha << 24);
		}
	}
	/*
	TEST
	Adds border around texture and character.
	*/
#if 0
	for (int i = 0; i < textureWidth; i++)
	{
		texturePtr[i] = 0xffff0000;
		texturePtr[(textureHeight - 1) * textureWidth + i] = 0xffff0000;
	}
	for (int i = 0; i < textureHeight; i++)
	{
		texturePtr[i * textureWidth] = 0xffff0000;
		texturePtr[i * textureWidth + (textureWidth - 1)] = 0xffff0000;
	}
#endif
#if 0
	for (int i = 0; i < charWidth; i++)
	{
		texturePtr[i] = 0xff00ff00;
		texturePtr[(charHeight - 1) * textureWidth + i] = 0xff00ff00;
	}
	for (int i = 0; i < charHeight; i++)
	{
		texturePtr[i * textureWidth] = 0xff00ff00;
		texturePtr[i * textureWidth + (charWidth - 1)] = 0xff00ff00;
	}
#endif
	/*
	END TEST
	*/
	mHGE->Texture_Unlock(Texture);
}

#if 0
void Font::renderGlyph(FT_Bitmap* Glyph, HTEXTURE Texture)
{
	uint32_t glyphWidth = Glyph->width;
	uint32_t glyphHeight = Glyph->rows;
	unsigned int textureWidth = Texture.GetValidTextureSize(glyphWidth);
	unsigned int textureHeight = Texture.GetValidTextureSize(glyphHeight);

	if (glyphWidth == 0 || glyphHeight == 0)
		return;

	Texture.Resize(textureWidth, textureHeight, sf::Color(255, 255, 255, 0));

	for (unsigned int row = 0; row < glyphHeight; ++row)
	{
		uint8_t* Src = Glyph->buffer + row * Glyph->pitch;
		for (unsigned int col = 0; col < glyphWidth; ++col)
		{
			uint32_t Alpha = *Src++;
			Texture.SetPixel(col, row, sf::Color(255, 255, 255, static_cast<uint8_t>(Alpha)));
		}
	}
	/*
	Testing
	*/
#if 0
	for (unsigned int i = 0; i < glyphWidth; ++i)
	{
		Texture.SetPixel(i, 0, sf::Color::Red);
		Texture.SetPixel(i, glyphHeight - 1, sf::Color::Red);
	}
	for (unsigned int i = 0; i < glyphHeight; ++i)
	{
		Texture.SetPixel(1, i, sf::Color::Red);
		Texture.SetPixel(glyphWidth - 1, i, sf::Color::Red);
	}
#endif
#if 0
	static bool bottom = true;
	for (int i = 0; i < Adv; ++i)
	{
		if (bottom)
		{
			Texture.SetPixel(i, glyphHeight + 2, sf::Color::Cyan);
			Texture.SetPixel(i, glyphHeight + 3, sf::Color::Cyan);
		}
		else
		{
			Texture.SetPixel(i, 1, sf::Color::Cyan);
			Texture.SetPixel(i, 2, sf::Color::Cyan);
		}
	}
	bottom = !bottom;
#endif
#if 0
	for (unsigned int i = 0; i < textureWidth; ++i)
	{
		Texture.SetPixel(i, 0, sf::Color::Green);
		Texture.SetPixel(i, textureHeight - 2, sf::Color::Green);
	}
	for (unsigned int i = 0; i < textureHeight; ++i)
	{
		Texture.SetPixel(1, i, sf::Color::Green);
		Texture.SetPixel(textureWidth - 1, i, sf::Color::Green);
	}
#endif
	/*
	END TEST
	*/
}
#endif

Font::uint32_t Font::spaceAdvance()
{
	//This is always done, regardless of auto cache
	cacheChar(L' ');
	CacheEntry* Space = getCacheEntry(L' ');
	return Space->Advance;
}

}
