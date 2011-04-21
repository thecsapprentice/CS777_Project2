#ifndef WORLDWINDOW_H
#define WORLDWINDOW_H

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class WorldBuilderWindow : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    WorldBuilderWindow ();

    virtual bool OnInitialize ();
    virtual void OnTerminate ();
    virtual void OnIdle ();

protected:
    virtual bool MoveCamera();
    
    void GetHandDeltas();

    void CreateScene ();

    Float2 mHandDists;
    Culler mCuller;
    WireStatePtr mWireState;
    TriStripPtr mStrip;
    TriMeshPtr mStripMesh;
    TriFanPtr mFan;
    TriMeshPtr mFanMesh;
    Triangles* mActive;
    NodePtr mScene;
    Float4 mTextColor;
};


#endif
