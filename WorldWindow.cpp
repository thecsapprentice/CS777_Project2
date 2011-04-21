#include "WorldWindow.h"

#include "OpenNISkeleton.h"

WM5_WINDOW_APPLICATION(WorldBuilderWindow);

//----------------------------------------------------------------------------

WorldBuilderWindow::WorldBuilderWindow ()
    :
    WindowApplication3("World Builder",
                       0,
                       0,
                       640,
                       480,
                       Float4(1.0f, 1.0f, 1.0f, 1.0f)
                       ),
    mTextColor(0.0f, 0.0f, 0.0f ,1.0f),
    mHandDists(-1.0f, -1.0f )
{
}

//---------------------------------------------------------------------------

bool WorldBuilderWindow::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

    OpenNISkeleton* oSkel = OpenNISkeleton::getInstance();
    oSkel->startGenerating();

    // Set up the camera.
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    APoint camPosition(0.0f, -1.0f, 0.1f);
    AVector camDVector(0.0f, 1.0f, 0.0f);
    AVector camUVector(0.0f, 0.0f, 1.0f);
    AVector camRVector = camDVector.Cross(camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    CreateScene();

    // Initial update of objects.
    mScene->Update();

    // Initial culling of scene.
    mCuller.SetCamera(mCamera);
    mCuller.ComputeVisibleSet(mScene);

    InitializeCameraMotion(0.001f, 0.001f);
    InitializeObjectMotion(mScene);
    return true;
}

//---------------------------------------------------------------------------

void WorldBuilderWindow::OnTerminate ()
{
    mScene = 0;

    WindowApplication3::OnTerminate();
}

//---------------------------------------------------------------------------

void WorldBuilderWindow::OnIdle ()
{
    MeasureTime();

    OpenNISkeleton* oSkel = OpenNISkeleton::getInstance();
    oSkel->g_Context.WaitOneUpdateAll(oSkel->g_DepthGenerator);


    GetHandDeltas();

    if (MoveCamera())
    {
        mCuller.ComputeVisibleSet(mScene);
    }

    if (MoveObject())
    {
        mScene->Update();
        mCuller.ComputeVisibleSet(mScene);
    }

    if (mRenderer->PreDraw())
    {
        mRenderer->ClearBuffers();
        mRenderer->Draw(mCuller.GetVisibleSet());
        DrawFrameRate(8, GetHeight()-8, mTextColor);
        mRenderer->PostDraw();
        mRenderer->DisplayColorBuffer();
    }

    UpdateFrameCount();
}

//----------------------------------------------------------------------------

void WorldBuilderWindow::GetHandDeltas()
{
    OpenNISkeleton* skel = OpenNISkeleton::getInstance();

    if( mHandDists[0] < 0 )
        mHandDists[0] = skel->getHandDist();
    else
        if( mHandDists[1] < 0 )
            mHandDists[1] = skel->getHandDist();
        else
            {
                mHandDists[0] = mHandDists[1];
                mHandDists[1] = skel->getHandDist();
            }
}


//----------------------------------------------------------------------------

bool WorldBuilderWindow::MoveCamera ()
{
    if (!mCameraMoveable)
    {
        return false;
    }

    bool bMoved = false;
    float handDelta = 0.0f;

    if( mHandDists[0] > 0 && mHandDists[1] > 0 )
        {
            handDelta = mHandDists[1] - mHandDists[0];
            if(  handDelta > 0 )
                {
                    MoveForward();
                    bMoved = true;
                }
            else if( handDelta < 0 )
                {
                    MoveBackward();
                    bMoved = true;
                }
        }
            

    if (mUArrowPressed)
    {
        MoveForward();
        bMoved = true;
    }

    if (mDArrowPressed)
    {
        MoveBackward();
        bMoved = true;
    }

    if (mHomePressed)
    {
        MoveUp();
        bMoved = true;
    }

    if (mEndPressed)
    {
        MoveDown();
        bMoved = true;
    }

    if (mLArrowPressed)
    {
        TurnLeft();
        bMoved = true;
    }

    if (mRArrowPressed)
    {
        TurnRight();
        bMoved = true;
    }

    if (mPgUpPressed)
    {
        LookUp();
        bMoved = true;
    }

    if (mPgDnPressed)
    {
        LookDown();
        bMoved = true;
    }

    if (mInsertPressed)
    {
        MoveRight();
        bMoved = true;
    }

    if (mDeletePressed)
    {
        MoveLeft();
        bMoved = true;
    }

    return bMoved;
}



