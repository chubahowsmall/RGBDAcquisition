#include "TrajectoryParser.h"
#include "InputParser_C.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#define LINE_MAX_LENGTH 1024
#define OBJECT_TYPES_TO_ADD_STEP 10
#define OBJECTS_TO_ADD_STEP 10
#define FRAMES_TO_ADD_STEP 100

#define PRINT_DEBUGGING_INFO 0
#define CASE_SENSITIVE_OBJECT_NAMES 0

int growVirtualStreamFrames(struct VirtualObject * streamObj,unsigned int framesToAdd)
{
  if (framesToAdd == 0) { return 0 ; }
  if (streamObj == 0) { fprintf(stderr,"Given an empty stream to grow \n"); return 0 ; }
  struct KeyFrame * new_frame;
  new_frame = (struct KeyFrame *) realloc( streamObj->frame, sizeof(struct KeyFrame)*( streamObj->MAX_numberOfFrames+framesToAdd ));

   if (new_frame == 0 )
    {
       fprintf(stderr,"Cannot add %u frames to our currently %u sized frame buffer\n",framesToAdd,streamObj->MAX_numberOfFrames);
       return 0;
    } else
     {
      //Clean up all new object types allocated
      void * clear_from_here  =  new_frame+streamObj->MAX_numberOfFrames;
      memset(clear_from_here,0,framesToAdd * sizeof(struct KeyFrame));
    }

   streamObj->MAX_numberOfFrames+=framesToAdd;
   streamObj->frame = new_frame ;
  return 1;
}


int growVirtualStreamObjectsTypes(struct VirtualStream * stream,unsigned int objectsTypesToAdd)
{
  if (objectsTypesToAdd == 0) { return 0 ; }
  if (stream == 0) { fprintf(stderr,"Given an empty stream to grow objects types on \n"); return 0 ; }
  struct ObjectType * new_objectTypes;
  new_objectTypes = (struct ObjectType *) realloc( stream->objectTypes , sizeof(struct ObjectType) * ( stream->MAX_numberOfObjectTypes+objectsTypesToAdd ));

   if (new_objectTypes == 0 )
    {
       fprintf(stderr,"Cannot add %u object types to our currently %u sized object type buffer\n",objectsTypesToAdd,stream->MAX_numberOfObjectTypes);
       return 0;
    } else
     {
      //Clean up all new object types allocated
      void * clear_from_here  = new_objectTypes+stream->MAX_numberOfObjectTypes;
      memset(clear_from_here,0,objectsTypesToAdd * sizeof(struct ObjectType));
    }

   stream->MAX_numberOfObjectTypes+=objectsTypesToAdd;
   stream->objectTypes = new_objectTypes ;
  return 1;
}



int growVirtualStreamObjects(struct VirtualStream * stream,unsigned int objectsToAdd)
{
  if (objectsToAdd == 0) { return 0 ; }
  if (stream == 0) { fprintf(stderr,"Given an empty stream to grow objects on \n"); return 0 ; }
  struct VirtualObject * new_object;
  new_object = (struct VirtualObject *) realloc( stream->object , sizeof(struct VirtualObject) * ( stream->MAX_numberOfObjects+objectsToAdd ));

   if (new_object == 0 )
    {
       fprintf(stderr,"Cannot add %u objects to our currently %u sized object buffer\n",objectsToAdd,stream->MAX_numberOfObjects);
       return 0;
    } else
    {
      //Clean up all new objects allocated
      void * clear_from_here  =  new_object+stream->MAX_numberOfObjects;
      memset(clear_from_here,0,objectsToAdd * sizeof(struct VirtualObject));
    }



   stream->MAX_numberOfObjects+=objectsToAdd;
   stream->object = new_object ;
  return 1;
}

/*!
    ------------------------------------------------------------------------------------------
                       /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\
                                 GROWING MEMORY ALLOCATIONS
    ------------------------------------------------------------------------------------------

    ------------------------------------------------------------------------------------------
                                 SEARCHING OF OBJECT ID's , TYPES etc
                       \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/
    ------------------------------------------------------------------------------------------

*/

