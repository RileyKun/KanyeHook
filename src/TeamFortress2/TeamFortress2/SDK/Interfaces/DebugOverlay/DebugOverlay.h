#pragma once
#include "../../Includes/Includes.h"

class  OverlayText_t
{
public:
    OverlayText_t()
    {
        nextOverlayText = 0;
 
        bUseOrigin = false;
        lineOffset = 0;
        flXPos = 0;
        flYPos = 0;
        text[0] = 0;
        m_flEndTime = 0.0f;
        m_nServerCount = -1;
        m_nCreationTick = -1;
        r = g = b = a = 255;
    }

    bool            IsDead();
    void            SetEndTime(float duration);

    Vector            origin;
    bool            bUseOrigin;
    int                lineOffset;
    float            flXPos;
    float            flYPos;
    char            text[512];
    float            m_flEndTime;            // When does this text go away
    int                m_nCreationTick;        // If > 0, show only one server frame
    int                m_nServerCount;            // compare server spawn count to remove stale overlays
    int                r;
    int                g;
    int                b;
    int                a;
    OverlayText_t* nextOverlayText;
};

/*
CIVDebugOverlay__AddEntityTextOverlay
CIVDebugOverlay__AddBoxOverlay
CIVDebugOverlay__AddTriangleOverlay
CIVDebugOverlay__AddLineOverlay
CIVDebugOverlay__AddTextOverlay
CIVDebugOverlay__AddTextOverlay_
CIVDebugOverlay__AddScreenTextOverlay
*/


class CDebugOverlay
{
public:
    virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
    virtual void AddBoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
    virtual void AddTriangleOverlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void AddLineOverlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
    virtual void AddTextOverlay(const Vector& origin, float duration, const char* format, ...) = 0;
    virtual void AddTextOverlay(const Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
    virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
    virtual void AddScreenTextOverlayNew() = 0; //added in vscript update
    virtual void AddSweptBoxOverlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
    virtual void AddGridOverlay(const Vector& origin) = 0;
    virtual int ScreenPosition(const Vector& point, Vector& screen) = 0;
    virtual int ScreenPosition(float flXPos, float flYPos, Vector& screen) = 0;
    virtual OverlayText_t* GetFirst(void) = 0;
    virtual OverlayText_t* GetNext(OverlayText_t* current) = 0;
    virtual void ClearDeadOverlays(void) = 0;
    virtual void ClearAllOverlays() = 0;
    virtual void AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
    virtual void AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
    virtual void AddLineOverlayAlpha(const Vector& origin, const Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void AddBoxOverlay2(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, const Color_t& faceColor, const Color_t& edgeColor, float duration) = 0;
};

#define VENGINE_DEBUGOVERLAY_INTERFACE_VERSION   "VDebugOverlay003"