#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>

#include "codecs/codecs.h"

using namespace cv;
using namespace std;

/*
unsigned short * convert24bitTo16bit(void * input24bit,unsigned int width , unsigned int height)
{
  unsigned short * output = (unsigned short * )  malloc ( sizeof(unsigned short)  * width * height  );
  if (output == 0 ) { fprintf(stderr,"Could not allocate output for convert24bitTo16bit\n"); return 0; }

  unsigned char * traverser=(unsigned char * ) input24bit;
  unsigned short * casterUshort=0;
  unsigned int *   casterUint=0;
  unsigned short * outputPointer=output;

  unsigned char * endOfMem = traverser + width * height * 3;
  unsigned short * endOfOutMem = output + width * height;

  unsigned char * byte1;
    unsigned char * byte2;
      unsigned char * byte3;

  while ( ( traverser < endOfMem) && (  outputPointer < endOfOutMem ) )
  {
    byte1 = traverser ++ ;
    byte2 = traverser ++ ;
    byte3 = traverser ++ ;

  // if ( (*byte1!=*byte2) || (*byte2!=*byte3) ) { fprintf(stderr,"!"); }

    casterUshort = (unsigned short*) byte1;
    casterUint = (unsigned int *) byte1;

    //unsigned int outBit = *casterUint;
    //*outputPointer = (unsigned short) outBit;
    *outputPointer = *casterUshort;

    ++outputPointer;
  }

 return output;
}*/


int swapEndianness(struct Image * img)
{
  unsigned char * traverser=(unsigned char * ) img->pixels;
  unsigned char * traverserSwap1=(unsigned char * ) img->pixels;
  unsigned char * traverserSwap2=(unsigned char * ) img->pixels;

  unsigned int bytesperpixel = (img->bitsperpixel/8);
  unsigned char * endOfMem = traverser + img->width * img->height * img->channels * bytesperpixel;

  unsigned char tmp ;
  while ( ( traverser < endOfMem)  )
  {
    traverserSwap1 = traverser;
    traverserSwap2 = traverser+1;

    tmp = *traverserSwap1;
    *traverserSwap1 = *traverserSwap2;
    *traverserSwap2 = tmp;

    traverser += bytesperpixel;
  }

 return 1;
}

int readFromPNGDepthAndWriteToPNMDepth(char * inputFilename,char *outputFilename)
{
    struct Image * newImg  = readImage(inputFilename,PNG_CODEC,0);
    swapEndianness(newImg);

    fprintf(stderr,"Loaded Image with width : %u , height %u , channels %u , bitsperpixel %u\n",newImg->width,newImg->height,newImg->channels,newImg->bitsperpixel);

/*
    namedWindow( "Display window", CV_WINDOW_AUTOSIZE );// Create a window for display.
    IplImage  *imageDepthTest = cvCreateImage( cvSize(newImg->width,newImg->height), IPL_DEPTH_16U , 1 );
    if (imageDepthTest==0) { fprintf(stderr,"Could not create a new Depth OpenCV Image\n");  return 0; }
    char *opencv_depthTest_pointer_retainer = imageDepthTest->imageData; // UGLY HACK
    imageDepthTest->imageData = (char *) newImg->pixels;


    cvShowImage("RGBDAcquisition Depth Initial", imageDepthTest);

    IplImage *rdepth8Test  = cvCreateImage(cvSize(newImg->width,newImg->height), IPL_DEPTH_8U, 1);
    cvConvertScaleAbs(imageDepthTest, rdepth8Test, 255.0/2048,0);
    cvShowImage("RGBDAcquisition Depth", rdepth8Test);
    cvReleaseImage( &rdepth8Test );


    imageDepthTest->imageData = opencv_depthTest_pointer_retainer; // UGLY HACK
    cvReleaseImage( &imageDepthTest );
    waitKey(0);

*/
    writeImageFile(newImg,PNM_CODEC,outputFilename);

   return 0;
}



int main( int argc, char** argv )
{

    fprintf(stderr,"%u arguments\n",argc);
    if( argc < 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay ImageToStore" << endl;
     return -1;
    }



    if( argc == 3)
    {
     if ( (strstr(argv[1],".png")!=0) && (strstr(argv[2],".pnm")!=0) )
     {
         fprintf(stderr,"Using my custom loader / writer \n");
         return readFromPNGDepthAndWriteToPNMDepth(argv[1],argv[2]);
     }
    }


    fprintf(stderr,"Using OpenCV loader / writer \n");







    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

//    vector<int> compression_params;
//    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
//    compression_params.push_back(9);

    namedWindow( "Display window", CV_WINDOW_AUTOSIZE );// Create a window for display.

    IplImage  *imageDepth = cvCreateImage( cvSize(image.cols,image.rows), IPL_DEPTH_16U , 1 );
    if (imageDepth==0) { fprintf(stderr,"Could not create a new Depth OpenCV Image\n");  return 0; }
    char *opencv_depth_pointer_retainer = imageDepth->imageData; // UGLY HACK
    imageDepth->imageData = (char *) image.data;


    IplImage *rdepth8  = cvCreateImage(cvSize(image.cols,image.rows), IPL_DEPTH_8U, 1);
    cvConvertScaleAbs(imageDepth, rdepth8, 255.0/2048,0);
    cvShowImage("RGBDAcquisition Depth", rdepth8);
    cvReleaseImage( &rdepth8 );

    //cvShowImage("RGBDAcquisition Depth RAW",imageDepth);
    imageDepth->imageData = opencv_depth_pointer_retainer; // UGLY HACK
    cvReleaseImage( &imageDepth );


    imshow( "Display window", image );                   // Show our image inside it.

    if( argc >= 3) { imwrite(argv[2], image  ); } //Convert the image
     else          { waitKey(0); }  // Just show the window

    return 0;
}
