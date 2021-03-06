#include "Engines.h"

using namespace std;

Engines::Engines()
{
}

Engines::~Engines()
{
}

int Engines::enginesInit()
{
	// 初始化引擎
	MRESULT ret = MERR_UNKNOWN;

	// 初始化FDE
	mFDEngine = new FDEngine();
	ret = mFDEngine->init();
	if (ret != MOK)
	{
		cout << "初始化FD引擎失败,错误码: %d, 程序将关闭！" << endl;
		enginesClose();
		return ret;
	}

	// 初始化FTE
	mFTEngine = new FTEngine();
	ret = mFTEngine->init();
	if (ret != MOK)
	{
		cout << "初始化FT引擎失败,错误码: %d, 程序将关闭！" << endl;
		enginesClose();
		return ret;
	}

	mFREngine = new FREngine();
	ret = mFREngine->init();
	if (ret != MOK)
	{
		cout << "初始化FD引擎失败,错误码: %d, 程序将关闭！" << endl;
		enginesClose();
		return ret;
	}
	mAgeEngine = new AgeEngine();
	ret = mAgeEngine->init();
	if (ret != MOK)
	{
		cout << "初始化FD引擎失败,错误码: %d, 程序将关闭！" << endl;
		enginesClose();
		return ret;
	}
	mGenderEngine = new GenderEngine();
	ret = mGenderEngine->init();
	if (ret != MOK)
	{
		cout << "初始化FD引擎失败,错误码: %d, 程序将关闭！" << endl;
		enginesClose();
		return ret;
	}
	return ret;

}

int Engines::enginesClose()
{
	// 停止运行并释放内存和引擎
	//Stop();
	if (mFDEngine != nullptr)
	{
		mFDEngine->uninit();
		delete mFDEngine;
		mFDEngine = nullptr;
	}
	if (mFREngine != nullptr)
	{
		mFREngine->uninit();
		delete mFREngine;
		mFREngine = nullptr;
	}
	if (mAgeEngine != nullptr)
	{
		mAgeEngine->uninit();
		delete mAgeEngine;
		mAgeEngine = nullptr;
	}
	if (mGenderEngine != nullptr)
	{
		mGenderEngine->uninit();
		delete mGenderEngine;
		mGenderEngine = nullptr;
	}


	exit(0);
	return -1;

}

int Engines::initFaceVariables()
{
	/* engine 视频输入 */
	mOffInput = { 0 };
	mOffInput.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;	//TODO
	mOffInput.ppu8Plane[0] = nullptr;

	/* 人脸检测结果 */
	mFaceRes = nullptr;

	/* gender engine 输入 */
	mGenderFaceInput = { 0 };
	mGenderFaceInput.lFaceNumber = 0;

	/* gender engine 输出 */
	mGenderResult = { 0 };

	/* ageengine 输入 */
	mAgeFaceInput = { 0 };

	/* ageEngine 输出*/
	mAgeResult = { 0 };

	/* FREngine 输入 */
	//mRecognitionFaceInput = 

	/* FREngine 提取的模型 */
	mFRFaceModel = { 0 };

	LocalFaceModels = { 0 };

	videoFaceModels = { 0 };

	return 0;
}

int Engines::initCamera()
{
	mCapture.open(0);
	if (!mCapture.isOpened())
	{
		cout << "摄像头未打开" << endl;
		return -1;
	}

	int CAP_WIDTH = mCapture.get(CV_CAP_PROP_FRAME_WIDTH);
	int CAP_HEIGHT = mCapture.get(CV_CAP_PROP_FRAME_HEIGHT);

	mFrame = new cv::Mat(CAP_HEIGHT, CAP_WIDTH, CV_8UC3);

	return 0;
}

int Engines::closeCamera()
{
	if (!mCapture.isOpened())
	{
		std::cout << "摄像头处于关闭状态" << std::endl;
		return 0;
	}
	mCapture.release();
	return 0;
}