static int dummy_strcasecmp_internal(char * input1, char * input2)
{
  #if CASE_SENSITIVE_OBJECT_NAMES
    return strcmp(input1,input2);
  #endif

    if ( (input1==0) || (input2==0) )
     {
         fprintf(stderr,"Error , calling strcasecmp_internal with null parameters \n");
         return 1;
     }
    unsigned int len1 = strlen(input1);
    unsigned int len2 = strlen(input2);
    if (len1!=len2)
     {
         //mismatched lengths of strings , they can't be equal..!
         return 1;
     }

   char A; //<- character buffer for input1
   char B; //<- character buffer for input2

   int i=0;
   while (i<len1) //len1 and len2 are equal
    {
       A = toupper(input1[i]);
       B = toupper(input2[i]);
       if (A!=B) { return 1; }
       ++i;
    }
  //if we reached this point , there where no reasons
  //why input1 and input2 could not be equal..
  return 0;
}



ObjectIDHandler getObjectID(struct VirtualStream * stream,char * name, unsigned int * found)
{
  if (stream==0) { fprintf(stderr,"Can't get object id (%s) for un allocated stream\n",name); }
  if (stream->object==0) { fprintf(stderr,"Can't get object id (%s) for un allocated object array\n",name); }

  *found=0;
  unsigned int i=0;
  for (i=0; i<stream->numberOfObjects; i++ )
   {
       if (dummy_strcasecmp_internal(name,stream->object[i].name)==0)
         {
              *found=1;
              return i;
         }
   }

   return 0;
}

ObjectTypeID getObjectTypeID(struct VirtualStream * stream,char * typeName,unsigned int * found)
{
  if (stream==0) { fprintf(stderr,"Can't get object id (%s) for un allocated stream\n",typeName); }
  if (stream->objectTypes==0) { fprintf(stderr,"Can't get object id (%s) for un allocated object type array\n",typeName); }

  *found=0;
  unsigned int i=0;
  for (i=0; i<stream->numberOfObjectTypes; i++ )
   {
       if (dummy_strcasecmp_internal(typeName,stream->objectTypes[i].name)==0)
         {
              *found=1;
              return i;
         }
   }

   return 0;
}

char * getObjectTypeModel(struct VirtualStream * stream,ObjectTypeID typeID)
{
  if (stream==0) { fprintf(stderr,"Can't get object id (%u) for un allocated stream\n",typeID); return 0; }
  if (stream->objectTypes==0) { fprintf(stderr,"Can't get object id (%u) for un allocated object type array\n",typeID); return 0;  }
  if (typeID>=stream->numberOfObjectTypes) { fprintf(stderr,"Can't get object id (%u) we only got %u Object Types \n",typeID,stream->numberOfObjectTypes); return 0; }

  return stream->objectTypes[typeID].model;
}


int getObjectColorsTrans(struct VirtualStream * stream,ObjectIDHandler ObjID,float * R,float * G,float * B,float * Transparency)
{
  *R = stream->object[ObjID].R;
  *G = stream->object[ObjID].G;
  *B = stream->object[ObjID].B;
  *Transparency = stream->object[ObjID].Transparency;
  return 1;
}


/*!
    ------------------------------------------------------------------------------------------
                       /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\
                                 SEARCHING OF OBJECT ID's , TYPES etc
    ------------------------------------------------------------------------------------------

    ------------------------------------------------------------------------------------------
                                   READING FILES , CREATING CONTEXT
                       \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/
    ------------------------------------------------------------------------------------------

*/

unsigned long getFileSize(char * filename)
{
  FILE * fp = fopen(filename,"r");
  if (fp == 0 ) { fprintf(stderr,"getFileSize cannot open %s \n",filename); return 0; }

  //Find out the size of the file..!
  fseek (fp , 0 , SEEK_END);
  unsigned long lSize = ftell (fp);
  fclose(fp);

  return lSize;
}