//----------------------------------------------------------------------------

void WorldBuilderWindow::CreateScene ()
{
 
   mScene = new0 Node();
    mWireState = new0 WireState();
    mRenderer->SetOverrideWireState(mWireState);

    // Texture effect shared by all objects.
    Texture2DEffect* effect = new0 Texture2DEffect(Shader::SF_LINEAR);
    std::string path = Environment::GetPathR("Horizontal.wmtf");
    Texture2D* texture = Texture2D::LoadWMTF(path);

    // Create the vertices and texture coordinates for the cube tristrip
    // and the trimesh corresponding to it.
    int sideDivisions = 0;
    int sideNumVertices = 2*(2 + sideDivisions);
    int numVertices = 6*sideNumVertices;
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
    int vstride = vformat->GetStride();

    VertexBuffer* vbuffer = new0 VertexBuffer(numVertices, vstride);
    VertexBufferAccessor vba(vformat, vbuffer);

    const float scale = 2.0f/3.0f;
    int i, side;
    for (side = 0; side < 6; ++side)
    {
        // Create initial corner vertices for this side of the cube.
        Vector3f cornerVertex[4];
        Vector2f cornerUV[4];
        int axis = side >> 1;
        float b2 = ((side & 1) ? +1.0f : -1.0f);
        for (i = 0; i < 4; ++i)
        {
            float b0 = ((i & 1) ? +1.0f : -1.0f);
            float b1 = ((i & 2) ? +1.0f : -1.0f);

            switch (axis)
            {
            case 0:
                cornerVertex[i] = Vector3f(b2, -b0, -b1*b2)*scale;
                break;
            case 1:
                cornerVertex[i] = Vector3f(b1*b2, b2, b0)*scale;
                break;
            case 2:
                cornerVertex[i] = Vector3f(b1*b2, -b0, b2)*scale;
                break;
            }
            cornerUV[i][0] = (float)(1 - ((i >> 1) & 1));
            cornerUV[i][1] = (float)(1 - (i & 1));
        }

        // Interpolate between the corner vertices.
        int offset = side*sideNumVertices;
        for (i = 0; i < sideNumVertices; ++i)
        {
            int parity = (i & 1);
            float interpolation = (float)(i >> 1)/(float)(1+sideDivisions);
            int index = i + offset;
            assertion(index < numVertices, "Unexpected condition.\n");
            vba.Position<Vector3f>(index) = cornerVertex[parity] *
                (1.0f - interpolation) + cornerVertex[2 + parity] *
                interpolation;
            vba.TCoord<Vector2f>(0, index) = cornerUV[parity] *
                (1.0f-interpolation) + cornerUV[2 + parity]*interpolation;
        }
    }

    // Generate tristrip indices for the cube [T = 6*(sideNumVertices-2)].
    int numIndices = 6*sideNumVertices + 10;
    IndexBuffer* ibuffer = new0 IndexBuffer(numIndices, sizeof(int));
    int* indices = (int*)ibuffer->GetData();
    for (side = 0; side < 6; ++side)
    {
        // Tristrip indices for the side.
        for (i = 0; i < sideNumVertices; ++i)
        {
            *indices++ = side*sideNumVertices + i;
        }

        // Join the next side by replication using the last vertex index from
        // the current side and the first vertex index from the next side.
        if (side < 5)
        {
            *indices++ = (side + 1)*sideNumVertices - 1;
            *indices++ = (side + 1)*sideNumVertices;
        }
    }

    mStrip = new0 TriStrip(vformat, vbuffer, ibuffer);
    mStrip->SetEffectInstance(effect->CreateInstance(texture));
    mStrip->Culling = Spatial::CULL_DYNAMIC;
    mScene->AttachChild(mStrip);

    mActive = mStrip;

}
