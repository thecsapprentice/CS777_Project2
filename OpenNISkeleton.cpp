#include "OpenNISkeleton.h"

OpenNISkeleton* OpenNISkeleton::myInstance = NULL;

OpenNISkeleton* OpenNISkeleton::getInstance()
{
    if(  myInstance == NULL )
        {
            myInstance = new OpenNISkeleton();
        }
    return myInstance;
}

void OpenNISkeleton::tearDown()
{
    if(  myInstance != NULL )
        {
            delete myInstance;
            myInstance = NULL;
        }
} 

OpenNISkeleton::OpenNISkeleton()
{

    g_bNeedPose = FALSE;
    g_strPose = new XnChar[20];

	XnStatus nRetVal = XN_STATUS_OK;

    xn::EnumerationErrors errors;
    nRetVal = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
		{
			XnChar strError[1024];
			errors.ToString(strError, 1024);
			printf("%s\n", strError);
			throw (nRetVal);
		}
    else if (nRetVal != XN_STATUS_OK)
		{
			printf("Open failed: %s\n", xnGetStatusString(nRetVal));
			throw (nRetVal);
		}
	

	nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator");
	nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = g_UserGenerator.Create(g_Context);
		CHECK_RC(nRetVal, "Find user generator");
	}

	if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
	{
		printf("Supplied user generator doesn't support skeleton\n");
		throw 1;
	}

	g_UserGenerator.RegisterUserCallbacks(User_NewUserW,
                                          User_LostUserW,
                                          NULL,
                                          hUserCallbacks);

	g_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStartW,
                                                                  UserCalibration_CalibrationEndW,
                                                                  NULL,
                                                                  hCalibrationCallbacks);

	if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		g_bNeedPose = TRUE;
		if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			printf("Pose required, but not supported\n");
			throw 1;
		}
		g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetectedW,
                                                                      NULL,
                                                                      NULL,
                                                                      hPoseCallbacks);
		g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
	}

	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

}

OpenNISkeleton::~OpenNISkeleton()
{
    g_Context.Shutdown();

    if( g_strPose )
        delete [] g_strPose;

}


void OpenNISkeleton::startGenerating()
{
    XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = g_Context.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating");
}


void XN_CALLBACK_TYPE OpenNISkeleton::User_NewUserW(xn::UserGenerator& generator,
                                           XnUserID nId,
                                           void* pCookie)
{
    if( myInstance )
        myInstance->User_NewUser( generator, nId, pCookie );

}

void XN_CALLBACK_TYPE OpenNISkeleton::User_LostUserW(xn::UserGenerator& generator,
                                            XnUserID nId,
                                            void* pCookie)
{
    if( myInstance )
        myInstance->User_LostUser( generator, nId, pCookie );

}

void XN_CALLBACK_TYPE OpenNISkeleton::UserPose_PoseDetectedW(xn::PoseDetectionCapability& capability,
                                                    const XnChar* strPose,
                                                    XnUserID nId,
                                                    void* pCookie)
{
    if( myInstance )
        myInstance->UserPose_PoseDetected( capability, strPose, nId, pCookie );


}

void XN_CALLBACK_TYPE OpenNISkeleton::UserCalibration_CalibrationStartW(xn::SkeletonCapability& capability,
                                                               XnUserID nId,
                                                           void* pCookie)
{
    if( myInstance )
        myInstance->UserCalibration_CalibrationStart( capability, nId, pCookie );


}

void XN_CALLBACK_TYPE OpenNISkeleton::UserCalibration_CalibrationEndW(xn::SkeletonCapability& capability,
                                                             XnUserID nId,
                                                             XnBool bSuccess,
                                                             void* pCookie)
{
    if( myInstance )
        myInstance->UserCalibration_CalibrationEnd( capability, nId, bSuccess,  pCookie );



}





// Callback: New user was detected
void XN_CALLBACK_TYPE OpenNISkeleton::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	printf("New User %d\n", nId);
	// New user found
	if (g_bNeedPose)
	{
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
	}
	else
	{
		g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE OpenNISkeleton::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	printf("Lost user %d\n", nId);
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE OpenNISkeleton::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	printf("Pose %s detected for user %d\n", strPose, nId);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE OpenNISkeleton::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	printf("Calibration started for user %d\n", nId);
}
// Callback: Finished calibration
void XN_CALLBACK_TYPE OpenNISkeleton::UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	if (bSuccess)
	{
		// Calibration succeeded
		printf("Calibration complete, start tracking user %d\n", nId);
		g_UserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		// Calibration failed
		printf("Calibration failed for user %d\n", nId);
		if (g_bNeedPose)
		{
			g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
		}
		else
		{
			g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

// #define XN_CALIBRATION_FILE_NAME "UserCalibration.bin"

// // Save calibration to file
// void OpenNISkeleton::SaveCalibration()
// {
// 	XnUserID aUserIDs[20] = {0};
// 	XnUInt16 nUsers = 20;
// 	g_UserGenerator.GetUsers(aUserIDs, nUsers);
// 	for (int i = 0; i < nUsers; ++i)
// 	{
// 		// Find a user who is already calibrated
// 		if (g_UserGenerator.GetSkeletonCap().IsCalibrated(aUserIDs[i]))
// 		{
// 			// Save user's calibration to file
// 			g_UserGenerator.GetSkeletonCap().SaveCalibrationDataToFile(aUserIDs[i], XN_CALIBRATION_FILE_NAME);
// 			break;
// 		}
// 	}
// }
// // Load calibration from file
// void OpenNISkeleton::LoadCalibration()
// {
// 	XnUserID aUserIDs[20] = {0};
// 	XnUInt16 nUsers = 20;
// 	g_UserGenerator.GetUsers(aUserIDs, nUsers);
// 	for (int i = 0; i < nUsers; ++i)
// 	{
// 		// Find a user who isn't calibrated or currently in pose
// 		if (g_UserGenerator.GetSkeletonCap().IsCalibrated(aUserIDs[i])) continue;
// 		if (g_UserGenerator.GetSkeletonCap().IsCalibrating(aUserIDs[i])) continue;

// 		// Load user's calibration from file
// 		XnStatus rc = g_UserGenerator.GetSkeletonCap().LoadCalibrationDataFromFile(aUserIDs[i], XN_CALIBRATION_FILE_NAME);
// 		if (rc == XN_STATUS_OK)
// 		{
// 			// Make sure state is coherent
// 			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUserIDs[i]);
// 			g_UserGenerator.GetSkeletonCap().StartTracking(aUserIDs[i]);
// 		}
// 		break;
// 	}
// }