int readVirtualStream(struct VirtualStream * newstream)
{
  #if PRINT_DEBUGGING_INFO
  fprintf(stderr,"readVirtualStream(%s) called \n",newstream->filename);
  #endif

  //Our stack variables ..
  unsigned int readOpResult = 0;
  char line [LINE_MAX_LENGTH]={0};

  //Try and open filename
  FILE * fp = fopen(newstream->filename,"r");
  if (fp == 0 ) { fprintf(stderr,"Cannot open trajectory stream %s \n",newstream->filename); return 0; }

  //Find out the size of the file..!
  fseek (fp , 0 , SEEK_END);
  unsigned long lSize = ftell (fp);
  rewind (fp);
  fprintf(stderr,"Opening a %lu byte file %s \n",lSize,newstream->filename);

  //Allocate a token parser
  struct InputParserC * ipc=0;
  ipc = InputParser_Create(LINE_MAX_LENGTH,5);
  if (ipc==0)  { fprintf(stderr,"Cannot allocate memory for new stream\n"); return 0; }

  newstream->fileSize = lSize;

  //Add a dummy CAMERA Object here!
  growVirtualStreamObjectsTypes(newstream,OBJECT_TYPES_TO_ADD_STEP);
  strcpy( newstream->objectTypes[0].name , "camera" );
  strcpy( newstream->objectTypes[0].model , "camera" );
  ++newstream->numberOfObjectTypes;

  growVirtualStreamObjects(newstream,OBJECTS_TO_ADD_STEP);
  strcpy( newstream->object[0].name, "camera");
  strcpy( newstream->object[0].typeStr, "camera");
  strcpy( newstream->object[0].value, "camera");
  newstream->object[0].type = 0; //Camera
  newstream->object[0].R =0;
  newstream->object[0].G =0;
  newstream->object[0].B =0;
  newstream->object[0].Transparency=0;
  ++newstream->numberOfObjects;
  // CAMERA OBJECT ADDED


 //Everything is set , Lets read the file!
  while (!feof(fp))
  {
   //We get a new line out of the file
   readOpResult = (fgets(line,LINE_MAX_LENGTH,fp)!=0);
   if ( readOpResult != 0 )
    {
      //We tokenize it
      unsigned int words_count = InputParser_SeperateWords(ipc,line,0);
      if ( words_count > 0 )
         {
            /*! REACHED AN AUTO REFRESH DECLERATION ( AUTOREFRESH(1500) )
              argument 0 = AUTOREFRESH , argument 1 = value in milliseconds (0 = off ) */
            if (InputParser_WordCompareNoCase(ipc,0,(char*)"AUTOREFRESH",11)==1)
            {
                newstream->autoRefresh = InputParser_GetWordInt(ipc,1);
            } else
            /*! REACHED AN INTERPOLATE TIME SWITCH DECLERATION ( INTERPOLATE_TIME(1) )
              argument 0 = INTERPOLATE_TIME , argument 1 = (0 = off ) ( 1 = on )*/
            if (InputParser_WordCompareNoCase(ipc,0,(char*)"INTERPOLATE_TIME",16)==1)
            {
                //The configuration INTERPOLATE_TIME is the "opposite" of this flag ignore time
                newstream->ignoreTime = InputParser_GetWordInt(ipc,1);
                // so we flip it here.. , the default is not ignoring time..
                if (newstream->ignoreTime == 0 ) { newstream->ignoreTime=1; } else
                                                 { newstream->ignoreTime=0; }
            } else
            /*! REACHED AN OBJECT TYPE DECLERATION ( OBJECTTYPE(spatoula_type,"spatoula.obj") )
              argument 0 = OBJECTTYPE , argument 1 = name ,  argument 2 = value */
            if (InputParser_WordCompareNoCase(ipc,0,(char*)"OBJECTTYPE",10)==1)
            {
               if (newstream->MAX_numberOfObjectTypes<=newstream->numberOfObjectTypes+1) { growVirtualStreamObjectsTypes(newstream,OBJECT_TYPES_TO_ADD_STEP); }
               //Now we should definately have enough space for our new frame
               if (newstream->MAX_numberOfObjectTypes<=newstream->numberOfObjectTypes+1) { fprintf(stderr,"Cannot add new OBJECTTYPE instruction\n"); }
                 else
                 {
                   //We have the space so lets fill our new object spot ..!
                   unsigned int pos = newstream->numberOfObjectTypes;
                    InputParser_GetWord(ipc,1,newstream->objectTypes[pos].name,MAX_PATH);
                    InputParser_GetWord(ipc,2,newstream->objectTypes[pos].model,MAX_PATH);
                   ++newstream->numberOfObjectTypes;
                 }
            } else
            /*! REACHED AN OBJECT DECLERATION ( OBJECT(something,spatoula_type,0,255,0,0,spatoula_something) )
              argument 0 = OBJECT , argument 1 = name ,  argument 2 = type ,  argument 3-5 = RGB color  , argument 6 Transparency , argument 7 = Data */
            if (InputParser_WordCompareNoCase(ipc,0,(char*)"OBJECT",6)==1)
            {
               if (newstream->MAX_numberOfObjects<=newstream->numberOfObjects+1) { growVirtualStreamObjects(newstream,OBJECTS_TO_ADD_STEP); }
               //Now we should definately have enough space for our new frame
               if (newstream->MAX_numberOfObjects<=newstream->numberOfObjects+1) { fprintf(stderr,"Cannot add new OBJECT instruction\n"); }
                 else
                 {
                   //We have the space so lets fill our new object spot ..!
                   unsigned int pos = newstream->numberOfObjects;
                    InputParser_GetWord(ipc,1,newstream->object[pos].name,MAX_PATH);
                    InputParser_GetWord(ipc,2,newstream->object[pos].typeStr,MAX_PATH);

                    newstream->object[pos].R = (float) InputParser_GetWordInt(ipc,3)  /  255;
                    newstream->object[pos].G = (float) InputParser_GetWordInt(ipc,4)  /  255;
                    newstream->object[pos].B = (float) InputParser_GetWordInt(ipc,5)  /  255;
                    newstream->object[pos].Transparency = (float) InputParser_GetWordInt(ipc,6)  /  255;
                    newstream->object[pos].nocolor = (float) InputParser_GetWordInt(ipc,7);

                    InputParser_GetWord(ipc,8,newstream->object[pos].value,15);

                    unsigned int found=0;
                    newstream->object[pos].type = getObjectTypeID(newstream,newstream->object[pos].typeStr,&found);
                    if (!found) { fprintf(stderr,"Please note that type %s couldn't be found for object %s \n",newstream->object[pos].typeStr,newstream->object[pos].name); }

                   ++newstream->numberOfObjects;
                 }
            } else
            /*! REACHED A POSITION DECLERATION ( POS(hand,0,   0.0,0.0,0.0 , 0.0,0.0,0.0,0.0 ) )
              argument 0 = POS , argument 1 = name ,  argument 2 = time in MS , argument 3-5 = X,Y,Z , argument 6-9 = Rotations*/
            if (InputParser_WordCompareNoCase(ipc,0,(char*)"POS",3)==1)
            {
               char Name[123];
               InputParser_GetWord(ipc,1,Name,123);
               unsigned int ObjFound = 0;
               unsigned int ObjID = getObjectID(newstream,Name,&ObjFound);

               if (!ObjFound)
               {
                  fprintf(stderr,"Could not Find object %s , line `%s` is skipped\n",Name , line);
               } else
               {
                 fprintf(stderr,"Get ObjID %u from String %s \n",ObjID,Name);
                 //We came across a POS command , lets see if it fits
               if (newstream->object[ObjID].MAX_numberOfFrames<=newstream->object[ObjID].numberOfFrames+1) { growVirtualStreamFrames(&newstream->object[ObjID],FRAMES_TO_ADD_STEP); }
               //Now we should definately have enough space for our new frame
               if (newstream->object[ObjID].MAX_numberOfFrames<=newstream->object[ObjID].numberOfFrames+1) { fprintf(stderr,"Cannot add new POS instruction to Object %u \n",ObjID); }
                 else
                 {
                   //We have the space so lets fill our new frame spot ..!
                   unsigned int pos = newstream->object[ObjID].numberOfFrames;

                   // 1 is object name
                   newstream->object[ObjID].frame[pos].time = InputParser_GetWordInt(ipc,2);
                   newstream->object[ObjID].frame[pos].x = InputParser_GetWordFloat(ipc,3);
                   newstream->object[ObjID].frame[pos].y = InputParser_GetWordFloat(ipc,4);
                   newstream->object[ObjID].frame[pos].z = InputParser_GetWordFloat(ipc,5);
                   newstream->object[ObjID].frame[pos].rot1 = InputParser_GetWordFloat(ipc,6);
                   newstream->object[ObjID].frame[pos].rot2 = InputParser_GetWordFloat(ipc,7);
                   newstream->object[ObjID].frame[pos].rot3 = InputParser_GetWordFloat(ipc,8);
                   newstream->object[ObjID].frame[pos].rot4 = InputParser_GetWordFloat(ipc,9);

                   if (newstream->object[ObjID].MAX_timeOfFrames <= newstream->object[ObjID].frame[pos].time)
                      {
                         newstream->object[ObjID].MAX_timeOfFrames = newstream->object[ObjID].frame[pos].time;
                      } else
                      {
                         fprintf(stderr,"Error in configuration file , object positions not in correct time order .. \n");
                      }

                   #if PRINT_DEBUGGING_INFO
                   fprintf(stderr,"String %s resolves to : \n",line);
                   fprintf(stderr,"X %02f Y %02f Z %02f ROT %02f %02f %02f %02f\n",newstream->object[ObjID].frame[pos].x,newstream->object[ObjID].frame[pos].y,newstream->object[ObjID].frame[pos].z ,
                                 newstream->object[ObjID].frame[pos].rot1 , newstream->object[ObjID].frame[pos].rot2 , newstream->object[ObjID].frame[pos].rot3 , newstream->object[ObjID].frame[pos].rot4 );
                   #endif


                   ++newstream->object[ObjID].numberOfFrames;
                 }
               }
            }
         } // End of line containing tokens
    } //End of getting a line while reading the file
  }

  fclose(fp);
  InputParser_Destroy(ipc);

  return 1;
}




