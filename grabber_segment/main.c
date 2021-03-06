#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../acquisition/Acquisition.h"
#include "../acquisitionSegment/AcquisitionSegment.h"




char outputfoldername[512]={0};
char inputname[512]={0};

int makepath(char * path)
{
    //FILE *fp;
    /* Open the command for reading. */
    char command[1024];
    sprintf(command,"mkdir -p %s",outputfoldername);
    fprintf(stderr,"Executing .. %s \n",command);

    return system(command);
}


int main(int argc, char *argv[])
{
 fprintf(stderr,"Generic Multiplexed Grabber Application based on Acquisition lib .. \n");
 unsigned int possibleModules = acquisitionGetModulesCount();
 fprintf(stderr,"Linked to %u modules.. \n",possibleModules);

 if (possibleModules==0)
    {
       fprintf(stderr,"Acquisition Library is linked to zero modules , can't possibly do anything..\n");
       return 1;
    }

 //We want to grab multiple frames in this example if the user doesnt supply a parameter default is 10..
  unsigned int frameNum=0,maxFramesToGrab=10;
  ModuleIdentifier moduleID_1 = TEMPLATE_ACQUISITION_MODULE;//OPENNI1_ACQUISITION_MODULE;//

  /*! --------------------------------- INITIALIZATION FROM COMMAND LINE PARAMETERS --------------------------------- */

  int i=0;
  for (i=0; i<argc; i++)
  {
    if (strcmp(argv[i],"-maxFrames")==0) {
                                           maxFramesToGrab=atoi(argv[i+1]);
                                           fprintf(stderr,"Setting frame grab to %u \n",maxFramesToGrab);
                                         } else
    if (strcmp(argv[i],"-module")==0)    {
                                           moduleID_1 = getModuleIdFromModuleName(argv[i+1]);
                                           fprintf(stderr,"Overriding Module Used , set to %s ( %u ) \n",getModuleStringName(moduleID_1),moduleID_1);
                                         } else
    if (
         (strcmp(argv[i],"-to")==0) ||
         (strcmp(argv[i],"-o")==0)
        )
        {
          strcpy(outputfoldername,"frames/");
          strcat(outputfoldername,argv[i+1]);
          makepath(outputfoldername);
          fprintf(stderr,"OutputPath , set to %s  \n",outputfoldername);
         }
       else
    if (
        (strcmp(argv[i],"-from")==0) ||
        (strcmp(argv[i],"-i")==0)
       )
       { strcat(inputname,argv[i+1]); fprintf(stderr,"Input , set to %s  \n",inputname); }
  }


  if (!acquisitionIsModuleLinked(moduleID_1))
   {
       fprintf(stderr,"The module you are trying to use as module A is not linked in this build of the Acquisition library..\n");
       return 1;
   }

  /*! --------------------------------- INITIALIZATION FROM COMMAND LINE PARAMETERS END --------------------------------- */



  //We need to initialize our module before calling any related calls to the specific module..
  if (!acquisitionStartModule(moduleID_1,16 /*maxDevices*/ , 0 ))
  {
       fprintf(stderr,"Could not start module A %s ..\n",getModuleStringName(moduleID_1));
       return 1;
   }
   //We want to initialize all possible devices in this example..
   unsigned int devID_1=0 ;


   char * devName = inputname;
   if (strlen(inputname)<1) { devName=0; }

    //Initialize Every OpenNI Device
    acquisitionOpenDevice(moduleID_1,devID_1,devName,640,480,25);
    acquisitionMapDepthToRGB(moduleID_1,devID_1);


    usleep(1000); // Waiting a while for the glitch frames to pass
    char outfilename[512]={0};


    unsigned int widthRGB , heightRGB , channelsRGB , bitsperpixelRGB;
    acquisitionGetColorFrameDimensions(moduleID_1,devID_1,&widthRGB,&heightRGB ,&channelsRGB , &bitsperpixelRGB );
    //Todo , check with module 2 bla bla
    char * rgbOut = ( char* )  malloc(widthRGB*heightRGB*channelsRGB * (bitsperpixelRGB/8 ) );

    unsigned int widthDepth , heightDepth , channelsDepth , bitsperpixelDepth;
    acquisitionGetDepthFrameDimensions(moduleID_1,devID_1,&widthDepth,&heightDepth ,&channelsDepth , &bitsperpixelDepth );
    short * depthOut = ( short* )  malloc(widthDepth*heightDepth*channelsDepth * (bitsperpixelDepth/8 ) );



   //------------------------------------------------------------------
   //                        CONFIGURATION
   //------------------------------------------------------------------
   int doNotSegmentRGB=0;
   int doNotSegmentDepth=1;

   struct SegmentationFeaturesRGB segConfRGB={0};
   segConfRGB.minX=0;  segConfRGB.maxX=611;
   segConfRGB.minY=230; segConfRGB.maxY=472;

   segConfRGB.minR=110; segConfRGB.minG=110; segConfRGB.minB=110;
   segConfRGB.maxR=256; segConfRGB.maxG=256; segConfRGB.maxB=256;

   segConfRGB.replaceR=92; segConfRGB.replaceG=45; segConfRGB.replaceB=36;
   segConfRGB.enableReplacingColors=1;

   struct SegmentationFeaturesDepth segConfDepth={0};
   segConfDepth.minX=23;  segConfDepth.maxX=556;
   segConfDepth.minY=101; segConfDepth.maxY=358;
   segConfDepth.minDepth=10; segConfDepth.maxDepth=790;
   //------------------------------------------------------------------
   //------------------------------------------------------------------


   float centerX;
   float centerY;
   float centerZ;

   for (frameNum=0; frameNum<maxFramesToGrab; frameNum++)
    {
        acquisitionSnapFrames(moduleID_1,devID_1);


        sprintf(outfilename,"%s/colorFrame_%u_%05u.pnm",outputfoldername,devID_1,frameNum);
        if (doNotSegmentRGB)
        { saveRawImageToFile(outfilename,acquisitionGetColorFrame(moduleID_1,devID_1),widthRGB,heightRGB,channelsRGB,bitsperpixelRGB); }
         else
        {
         char * segmentedRGB = segmentRGBFrame(acquisitionGetColorFrame(moduleID_1,devID_1),widthRGB , heightRGB, &segConfRGB);
         saveRawImageToFile(outfilename,segmentedRGB,widthRGB,heightRGB,channelsRGB,bitsperpixelRGB);
         free (segmentedRGB);
        }


        sprintf(outfilename,"%s/depthFrame_%u_%05u",outputfoldername,devID_1,frameNum);
        if (doNotSegmentDepth)
        { saveRawImageToFile(outfilename,(char*) acquisitionGetDepthFrame(moduleID_1,devID_1),widthDepth,heightDepth,channelsDepth,bitsperpixelDepth); }
         else
        {
         short * segmentedDepth = segmentDepthFrame(acquisitionGetDepthFrame(moduleID_1,devID_1), widthDepth,heightDepth,&segConfDepth);
         saveRawImageToFile(outfilename,(char*) segmentedDepth,widthDepth,heightDepth,channelsDepth,bitsperpixelDepth);
         getDepthBlobAverage(&centerX,&centerY,&centerZ,segmentedDepth,widthDepth,heightDepth);
         fprintf(stderr,"AVG!%0.2f#%0.2f#%0.2f\n",centerX,centerY,centerZ);
         free (segmentedDepth);
        }


    }


    fprintf(stderr,"Done grabbing %u frames! \n",maxFramesToGrab);
    acquisitionCloseDevice(moduleID_1,devID_1);


    acquisitionStopModule(moduleID_1);


    return 0;
}