void Engines::releaseCamera()
{
	if (mCapture.isOpened())
	{
		mCapture.release();
	}
}

int Engines::initRet()
{
	faceTrackingRet = MERR_UNKNOWN;
	extractFRFeatureRet = MERR_UNKNOWN;
	return 0;
}

int Engines::cameraToOffInput(cv::VideoCapture* capture,cv::Mat* mFrame, LPASVLOFFSCREEN mOffInput)
{
	
	*capture >> *mFrame;
	mOffInput->u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
	mOffInput->i32Width = mFrame->cols;
	mOffInput->i32Height = mFrame->rows;
	mOffInput->ppu8Plane[0] = mFrame->data;
	mOffInput->pi32Pitch[0] = mOffInput->i32Width * 3;
	return 0;
}

MRESULT Engines::faceTracking(LPASVLOFFSCREEN pImgData, LPAFT_FSDK_FACERES *pFaceRes)
{
	faceTrackingRet = mFTEngine->FaceTracking(pImgData, pFaceRes);
	return faceTrackingRet;
}

MRESULT Engines::faceDetect(LPASVLOFFSCREEN pImgData, LPAFT_FSDK_FACERES * pFaceRes)
{
	faceDetectRet = mFTEngine->FaceTracking(pImgData, pFaceRes);
	return faceDetectRet;
}

void Engines::getGenderFaceInput()
{
	if (faceTrackingRet == MOK)
	{
		mGenderFaceInput.lFaceNumber = mFaceRes->nFace;
		mGenderFaceInput.pFaceRectArray = mFaceRes->rcFace;
		mGenderFaceInput.pFaceOrientArray = &(mFaceRes->lfaceOrient);
	}
}

void Engines::getAgeFaceInput()
{
	if (faceTrackingRet == MOK)
	{
		mAgeFaceInput.lFaceNumber = mFaceRes->nFace;
		mAgeFaceInput.pFaceOrientArray = &mFaceRes->lfaceOrient;
		mAgeFaceInput.pFaceRectArray = mFaceRes->rcFace;
	}
}

void Engines::getFRFaceInput(LPAFT_FSDK_FACERES FaceRes, AFR_FSDK_FACEINPUT* FRFaceInput)
{
	if (faceTrackingRet == MOK)
	{
		FRFaceInput->lOrient = FaceRes->lfaceOrient;
		FRFaceInput->rcFace = *FaceRes->rcFace;
	}
}

MRESULT Engines::extractFRFeature(LPASVLOFFSCREEN pImgData, LPAFR_FSDK_FACEINPUT pFaceRes, LPAFR_FSDK_FACEMODEL pFaceModels)
{
	extractFRFeatureRet = mFREngine->ExtractFRFeature(pImgData, pFaceRes, pFaceModels);
	return extractFRFeatureRet;
}

AFR_FSDK_FACEMODEL Engines::getFaceModelFromBMP(const char * path, ASVLOFFSCREEN* offInput1)
{
	///* 临时变量 获得本地脸部模型*/
	offInput1->u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
	offInput1->ppu8Plane[0] = nullptr;

	readBMP(path, (uint8_t**)&offInput1->ppu8Plane[0], &offInput1->i32Width, &offInput1->i32Height);
	offInput1->pi32Pitch[0] = offInput1->i32Width * 3;
	
	LPAFD_FSDK_FACERES localFaceRes = nullptr;
	AFR_FSDK_FACEINPUT localFRInput;
	mFDEngine->FaceDetection(offInput1, &localFaceRes);
	localFRInput.lOrient = *(localFaceRes->lfaceOrient);
	localFRInput.rcFace = *(localFaceRes->rcFace);	
	
	AFR_FSDK_FACEMODEL localFaceModel = { 0 };
	

	extractFRFeatureRet = mFREngine->ExtractFRFeature(offInput1, &localFRInput, &localFaceModel);
	return localFaceModel;
	//if (extractFRFeatureRet != MOK)
	//{
	//	return extractFRFeatureRet;
	//}
	//LocalFaceModels.lFeatureSize = localFaceModel.lFeatureSize;

	///* 可能引发内存泄漏 */
	//free(LocalFaceModels.pbFeature);
	//LocalFaceModels.pbFeature = (MByte*)malloc(localFaceModel.lFeatureSize);
	//memcpy(LocalFaceModels.pbFeature, localFaceModel.pbFeature, localFaceModel.lFeatureSize);

	//return 0;
}