int destroyVirtualStreamInternal(struct VirtualStream * stream,int also_destrstream_struct)
{
   if (stream==0) { return 1; }
   if (stream->object==0) { return 1; }
   unsigned int i =0 ;

  //CLEAR OBJECTS , AND THEIR FRAMES
   for ( i=0; i<stream->MAX_numberOfObjects; i++)
    {
       if ( stream->object[i].frame!= 0 )
         {
            free(stream->object[i].frame);
            stream->object[i].frame=0;
         }
    }
   stream->MAX_numberOfObjects=0;
   stream->numberOfObjects=0;
   free(stream->object);
   stream->object=0;

   //CLEAR TYPES OF OBJECTS
    if ( stream->objectTypes!= 0 )
         {
            free(stream->objectTypes);
            stream->objectTypes=0;
         }
    stream->MAX_numberOfObjectTypes=0;
    stream->numberOfObjectTypes=0;

   if (also_destrstream_struct) { free(stream); }
   return 1;
}


int destroyVirtualStream(struct VirtualStream * stream)
{
    return destroyVirtualStreamInternal(stream,1);
}



int refreshVirtualStream(struct VirtualStream * newstream)
{
   #if PRINT_DEBUGGING_INFO
   fprintf(stderr,"refreshingVirtualStream\n");
   #endif

   destroyVirtualStreamInternal(newstream,0);
   //Please note that the newstream structure does not get a memset operation anywhere around here
   //thats in order to keep the initial time / frame configuration
   //Object numbers , Object type numbers,  Frame numbers are cleaned by the destroyVirtualStreamInternal call

   return readVirtualStream(newstream);
}

