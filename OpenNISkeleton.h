#ifndef OPEN_NI_SKELETON_H_
#define OPEN_NI_SKELETON_H_


#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>

#define SAMPLE_XML_PATH "Config.xml"

#define CHECK_RC(nRetVal, what)										\
	if (nRetVal != XN_STATUS_OK)									\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));\
		throw nRetVal;												\
	}


class OpenNISkeleton
{

 public:
    
    static OpenNISkeleton* getInstance();
    static void tearDown();
    void startGenerating();

    xn::Context g_Context;
    xn::DepthGenerator g_DepthGenerator;
    xn::UserGenerator g_UserGenerator;
    
 private:

    OpenNISkeleton();
    ~OpenNISkeleton();

    static OpenNISkeleton* myInstance;
  
    XnBool g_bNeedPose;
    XnChar* g_strPose;

    XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;


    static void XN_CALLBACK_TYPE User_NewUserW(xn::UserGenerator& generator,
                                       XnUserID nId,
                                       void* pCookie);
    
    static void XN_CALLBACK_TYPE User_LostUserW(xn::UserGenerator& generator,
                                        XnUserID nId,
                                        void* pCookie);
    
    static void XN_CALLBACK_TYPE UserPose_PoseDetectedW(xn::PoseDetectionCapability& capability,
                                                const XnChar* strPose,
                                                XnUserID nId,
                                                void* pCookie);
    
    static void XN_CALLBACK_TYPE UserCalibration_CalibrationStartW(xn::SkeletonCapability& capability,
                                                           XnUserID nId,
                                                           void* pCookie);

    static void XN_CALLBACK_TYPE UserCalibration_CalibrationEndW(xn::SkeletonCapability& capability,
                                                         XnUserID nId,
                                                         XnBool bSuccess,
                                                         void* pCookie);

    void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator,
                                       XnUserID nId,
                                       void* pCookie);
    
    void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator,
                                        XnUserID nId,
                                        void* pCookie);
    
    void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability,
                                                const XnChar* strPose,
                                                XnUserID nId,
                                                void* pCookie);
    
    void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability,
                                                           XnUserID nId,
                                                           void* pCookie);

    void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability,
                                                         XnUserID nId,
                                                         XnBool bSuccess,
                                                         void* pCookie);


};


#endif
