#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

//fuctiong to get 2DHE operationed imagevoid cal_2DHE(Mat img, Mat dst);

//parameters
int w = 13;  //size of mask

//definitions
const int K = 256;  //8-bit

//image name
#define IMAGE_NAME "plane.tiff"

void  main()
{
	//input image
	Mat input_rgb = imread(IMAGE_NAME);
	Mat input(size(input_rgb), CV_8UC1);
	cvtColor(input_rgb, input, CV_RGB2GRAY);
	namedWindow("input image",WINDOW_AUTOSIZE);
	imshow("input image",input);

	//output image
	Mat output(size(input), CV_8UC1);
	cal_2DHE(input, output);
	namedWindow("output image", WINDOW_AUTOSIZE);
	imshow("output image", output);

	waitKey(0);
}

void cal_2DHE(Mat img, Mat dst)
{
	//input image size
	int row = img.rows;
	int col = img.cols;

	//initialize histogram
	double his[K][K] = { 0 };
	
	////////////////////////////  get 2D histogram  ///////////////////////////////
	int xm, xn;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{	
			//central pixel
			xm = img.at<uchar>(i, j);
			//if (xm == 20)
			//	cout<<"stop"<<endl;
			if (i - (w - 1) / 2 < 0 || j - (w - 1) / 2 < 0 || i + (w + 1) / 2 >= K || j + (w + 1) / 2 >= K)
				continue;
			else
			{
				for (int k = -(w - 1) / 2; k < (w + 1) / 2; k++)
				{
					for (int l = -(w - 1) / 2; l < (w + 1) / 2; l++)
					{
						//neighborhood pixel
						//if (i + k >= 0 && j + l >= 0 && i + k < K && j + l < K)
						{
							xn = img.at<uchar>(i + k, j + l);
							his[xm][xn] += abs(xm - xn) + 1;
						}
					}
				}
			}			
		}
	}
	//Mat Hist (256, 256, CV_32FC1, &his);
	//Mat Hist_Norm;
	//normalize(Hist, Hist_Norm, 128, 255);
	//Hist_Norm.convertTo(Hist_Norm, CV_8UC1);

	//cout << "The calculated 2D histogram " << Hist << endl;
	//imshow("2D histogram", Hist_Norm);
	//waitKey(0);

	///////////////////////  PDF of input image and save to excel  ///////////////////////
	double in_pdf[K] = { 0 };
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			in_pdf[int(img.at<uchar>(i,j))] ++;
		}
	}
	ofstream ExcelFile_InputPDF;
	ExcelFile_InputPDF.open("InputPDF.csv");
	for (int i = 0; i < K; i++)
	{
		in_pdf[i] = in_pdf[i] / (row*col);
		ExcelFile_InputPDF << in_pdf[i] << endl;
	}
	ExcelFile_InputPDF.close();

	///////////////////////  PDF of normalizing 2D histogram  ///////////////////////
	double sum = 0;
	for (int i = 0; i < K; i++)
	{
		for (int j = 0; j < K; j++)
		{
			sum += his[i][j];
		}
	}
	for (int i = 0; i < K; i++)
	{
		for (int j = 0; j < K; j++)
		{
			his[i][j] = his[i][j] / sum;
		}
	}

	///////////////////////  save 2D histogram to excel file  //////////////////////
	ofstream ExcelFile_2Dhistogram;
	ExcelFile_2Dhistogram.open("2D histogram.csv");
	//double rho = 1e-6;
	for (int i = 0; i < K; i++)
	{
		for (int j = 0; j < K; j++)
		{
			double temp;
			if (his[i][j] == 0)
				temp = -20;
			else
				temp = log(his[i][j]/*+rho*/);
			ExcelFile_2Dhistogram << temp << ",";
		}
		ExcelFile_2Dhistogram << endl;
	}
	ExcelFile_2Dhistogram.close();

	//////////////////////////  1D PDF and save to excel  //////////////////////////
	double pdf[K] = { 0 };
	for (int i = 0; i < K; i++)
	{
		for (int j = 0; j < K; j++)
		{
			pdf[i] += his[i][j];
		}
	}
	ofstream ExcelFile_PDF;
	ExcelFile_PDF.open("PDF.csv");
	for (int i = 0; i < K; i++)
	{
		ExcelFile_PDF << pdf[i] << endl;
	}
	ExcelFile_PDF.close();

	////////////////////////////////////  CDF  ////////////////////////////////////
	double cdf[K] = { 0 };
	double cdf_c[K] = { 0 };
	for (int i = 0; i < K; i++)
	{
		for (int j = 0; j < K; j++)
		{
			cdf_c[i] += his[i][j];
		}
		if (i == 0)
		{
			cdf[i] = cdf_c[i];
		}
		else
		{
			cdf[i] = cdf_c[i];
			cdf[i] += cdf[i - 1];
		}
	}	

	///////////////////  save CDF of input image to excel file  //////////////////
	ofstream ExcelFile_CDF;
	ExcelFile_CDF.open("CDF.csv");
	for (int i = 0; i < K; i++)
	{
		ExcelFile_CDF << cdf[i] << endl;
	}
	ExcelFile_CDF.close();

	//uniform histogram and its CDF
	double u_cdf[K] = { 0 };
	for (int i = 0; i < K; i++)
	{
		u_cdf[i] = (i + 1) /double(K);
	}

	////////////////////////////////  optimization  /////////////////////////////
	int trans[K] = { 0 };
	//int temp_m;
	//double temp_CDF;
	//for (int m = 0; m < K; m++)
	//{
	//	temp_m = 0;
	//	temp_CDF = abs(cdf[m] - u_cdf[0]);
	//	for (int i = 1; i < K; i++)
	//	{
	//		//find argument minimum of the difference between CDF of input and uniform 
	//		if (temp_CDF >= abs(cdf[m] - u_cdf[i]))
	//		{
	//			temp_CDF = abs(cdf[m] - u_cdf[i]);
	//			temp_m = i;
	//			continue;
	//		}
	//		else
	//			continue;
	//	}
	//	trans[m] = temp_m;
	//}
	for (int m = 0; m < K; m++)
	{
		trans[m] = floor(cdf[m]* (K-1));
		cout << "cdf[m]*double(K) is: " << cdf[m] * double(K) << endl;
		cout << "trans[m]: " << trans[m] << endl;
	}

	//////////////////  save transform function to excel file  //////////////////
	ofstream ExcelFile_trans;
	ExcelFile_trans.open("transform.csv");
	for (int i = 0; i < K; i++)
	{
		ExcelFile_trans << trans[i] << endl;
	}
	ExcelFile_trans.close();

	/////////////////////////  reconstruction image  ///////////////////////////
	int temp1;
	int temp2;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			temp1 = img.at<uchar>(i, j);
			temp2 = trans[temp1];
			dst.at<uchar>(i, j) = temp2;
		}
	}

	////////////////  get PDF of resutl image and save to excel  //////////////
	double out_pdf[K] = { 0 };
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			out_pdf[int(dst.at<uchar>(i, j))]++;
		}
	}
	for (int i = 0; i < K; i++)
	{
		out_pdf[i] = out_pdf[i] / (row*col);
	}
	ofstream ExcelFile_OutputPDF;
	ExcelFile_OutputPDF.open("OutputPDF.csv");
	for (int i = 0; i < K; i++)
	{
		ExcelFile_OutputPDF << out_pdf[i] << endl;
	}
	ExcelFile_OutputPDF.close();

}