struct VirtualStream * createVirtualStream(char * filename)
{
  //Allocate a virtual stream structure
  struct VirtualStream * newstream = (struct VirtualStream *) malloc(sizeof(struct VirtualStream));
  if (newstream==0)  {  fprintf(stderr,"Cannot allocate memory for new stream\n"); return 0; }

  //Clear the whole damn thing..
  memset(newstream,0,sizeof(struct VirtualStream));
  strncpy(newstream->filename,filename,MAX_PATH);

  if (!readVirtualStream(newstream))
    {
      fprintf(stderr,"Could not read Virtual Stream from file %s \n",filename);
      destroyVirtualStream(newstream);
      return 0;
    }

  return newstream;
}

/*!
    ------------------------------------------------------------------------------------------
                       /\   /\   /\   /\   /\   /\   /\   /\   /\   /\   /\
                                 READING FILES , CREATING CONTEXT
    ------------------------------------------------------------------------------------------

    ------------------------------------------------------------------------------------------
                                     GETTING AN OBJECT POSITION
                       \/   \/   \/   \/   \/   \/   \/   \/   \/   \/   \/
    ------------------------------------------------------------------------------------------

*/





int fillPosWithNull(struct VirtualStream * stream,ObjectIDHandler ObjID,float * pos)
{
    #if PRINT_DEBUGGING_INFO
    fprintf(stderr,"Returning null frame for obj %u \n",ObjID);
    #endif

    pos[0]=0.0;
    pos[1]=0.0;
    pos[2]=0.0;
    pos[3]=0.0;
    pos[4]=0.0;
    pos[5]=0.0;
    pos[6]=0.0;

    return 1;
}



