#include <Windows.h>
#include <hge.h>
#include "hgeTTF/FontManager.h"
#include "hgeTTF/Font.h"
#include <cassert>

static const int ScreenWidth = 800;
static const int ScreenHeight = 600;
static const unsigned char ScreenBpp = 32;
static const char* WindowTitle = "hgeTTF Test";
static const char* LogFile = "Log.txt";

HGE* hge = hgeCreate(HGE_VERSION);
hgeTTF::FontManager FontMgr;
hgeTTF::Font* Font = NULL;
float Rotation = 0;

bool Update();
bool Render();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hge->System_SetState(HGE_FRAMEFUNC, Update);
	hge->System_SetState(HGE_RENDERFUNC, Render);
	hge->System_SetState(HGE_WINDOWED, false);
	hge->System_SetState(HGE_SCREENWIDTH, ScreenWidth);
	hge->System_SetState(HGE_SCREENHEIGHT, ScreenHeight);
	hge->System_SetState(HGE_SCREENBPP, ScreenBpp);
	hge->System_SetState(HGE_TITLE, WindowTitle);
	hge->System_SetState(HGE_SHOWSPLASH, false);
	hge->System_SetState(HGE_LOGFILE, LogFile);
	hge->System_SetState(HGE_USESOUND, false);
	hge->System_SetState(HGE_HIDEMOUSE, false);
	hge->System_SetState(HGE_FPS, HGEFPS_UNLIMITED);

	if (!hge->System_Initiate())
		return false;

	if (!FontMgr.Initialize())
		return 1;

	if (!FontMgr.loadFont("monafont-ttf-2.90/mona.ttf"))
		return 2;

	Font = FontMgr.getFont("monafont-ttf-2.90/mona.ttf");
	Font->setSize(32);
	if (!Font->cacheChars('a', 'z'))
		return 3;

	if (!Font->cacheChars(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
		return 4;

	hge->System_Start();
	hge->Release();
	return 0;
}

bool Update()
{
	hgeInputEvent Event;
	while (hge->Input_GetEvent(&Event))
	{
		switch (Event.type)
		{
		case INPUT_KEYDOWN:
			switch (Event.key)
			{
			case HGEK_ESCAPE:
				return true;
			case HGEK_K:
				//Toggle kerning
				Font->useKerning(!Font->usingKerning());
				break;
			}
			break;
		}
	}
	Rotation += 5.0f * hge->Timer_GetDelta();
	return false;
}

bool Render()
{
	hge->Gfx_BeginScene();
	hge->Gfx_Clear(0);

	Font->setRotation(0.0f);
	Font->setColor(0xffffffff);
	Font->setSize(22);
	Font->printf(5, 30, L"FPS: %d", hge->Timer_GetFPS());
	float mouseX = 0.0f, mouseY = 0.0f;
	hge->Input_GetMousePos(&mouseX, &mouseY);
	Font->printf(5, 60, L"%d x %d", (int)mouseX, (int)mouseY);

	Font->setColor(0xffffff00);
	Font->setSize(42);
	Font->drawString(L"\u65e5\u672c\u8a9e", 400, 200);
	hge->Gfx_RenderLine(400, 210, 400 + Font->getStringWidth(L"\u65e5\u672c\u8a9e"), 210);

	Font->setSize(22);
	Font->printf(300, 50, L"%s", L"printf");
	hge->Gfx_RenderLine(300, 55, 300 + Font->getStringWidth(L"printf"), 55);

	Font->setColor(0xffcc0000);
	Font->drawString(L"Multi\nLine", 150, 200);

	Font->setColor(0xffff0000, 1);
	Font->setColor(0xffff0000, 2);
	Font->setColor(0xff00ff00, 3);
	Font->setColor(0xff00ff00, 4);
	Font->drawString(L"Horizontal\t Tab", 50, 400);
	Font->drawString(L"Vertical\v Tab", 50, 450);

	Font->setSize(32);
	Font->setColor(0xff00ff00);
	Font->setRotation(Rotation);
	Font->drawString(L"Rotation", 500, 400);

	hge->Gfx_EndScene();
	return false;
}
