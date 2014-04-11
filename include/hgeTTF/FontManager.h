#ifndef HGETTF_FONT_MANAGER_H
#define HGETTF_FONT_MANAGER_H

#include <string>
#include <map>
#include "hgeTTF/Font.h"

namespace hgeTTF
{

class FontManager
{
public:
	FontManager();
	~FontManager();

	bool Initialize();
	bool isInitialized() {return mInitialized;}

	bool loadFont(const std::string& Filename);
	bool isFontLoaded(const std::string& Filename);
	void unloadFont(const std::string& Filename);
	void unloadFonts();
	Font* getFont(const std::string& Filename);

private:
	typedef std::map<const std::string, Font*> FontList;
	typedef FontList::iterator FontListIterator;

	FontManager(const FontManager&);
	const FontManager& operator=(const FontManager&);
	FontListIterator findFont(const std::string& Filename);

	FT_Library mLibrary;
	bool mInitialized;
	FontList mFontList;
};

}

#endif
