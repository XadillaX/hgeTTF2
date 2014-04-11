# hgeTTF2

HGE TTF supported. It's not coded by me. Someone seems not support it any more, so I hold it on GITHUB.

You can find the origin [here](http://relishgames.com/forum/index.php?p=/discussion/3860/).

## Description

hgeTTF v2.0 has improved greatly since v1.0.

Bugs have been fixed.

Performance has been improved a significant amount.

hgeTTF turned into sfmlTTF when I switched to SFML.

But I ported it back to HGE for you HGEers.

You do not need anything except hge+hgehelp to use the library.

It is already linked with FreeType2.

## Changes

Everything is now in the hgeTTF namespace.

No more singletons.

## Features

+ UNICODE support (via wstring/wchar_t)
+ Uses FreeType2
+ Uses hgeSprite for rotations, coloring, etc
+ Uses kerning for fonts that support it
+ Very easy to use
+ Extremely fast
+ Cache system

## Usage

Add the include/lib directories to your project.

+ Includes: hgeTTF/include
+ Libs: hgeTTF/lib
+ Link with the library: hgeTTF_Debug.lib or hgeTTF_Release.lib

In your code you'll need to:

1. Init hgeTTF::FontManager
2. Load the font through the manager
3. Draw

Here's an example:

```cpp
hgeTTF::FontManager FontMgr;
FontMgr.Initialize();
FontMgr.loadFont("Font.ttf");
hgeTTF::Font* Font = FontMgr.getFont("Font.ttf");
Font->setSize(22);
Font->drawString(L"Test", 10, 50);
Font->printf(100, 100, L".2f", 2.0f);
```

First you need a `hgeTTF::FontManager`.

You must initialize this manager with `Initialize()`. If it is already initialized, this has no effect.

Then you use the manager to load a font - `loadFont(Filename)`.

Then get the font from the manager using the filename - `getFont(Filename)`.

Here we didn't check any return values, just to keep things simple.

## Docs

There's no real documentation but you don't really need any.

Just view the header files.

Or hopefully you have some form of code completion (like VisualAssist).

## Notes

+ setHotSpot is relative. So 0.5,0.5 is the center.
+ Compiled under VS2008 and includes those project files.

## Author

[dewyatt](http://relishgames.com/forum/index.php?p=/profile/4903/dewyatt) @ HGE Forum
