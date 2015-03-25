#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
using namespace cv;


Mat inputImage,channel[3];
bool isGrayScale=true;
std::string labels[3] = {"Layer Cy3", "Layer Cy5", "Grayscale"};

//RNG rng(12345);


void quantifyLayer(int layer);

int main(int argc, char** argv){
	inputImage = imread(argv[1]); // read colored BGR image
	//imshow("Original Image",inputImage);


	// For Color Image
	if(argc>2){
			isGrayScale=false;
	}

	if(isGrayScale){
		cvtColor(inputImage,inputImage,COLOR_BGR2GRAY);
		quantifyLayer(2);
	}
	else{
		cvtColor(inputImage, inputImage, COLOR_BGR2RGB);
		split(inputImage, channel);
		for(int i=0;i<2;i++){
			quantifyLayer(i);
		}
	}

	waitKey(0);
	return 0;
}


void quantifyLayer(int layer){
	Mat input(inputImage.size(),inputImage.type());
	Mat channel_blur,img_bw;

	if(layer==2)
		inputImage.copyTo(input);
	else
		channel[layer].copyTo(input);


	//****************************** Filter ****************************************
	GaussianBlur(input,channel_blur,Size(11,11),0,0);

	//****************************** Adaptive Threshold ****************************

 	int thresholdWindow=141; // Based on Image width
	adaptiveThreshold(channel_blur, img_bw,255,ADAPTIVE_THRESH_MEAN_C ,THRESH_BINARY_INV,thresholdWindow,2);
 	//imshow("Before Morphy",img_bw);

 	int morph_size = 2;
 	int close_iter=9,open_iter=19;
    Mat kernel = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1,morph_size+1 ), Point( morph_size, morph_size ) );
	morphologyEx(img_bw,img_bw, MORPH_OPEN,kernel,Point(-1,-1),open_iter);
	morphologyEx(img_bw,img_bw, MORPH_CLOSE,kernel,Point(-1,-1),close_iter);
 	//imshow(labels[layer],img_bw);
 	

 	//**************************** Find ROIs ****************************************
 	cv::Mat threshold_output;
	img_bw.convertTo(threshold_output, CV_8U);
	std::vector<std::vector<cv::Point> > contours;

	cv::findContours(threshold_output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// Total objects
	//int ncomp = contours.size();
	//std::cout<<labels[layer]<<" number of detected objects: "<<ncomp<<std::endl;



  	/// Draw polygonal bonding rects on image
  	vector<vector<Point> > contours_poly( contours.size() );
  	vector<Rect> boundRect(contours.size());

  	for( int i = 0; i < contours.size(); i++ )
     { 
       approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
       boundRect[i] = boundingRect(Mat(contours_poly[i]));
     }

  	Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  	for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar(0,0,0);
       // drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       rectangle(input, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
     }
     imshow(labels[layer],input );
     imwrite("output.jpg",input);



 	//**************************** Calculations ****************************************
     std::ofstream fout("output.txt");

	 double *results;
	 results=new double[contours.size()];
     for(int i = 0; i< contours.size(); i++ )
     {
     	Mat roi(input(boundRect[i]));
     	results[i]=sum(roi)[0];
     	fout<<"Region"<<i+1<<" "<<results[i]<<std::endl;
     }

	// First Try
	// cv::threshold(channel_blur, img_bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	// Mat grad_x,abs_grad_x;
	// int scale = 1;
	// int delta = 0;
	// Sobel(channel_blur, grad_x, CV_16S, 1, 0, 5, scale, delta, BORDER_DEFAULT );
	// convertScaleAbs( grad_x, abs_grad_x );
	// imshow("red filter (1-Channels)",abs_grad_x);
	// merge(aux, 3, outputImage);
	// cvtColor(outputImage, outputImage, convertBack);
	// imshow(" (3-Channels)", outputImage);


}
