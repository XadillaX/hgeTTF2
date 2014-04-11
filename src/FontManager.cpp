#include <ft2build.h>
#include FT_FREETYPE_H

#include "hgeTTF/FontManager.h"
#include "hgeTTF/Font.h"

namespace hgeTTF
{

FontManager::FontManager() : mInitialized(false)
{

}

FontManager::~FontManager()
{
	unloadFonts();
	if (mInitialized)
		FT_Done_FreeType(mLibrary);
}

bool FontManager::Initialize()
{
	if (isInitialized())
		return true;

	if (FT_Init_FreeType(&mLibrary) != 0)
		return false;

	mInitialized = true;
	return true;
}

bool FontManager::loadFont(const std::string& Filename)
{
	if (!isInitialized())
		return false;

	if (isFontLoaded(Filename))
		return true;

	Font* newFont = new Font(mLibrary);
	if (!newFont->Load(Filename))
	{
		delete newFont;
		return false;
	}
	mFontList[Filename] = newFont;
	return true;
}

bool FontManager::isFontLoaded(const std::string& Filename)
{
	return findFont(Filename) != mFontList.end();
}

void FontManager::unloadFont(const std::string& Filename)
{
	if (!isInitialized())
		return;

	FontListIterator itFont = mFontList.begin();
	for (; itFont != mFontList.end(); ++itFont)
	{
		Font* theFont = itFont->second;
		if (theFont->getFilename() == Filename)
		{
			mFontList.erase(itFont);
			theFont->Unload();
			delete theFont;
		}
	}
}

void FontManager::unloadFonts()
{
	if (!isInitialized())
		return;

	while (!mFontList.empty())
	{
		Font* theFont = mFontList.begin()->second;
		mFontList.erase(mFontList.begin());
		theFont->Unload();
		delete theFont;
	}
}

Font* FontManager::getFont(const std::string& Filename)
{
	if (!isInitialized() || !isFontLoaded(Filename))
		return NULL;

	return findFont(Filename)->second;
}

/************************************************************************/
/*                               Private                                */
/************************************************************************/
FontManager::FontListIterator FontManager::findFont(const std::string& Filename)
{
	return mFontList.find(Filename);
}

}