int Engines::getVideoFaceModel()
{
	if (faceTrackingRet != MOK)
	{
		return -1;
	}

	AFR_FSDK_FACEINPUT videoFRInput;
	videoFRInput.lOrient = mFaceRes->lfaceOrient;
	videoFRInput.rcFace = *(mFaceRes->rcFace);

	AFR_FSDK_FACEMODEL videoFaceModel = { 0 };
	
	extractFRFeatureRet = mFREngine->ExtractFRFeature(&mOffInput, &videoFRInput, &videoFaceModel);
	if (extractFRFeatureRet != MOK)
	{
		return extractFRFeatureRet;
	}

	videoFaceModels.lFeatureSize = videoFaceModel.lFeatureSize;
	
	/* 引发内存泄漏点，未处理*/
	free(videoFaceModels.pbFeature);
	videoFaceModels.pbFeature = (MByte*)malloc(videoFaceModel.lFeatureSize);
	memcpy(videoFaceModels.pbFeature, videoFaceModel.pbFeature, videoFaceModel.lFeatureSize);


	return 0;
}

int Engines::faceRecognition(AFR_FSDK_FACEMODEL *reffeature, AFR_FSDK_FACEMODEL *probefeature, MFloat *pfSimilScore)
{
	facePairMatchingRet =  mFREngine->FacePairMatching(reffeature, probefeature, pfSimilScore);
	return facePairMatchingRet;
}

void Engines::drawFaceRect()
{
	if (faceTrackingRet == MOK)
	{
		for (int i = 0; i < mFaceRes->nFace; ++i)
		{
			//在视频上画出矩形
			cv::rectangle(*mFrame, CvPoint(mFaceRes->rcFace[i].left, mFaceRes->rcFace[i].top), CvPoint(mFaceRes->rcFace[i].right, mFaceRes->rcFace[i].bottom), CvScalar(255, 0, 0), 2);
					
		}
	}
//if (nRet == MOK)
//{
//	//fprintf(stdout, "The number of face: %d\r\n", FaceRes->nFace);
//	for (int i = 0; i < FaceRes->nFace; ++i)
//	{
//		//在视频上画出矩形
//		cv::rectangle(frame, CvPoint(FaceRes->rcFace[i].left, FaceRes->rcFace[i].top), CvPoint(FaceRes->rcFace[i].right, FaceRes->rcFace[i].bottom), CvScalar(255, 0, 0), 2);
//		
//		//在矩形底部打印信息
//		sprintf(age, "%d", ageResult.pAgeResultArray[i]);
//		info = "name:" + name + "sex" + sex[genderResult.pGenderResultArray[i] + 1] + "age" + age;
//		//strcpy(info, "name:%csex:%c\tage:%d", "huanglei", "male", ageResult.pAgeResultArray[i]);
//		cv::putText(frame, info, CvPoint(FaceRes->rcFace[i].left, FaceRes->rcFace[i].bottom), cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 0, 0));
//	}
//}
//else
//{
//	//fprintf(stderr, "Face Detection failed, error code: %d\r\n", nRet);
//}
//
//cv::imshow("video", frame);
}

void Engines::showVideo()
{
	drawFaceRect();
	cv::imshow("video", *mFrame);
}
/*From 王宏博*/