int fillPosWithFrame(struct VirtualStream * stream,ObjectIDHandler ObjID,unsigned int FrameIDToReturn,float * pos)
{
    #if PRINT_DEBUGGING_INFO
    fprintf(stderr,"Returning frame %u \n",FrameIDToReturn);
    #endif

    if (FrameIDToReturn >= stream->object[ObjID].numberOfFrames )
     {
         fprintf(stderr,"fillPosWithFrame asked to return frame out of bounds\n");
         return 0;
     }

    pos[0]=stream->object[ObjID].frame[FrameIDToReturn].x;
    pos[1]=stream->object[ObjID].frame[FrameIDToReturn].y;
    pos[2]=stream->object[ObjID].frame[FrameIDToReturn].z;
    pos[3]=stream->object[ObjID].frame[FrameIDToReturn].rot1;
    pos[4]=stream->object[ObjID].frame[FrameIDToReturn].rot2;
    pos[5]=stream->object[ObjID].frame[FrameIDToReturn].rot3;
    pos[6]=stream->object[ObjID].frame[FrameIDToReturn].rot4;
    return 1;
}


int fillPosWithInterpolatedFrame(struct VirtualStream * stream,ObjectIDHandler ObjID,float * pos,
                                 unsigned int PrevFrame,unsigned int NextFrame , unsigned int time )
{
   if (PrevFrame==NextFrame)
    {
       return fillPosWithFrame(stream,ObjID,PrevFrame,pos);
    }


    #if PRINT_DEBUGGING_INFO
    fprintf(stderr,"Interpolating frames @  %u , between %u and %u \n",time,PrevFrame,NextFrame);
    #endif
    float interPos[7];

    unsigned int MAX_stepTime= stream->object[ObjID].frame[NextFrame].time - stream->object[ObjID].frame[PrevFrame].time;
    if (MAX_stepTime == 0 ) { MAX_stepTime=1; }
    unsigned int our_stepTime= time - stream->object[ObjID].frame[PrevFrame].time;


    interPos[0]=(float) ( stream->object[ObjID].frame[NextFrame].x-stream->object[ObjID].frame[PrevFrame].x ) * our_stepTime / MAX_stepTime;
    interPos[0]+=stream->object[ObjID].frame[PrevFrame].x;

    interPos[1]=(float) ( stream->object[ObjID].frame[NextFrame].y-stream->object[ObjID].frame[PrevFrame].y ) * our_stepTime / MAX_stepTime;
    interPos[1]+=stream->object[ObjID].frame[PrevFrame].y;

    interPos[2]=(float) ( stream->object[ObjID].frame[NextFrame].z-stream->object[ObjID].frame[PrevFrame].z ) * our_stepTime / MAX_stepTime;
    interPos[2]+=stream->object[ObjID].frame[PrevFrame].z;

    interPos[3]=(float) ( stream->object[ObjID].frame[NextFrame].rot1-stream->object[ObjID].frame[PrevFrame].rot1 ) * our_stepTime / MAX_stepTime;
    interPos[3]+=stream->object[ObjID].frame[PrevFrame].rot1;

    interPos[4]=(float) ( stream->object[ObjID].frame[NextFrame].rot2-stream->object[ObjID].frame[PrevFrame].rot2 ) * our_stepTime / MAX_stepTime;
    interPos[4]+=stream->object[ObjID].frame[PrevFrame].rot2;

    interPos[5]=(float) ( stream->object[ObjID].frame[NextFrame].rot3-stream->object[ObjID].frame[PrevFrame].rot3 ) * our_stepTime / MAX_stepTime;
    interPos[5]+=stream->object[ObjID].frame[PrevFrame].rot3;

    interPos[6]=(float) ( stream->object[ObjID].frame[NextFrame].rot4-stream->object[ObjID].frame[PrevFrame].rot4 ) * our_stepTime / MAX_stepTime;
    interPos[6]+=stream->object[ObjID].frame[PrevFrame].rot4;

    pos[0]=interPos[0]; pos[1]=interPos[1]; pos[2]=interPos[2];
    pos[3]=interPos[3]; pos[4]=interPos[4]; pos[5]=interPos[5];
    pos[6]=interPos[6];

    #if PRINT_DEBUGGING_INFO
    fprintf(stderr,"ok \n");
    #endif

    return 1;
}






