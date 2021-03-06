#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../acquisition/Acquisition.h"

char inputname[512]={0};
char outputfoldername[512]={0};




int makepath(char * path)
{
    // FILE *fp;
    /* Open the command for reading. */
    char command[1024];
    sprintf(command,"mkdir -p %s",outputfoldername);
    fprintf(stderr,"Executing .. %s \n",command);

    return system(command);
}







int main(int argc, char *argv[])
{
 fprintf(stderr,"Generic Grabber Application based on Acquisition lib .. \n");
 unsigned int possibleModules = acquisitionGetModulesCount();
 fprintf(stderr,"Linked to %u modules.. \n",possibleModules);


  ModuleIdentifier moduleID = OPENGL_ACQUISITION_MODULE;//OPENNI1_ACQUISITION_MODULE;//

  if (possibleModules==0)
    {
       fprintf(stderr,"Acquisition Library is linked to zero modules , can't possibly do anything..\n");
       return 1;
    }

  unsigned int width=640,height=480,framerate=25;
  unsigned int frameNum=0,maxFramesToGrab=10;
  int i=0;
  for (i=0; i<argc; i++)
  {
    if (strcmp(argv[i],"-maxFrames")==0) {
                                           maxFramesToGrab=atoi(argv[i+1]);
                                           fprintf(stderr,"Setting frame grab to %u \n",maxFramesToGrab);
                                         } else
    if (strcmp(argv[i],"-module")==0)    {
                                           moduleID = getModuleIdFromModuleName(argv[i+1]);
                                           fprintf(stderr,"Overriding Module Used , set to %s ( %u ) \n",getModuleStringName(moduleID),moduleID);
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
      else
    if (strcmp(argv[i],"-fps")==0)       {
                                             framerate=atoi(argv[i+1]);
                                             fprintf(stderr,"Framerate , set to %u  \n",framerate);
                                         }
  }


  if (!acquisitionIsModuleLinked(moduleID))
   {
       fprintf(stderr,"The module you are trying to use is not linked in this build of the Acquisition library..\n");
       return 1;
   }

  //We need to initialize our module before calling any related calls to the specific module..
  if (!acquisitionStartModule(moduleID,16 /*maxDevices*/ , 0 ))
  {
       fprintf(stderr,"Could not start module %s ..\n",getModuleStringName(moduleID));
       return 1;
   }

  //We want to initialize all possible devices in this example..
  unsigned int devID=0,maxDevID=acquisitionGetModuleDevices(moduleID);
  if (maxDevID==0)
  {
      fprintf(stderr,"No devices found for Module used \n");
      return 1;
  }


  char * devName = inputname;
  if (strlen(inputname)<1) { devName=0; }
    //Initialize Every OpenNI Device
    for (devID=0; devID<maxDevID; devID++)
     {
        /*The first argument (Dev ID) could also be ANY_OPENNI2_DEVICE for a single camera setup */
        acquisitionOpenDevice(moduleID,devID,devName,width,height,framerate);
        acquisitionMapDepthToRGB(moduleID,devID);
        //acquisitionMapRGBToDepth(moduleID,devID);
     }
    fprintf(stderr,"3\n");
    usleep(1000*1000); // Waiting a while for the glitch frames to pass
    fprintf(stderr,"2\n");
    usleep(1000*1000); // Waiting a while for the glitch frames to pass
    fprintf(stderr,"1\n");
    usleep(1000*1000); // Waiting a while for the glitch frames to pass
    fprintf(stderr,"Starting \n");

    char outfilename[1024]={0};

   if ( maxFramesToGrab==0 ) { maxFramesToGrab= 1294967295; } //set maxFramesToGrab to "infinite" :P
   for (frameNum=0; frameNum<maxFramesToGrab; frameNum++)
    {

     for (devID=0; devID<maxDevID; devID++)
      {
        acquisitionSnapFrames(moduleID,devID);

        sprintf(outfilename,"%s/colorFrame_%u_%05u",outputfoldername,devID,frameNum);
        acquisitionSaveColorFrame(moduleID,devID,outfilename);

        sprintf(outfilename,"%s/depthFrame_%u_%05u",outputfoldername,devID,frameNum);
        acquisitionSaveDepthFrame(moduleID,devID,outfilename);

        //sprintf(outfilename,"%s/pointCloud_%u_%05u.pcd",outputfoldername,devID,frameNum);
        //acquisitionSavePCDPointCoud(moduleID,devID,outfilename);

        //sprintf(outfilename,"%s/depthFrame1C_%u_%05u.pnm",outputfoldername,devID,frameNum);
        //acquisitionSaveDepthFrame1C(moduleID,devID,outfilename);

        //sprintf(outfilename,"%s/coloreddepthFrame_%u_%05u.pnm",outputfoldername,devID,frameNum);
        //acquisitionSaveColoredDepthFrame(moduleID,devID,outfilename);
      }
    }

    fprintf(stderr,"Done grabbing %u frames! \n",maxFramesToGrab);

    for (devID=0; devID<maxDevID; devID++)
     {
        /*The first argument (Dev ID) could also be ANY_OPENNI2_DEVICE for a single camera setup */
        acquisitionCloseDevice(moduleID,devID);
     }

    acquisitionStopModule(moduleID);

    return 0;
}
