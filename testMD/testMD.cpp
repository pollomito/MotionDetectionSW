// testMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

#include <opencv2/opencv.hpp>
//#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
//#include <unistd.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>

using namespace cv;
using namespace std;




int main(int argc, _TCHAR* argv[])
{


	TCHAR szDir[MAX_PATH];
	int xstart = 0, xend = 0, ystart = 0, yend = 0, fps = 0;

	if((argc <= 1) || (argv[1] == "-h"))
	{
		cout << "Usage : -h  Helo" << endl;
		cout << "      : folder [xstart xend ystart yend]" << endl;
		cout << "      : xstart xend ystart yend in px or end for full size" << endl;
		cout << " Example: C:\\Clips 1500 end 200 500" << endl;
		return 0;
	}


	string filedir = argv[1];
	if(filedir.back() != '\\')
	{
		filedir += "\\";
		StringCchCopy(szDir, MAX_PATH, argv[1]);
		StringCchCat(szDir, _countof(szDir), TEXT("\\*"));
	}
	else
	{
		StringCchCopy(szDir, MAX_PATH, argv[1]);
		StringCchCat(szDir, _countof(szDir), TEXT("*"));
	}

	if (argc == 6)
	{
		xstart = atoi(argv[2]);
		if(argv[3] != "end")
			xend = atoi(argv[3]);
		ystart = atoi(argv[4]);
		if (argv[5] != "end")
			yend = atoi(argv[5]);
	}

//	StringCchCopy(szDir, MAX_PATH, argv[2]);
//    StringCchCopy(szDir, MAX_PATH, TEXT("C:\\SharedFolder\\Downloads_15_11\\"));


//	StringCchCopy(szDir, MAX_PATH, TEXT("C:\\Users\\s90653\\Desktop\\Clips\\*"));
//	StringCchCopy(szDir, MAX_PATH, TEXT("C:\\SharedFolder\\Downloads_15_11\\*"));
	

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(szDir, &FindFileData);

	/*if(hFind != 0)
	{
		cout << "No files... Press any key to exit..." << hFind << szDir  << endl;
		waitKey();
		return 0;
	}
	*/
	do {
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			_tprintf(TEXT("  %s \n"), FindFileData.cFileName);

			string filename = filedir + FindFileData.cFileName; //(FindFileData.cFileName);





			Mat frame, gray, frameDelta, thresh, firstFrame, lastFrameMoving;
			vector<vector<Point> > cnts;
			VideoCapture camera( filename ); //open camera
			//VideoCapture camera("C:\\Users\\s90653\\Desktop\\Clips\\Big_Buck_Bunny_360_10s_5MB.mp4");
									//set the video size to 512x288 to process faster
		//	camera.set(3, 512);
		//	camera.set(4, 288);

			Sleep(3);
			camera.read(frame);

			cout << " Clip Props : " << endl;
			cout << "     Width  : " << frame.size().width << endl;
			cout << "     Height : " << frame.size().height << endl;
			cout << "     fps    : " << camera.get(CAP_PROP_FPS) << endl;

			fps = camera.get(CAP_PROP_FPS);


			if(xend == 0)
				xend = frame.size().width;
			if(yend == 0)
				yend = frame.size().height;


			frame = frame(Range(ystart, yend), Range(xstart, xend));



			//convert to grayscale and set the first frame
			cvtColor(frame, firstFrame, COLOR_BGR2GRAY);
			GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);
			int frmcnt = 0;



			while (camera.read(frame))
			{
				Rect ScanZone;
				ScanZone.x = xstart;
				ScanZone.y = ystart;
				ScanZone.width = xend - xstart;
				ScanZone.height = yend - ystart;

				lastFrameMoving = frame.clone();
				rectangle(lastFrameMoving, ScanZone, Scalar(0, 0, 255), 2, 8, 0);
				imshow("Clip Under Analyze", lastFrameMoving);
				lastFrameMoving.release();

				frame = frame(Range(ystart, yend), Range(xstart, xend));

				//convert to grayscale
				cvtColor(frame, gray, COLOR_BGR2GRAY);
		//		GaussianBlur(gray, gray, Size(21, 21), 0);
				GaussianBlur(gray, gray, Size(11, 11), 0);

				//compute difference between first frame and current frame
				absdiff(firstFrame, gray, frameDelta);
				threshold(frameDelta, thresh, 25, 255, THRESH_BINARY);

				dilate(thresh, thresh, Mat(), Point(-1, -1), 2);
				findContours(thresh, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

				for (int i = 0; i< cnts.size(); i++) {
					if (contourArea(cnts[i]) < 500) {
						continue;
					}
					if(lastFrameMoving.empty())
					{
						camera.read(lastFrameMoving);
						//lastFrameMoving = lastFrameMoving(Range(0, 500), Range(1300, 1920));
						putText(lastFrameMoving, "Motion.. Key to Continue...", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
					}

					Rect position = boundingRect(cnts[i]);
					position.y = position.y + ystart;
					position.x = position.x + xstart;
					rectangle(lastFrameMoving, position, Scalar(0, 255, 0), 2, 8, 0);
				}

				if(!lastFrameMoving.empty())
				{
					imshow("Diff", thresh);
					imshow("Camera", lastFrameMoving);
					cout << "Motion... Press any key to Continue..." << endl;
					waitKey();
				}

				lastFrameMoving.release();// = Scalar(0, 0, 0);

	//			imshow("old", firstFrame);
	//			imshow("new", gray);

		

				if (waitKey(1) == 27) {
					//exit if ESC is pressed
					break;
				}
				firstFrame = gray.clone();

				frmcnt += fps*2;
				camera.set(CAP_PROP_POS_FRAMES, frmcnt);

			}



		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	cout << "End... Press any key to exit..." << endl;
	waitKey(); 

	return 0;
}