int calculateVirtualStreamPos(struct VirtualStream * stream,ObjectIDHandler ObjID,unsigned int timeAbsMilliseconds,float * pos)
{
   if (stream==0) { fprintf(stderr,"calculateVirtualStreamPos called with null stream\n"); return 0; }
   if (stream->object==0) { fprintf(stderr,"calculateVirtualStreamPos called with null object array\n"); return 0; }
   if (stream->numberOfObjects<=ObjID) { fprintf(stderr,"calculateVirtualStreamPos ObjID %u is out of bounds (%u)\n",ObjID,stream->numberOfObjects); return 0; }
   if (stream->object[ObjID].frame == 0 ) { fprintf(stderr,"calculateVirtualStreamPos ObjID %u does not have a frame array allocated\n",ObjID); return 0; }
   if (stream->object[ObjID].numberOfFrames == 0 ) { fprintf(stderr,"calculateVirtualStreamPos ObjID %u has 0 frames\n",ObjID); return 0; }


   if (stream->autoRefresh != 0 )
    {
         //Check for refreshed version ?
       if (stream->autoRefresh < timeAbsMilliseconds-stream->lastRefresh )
          {
            unsigned long current_size = getFileSize(stream->filename);
            if (current_size != stream->fileSize)
             {
              refreshVirtualStream(stream);
              stream->lastRefresh = timeAbsMilliseconds;
             }
          }
    }

   unsigned int FrameIDToReturn = 0;
   unsigned int FrameIDLast = 0;
   unsigned int FrameIDNext = 0;


   /*!OK , Two major cases here..! The one is a simple Next frame getter , the second is a more complicated interpolated frame getter..! */
   if ( (stream->object[ObjID].MAX_numberOfFrames == 0 ) )
   {
       fillPosWithNull(stream,ObjID,pos);
       return 1;
   } else
   if  ( (stream->ignoreTime) || (stream->object[ObjID].MAX_numberOfFrames == 1 ) )
   {
    //We might want to ignore time and just return frame after frame on each call!
    //Also if we only got one frame for the object there is no point in trying to interpolate time etc.. so just handle things here..
    if ( stream->object[ObjID].lastFrame +1 >= stream->object[ObjID].MAX_numberOfFrames ) { stream->object[ObjID].lastFrame  = 0; }
    FrameIDToReturn = stream->object[ObjID].lastFrame;
    ++stream->object[ObjID].lastFrame;

    fillPosWithFrame(stream,ObjID,FrameIDToReturn,pos);

    FrameIDLast = FrameIDToReturn;
    FrameIDNext = FrameIDToReturn+1;
    if ( FrameIDNext >= stream->object[ObjID].numberOfFrames )
     {
       FrameIDNext  = 0;
     }

     return 1;

   } /*!END OF SIMPLE FRAME GETTER*/
   else
   { /*!START OF INTERPOLATED FRAME GETTER*/
     //This is the case when we respect time , we will pick two frames and interpolate between them
     if ( timeAbsMilliseconds > stream->object[ObjID].MAX_timeOfFrames )
     {
       //This means we have passed the last frame.. so lets find out where we really are..
       timeAbsMilliseconds = timeAbsMilliseconds % stream->object[ObjID].MAX_timeOfFrames;
       //timeAbsMilliseconds should contain a valid value now somewhere from 0->MAX_timeOfFrames
     }

     #if PRINT_DEBUGGING_INFO
     fprintf(stderr,"Object %u has %u frames , lets search where we are \n",ObjID,stream->object[ObjID].numberOfFrames);
     #endif

     //We scan all the frames to find out the "last one" and the "next one"
     unsigned int i =0;
     for ( i=0; i <stream->object[ObjID].MAX_numberOfFrames-1; i++ )
      {
       if (( stream->object[ObjID].frame[i].time <= timeAbsMilliseconds )
                 &&
           ( timeAbsMilliseconds <= stream->object[ObjID].frame[i+1].time )  )
            {
               //This is the "next" frame!
               FrameIDLast = i;
               FrameIDNext = i+1;
               //This should be handled by raw response to zero elemetn :P
              break;
            }
      }

    //We now have our Last and Next frame , all that remains is extracting the
    //interpolated time between them..!
    return fillPosWithInterpolatedFrame(stream,ObjID,pos,FrameIDLast,FrameIDNext,timeAbsMilliseconds);

   } /*!END OF INTERPOLATED FRAME GETTER*/

    return 0;
}



int calculateVirtualStreamPosAfterTime(struct VirtualStream * stream,ObjectIDHandler ObjID,unsigned int timeAfterMilliseconds,float * pos)
{
   stream->object[ObjID].lastCalculationTime+=timeAfterMilliseconds;
   return calculateVirtualStreamPos(stream,ObjID,stream->object[ObjID].lastCalculationTime,pos);
}
