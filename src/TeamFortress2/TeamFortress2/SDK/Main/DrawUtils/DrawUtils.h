#pragma once
#include "../../Interfaces/Interfaces.h"

struct ScreenSize_t
{
	int w = 0, h = 0, c = 0;
	void Update();
};

inline ScreenSize_t g_ScreenSize;

struct Font_t
{
	DWORD dwFont = 0x0;
	const char* szName = 0;
	int nTall = 0;
	int nWeight = 0;
	int nFlags = 0;
};

enum EFonts
{
	FONT_ESP,
	FONT_ESP_NAME,
	FONT_ESP_COND,
	FONT_ESP_PICKUPS,
	FONT_MENU,
	FONT_INDICATORS,
	FONT_IMGUI,
};

enum EStringAlign
{
	ALIGN_DEFAULT,
	ALIGN_CENTER,
	ALIGN_CENTERVERTICAL,
	ALIGN_CENTERHORIZONTAL,
	ALIGN_REVERSE
};

struct Liner
{
	Liner(int nx1, int ny1, int nx2, int ny2, Color_t c) : x1(nx1), y1(ny1), x2(nx2), y2(ny2), cColor(c) {}
	int x1, x2, y1, y2;
	Color_t cColor;
};


struct Draw_t
{
	std::vector<Font_t>   m_vecFonts = { };
	std::unordered_map<uint64, int> m_mapAvatars = { };

	void InitFonts(const std::vector<Font_t>& fonts);
	void RemakeFonts(const std::vector<Font_t>& fonts);
	void ReloadFonts();
	void String(const size_t& font_idx, int x, int y, const Color_t& clr, const EStringAlign& align, const char* str, ...);
	void String(const size_t& font_idx, int x, int y, const Color_t& clr, const EStringAlign& align, const wchar_t* str, ...);
	void Line(int x, int y, int x1, int y1, const Color_t& clr);
	void DrawTexturedPolygon(int count, Vertex_t* vertices, const Color_t& clr);
	void DrawFilledTriangle(std::array<Vec2, 3> points, const Color_t& clr);
	void DrawOutlinedTriangle(std::array<Vec2, 3> points, const Color_t& clr);
	void Rect(int x, int y, int w, int h, const Color_t& clr);
	void RectOverlay(int x, int y, int w, int h, float bwidthp, const Color_t& clr, const Color_t& overlay_clr, bool horizontal);
	void OutlinedRect(int x, int y, int w, int h, const Color_t& clr);
	void GradientRect(int x, int y, int x1, int y1, const Color_t& top_clr, const Color_t& bottom_clr, bool horizontal);
	void GradientRectWH(int x, int y, int w, int h, const Color_t& top_clr, const Color_t& bottom_clr, bool horizontal);
	void OutlinedGradientBar(int x, int y, int w, int h, float bwidthp, const Color_t& top_clr, const Color_t& bottom_clr, const Color_t& overlay_clr, bool horizontal);
	void GradientRectA(int x, int y, int x1, int y1, const Color_t& top_clr, const Color_t& bottom_clr, bool horizontal);
	void OutlinedCircle(int x, int y, float radius, int segments, const Color_t& clr);
	void FilledCircle(const int x, const int y, const int radius, const int segments, const Color_t clr);
	void CornerRect(int x, int y, int w, int h, int _x, int _y, const Color_t& color);
	void Texture(int x, int y, int w, int h, const Color_t& clr, int nIndex);
	CHudTexture* GetIcon(const char* szIcon, int eIconFormat = 0);
	int CreateTextureFromArray(const unsigned char* rgba, int w, int h);
	void DrawHudTexture(float x0, float y0, float s0, CHudTexture* texture, Color_t col0);
	void DrawHudTextureByName(float x0, float y0, float s0, const char* textureName, Color_t col0);
	void Avatar(const int x, const int y, const int w, const int h, const uint32 nFriendID);
	void RoundedBoxStatic(const int x, const int y, const int w, const int h, const int radius, const Color_t& col);
	float EaseOut(float start, float end, float speed);
	float EaseIn(float start, float end, float speed);
	float Linear(float start, float end, float speed);
	bool Timer();
	void ClearAvatarCache();

	// If you want to apply an outline to a bunch of drawn lines without overlapping
	void BatchDrawLines(std::vector<Liner>* Lines);
};



inline Draw_t g_Draw;