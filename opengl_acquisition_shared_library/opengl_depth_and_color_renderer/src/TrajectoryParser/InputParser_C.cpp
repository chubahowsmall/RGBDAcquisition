#include "InputParser_C.h"
/* InputParser..
   A small generic library for parsing a string and tokenizing it!
   GITHUB Repo : http://github.com/AmmarkoV/InputParser
   my URLs: http://ammarkov.ath.cx
   Written by Ammar Qammaz a.k.a. AmmarkoV 2006-2010
*/


char _ipc_ver[]=" 0.356 written from scratch - 8/2/10 \0";

char * InputParserC_Version()
{
  return (char *) _ipc_ver;
}



int InputParser_ClearNonCharacters(char * inpt , unsigned int length)
{
   unsigned int skip_chars=0;
   unsigned int i=0;
   while  (i<length)
    {
       if (inpt[i]<' ') /*The characters before space should be filtered*/
        {
          ++skip_chars;
        }

        if (skip_chars>0)
        {
           if (i+skip_chars>=length)
             {
                inpt[i]=0;
             } else
             {
                inpt[i]=inpt[i+skip_chars];
             }
        }
      ++i;
    }
   return 1;
}



int InputParser_TrimCharactersStart(char * inpt , unsigned int length,char what2trim)
{
   unsigned int skip_chars=0;
   unsigned int i=0;


   while ((inpt[skip_chars]==what2trim)&&(skip_chars<length)) { ++skip_chars; }

   while  (i<length)
    {
        if (skip_chars>0)
        {
           if (i+skip_chars>=length)
             {
                inpt[i]=0;
             } else
             {
                inpt[i]=inpt[i+skip_chars];
             }
        }
      ++i;
    }

   return 1;
}


int InputParser_TrimCharactersEnd(char * inpt , unsigned int length,char what2trim)
{
   if ( strlen(inpt)!=length ) { length=strlen(inpt); }
   if ( length==0 ) { return 1;}
   if ( (length==1) && (inpt[0]==what2trim) ) {  inpt[0]=0; return 1; } else
   if ( length==1 )                           { return 1; }

   unsigned int i;
   i=length-1;
   while ((inpt[i]==what2trim)&&(i>0)) { --i; }
   if ( i==length-1 ) { /*No chars found*/ return 1; }

   if ((i==0) && (inpt[0]==what2trim) ) { inpt[0]=0; } else
                                        {
                                          inpt[i+1]=0;
                                        }
   return 1;
}

int InputParser_TrimCharacters(char * inpt , unsigned int length,char what2trim)
{
  int i=0;
  i=InputParser_TrimCharactersStart(inpt,length,what2trim);
  if (i==0) { return 0; }

  return InputParser_TrimCharactersEnd(inpt,length,what2trim);
}

inline signed int Str2Int_internal(char * inpt,unsigned int start_from,unsigned int length)
{
    if ( inpt == 0 ) { fprintf(stderr,"Null string to Str2IntInternal!\n"); return 0;}
    int intresult;
    int multiplier;
    int curnum;
    unsigned char trailing_sign_seek;
    unsigned char positive_number;
    signed int i;

    intresult=0,multiplier=1,curnum=0;
    trailing_sign_seek=1 , positive_number=1;
    /*fprintf(stderr,"Converting to int string (%p) begining from %u and ending at %u ",inpt,start_from,start_from+length);*/
    for (i=(signed int) start_from+length-1; i>=(signed int) start_from; i--)
    {
        if ( i < 0 ) { /*fprintf("Gone negative! %u \n",i);*/ break; }

        curnum=(char) (inpt[i])-'0';
        if ((curnum>=0)&(curnum<=9))
        {
            intresult=intresult+(multiplier*curnum);
            multiplier=multiplier*10;
        }

      if (trailing_sign_seek)
       {
          /*fprintf(stderr,"Run to %c while searching for sign \n",inpt[i]);*/
          if (inpt[i]=='+') { trailing_sign_seek=0; } else
          if (inpt[i]=='-') { trailing_sign_seek=0; positive_number=0; }
       }
    }

    if (!positive_number) { intresult = intresult * (-1); }
    return intresult;
}



inline unsigned char CheckIPCOk(struct InputParserC * ipc)
{
    if ( ipc->guardbyte1.checksum != ipc->guardbyte2.checksum ) { fprintf(stderr,"Input Parser - Wrong GuardChecksums #1\n"); return 0; }
    if ( ipc->guardbyte3.checksum != ipc->guardbyte4.checksum ) { fprintf(stderr,"Input Parser - Wrong GuardChecksums #2\n"); return 0; }
    if ( ipc->guardbyte1.checksum != ipc->guardbyte4.checksum ) { fprintf(stderr,"Input Parser - Wrong GuardChecksums #3\n"); return 0; }
    if ( (ipc->tokenlist==0) ||  (ipc->tokens_max<ipc->tokens_count) ) { fprintf(stderr,"Input Parser - Tokenlist error\n"); return 0; }
    if ( (ipc->delimeters==0) || (ipc->max_delimeter_count<ipc->cur_delimeter_count) ) { fprintf(stderr,"Input Parser - Delimeter error\n"); return 0; }

    return 1;
}


void InputParser_DefaultDelimeters(struct InputParserC * ipc)
{
    if ( CheckIPCOk(ipc) == 0 ) { return; }
    if ( ipc->delimeters == 0 ) { return; }

    int i;
    /*fprintf(stderr,"Default Delimters ( %u ) ",ipc->max_delimeter_count);*/
    for ( i=0; i<ipc->max_delimeter_count; i++)
     {
         switch (i)
         {
           case 0 : ipc->delimeters[0]='\n'; break;
           case 1 : ipc->delimeters[1]=',';  break;
           case 2 : ipc->delimeters[2]='=';  break;
           case 3 : ipc->delimeters[3]='(';  break;
           case 4 : ipc->delimeters[4]=')';  break;
           default : ipc->delimeters[i]='\n'; break;
         };

         /*fprintf(stderr," %u - %c ",i,ipc->delimeters[i]);*/

     }
   ipc->cur_delimeter_count = ipc->max_delimeter_count;
   /*fprintf(stderr,"\n");*/

   return;
}

/*
  InitInputParser..

  Allocates an ipc structure and with memory to it according to options for future use!
  for example InitInputParser( 256 , 5 ) will allocate an InputParser Instance capeable of tokenizing up to 256 different words
  from 5 delimeters
*/
struct InputParserC * InputParser_Create(unsigned int max_string_count,unsigned int max_delimiter_count)
{
    struct InputParserC * ipc=0;

    ipc = ( struct InputParserC * ) malloc ( sizeof ( struct InputParserC ) );
    if ( ipc  == 0 ) { fprintf(stderr,"InputParserC unable to commit memory for a new instance\n"); return 0; }

    ipc->tokenlist = (struct tokens *) malloc( sizeof(struct tokens) * (max_string_count+1) );
    if ( ipc->tokenlist  == 0 ) { fprintf(stderr,"InputParserC unable to commit memory for a new Token List\n"); return 0; }
    ipc->tokens_count=0;
    ipc->tokens_max = max_string_count;


    ipc->delimeters = (char *) malloc( sizeof(char) * (max_delimiter_count+1) );
    if ( ipc->delimeters  == 0 ) { fprintf(stderr,"InputParserC unable to commit memory for a new Delimeter List\n"); return 0; }
    ipc->max_delimeter_count=max_delimiter_count;
    ipc->cur_delimeter_count=max_delimiter_count;


    ipc->container_start=0;
    ipc->container_end=0;

    ipc->guardbyte1.checksum=66666;
    ipc->guardbyte2.checksum=66666;
    ipc->guardbyte3.checksum=66666;
    ipc->guardbyte4.checksum=66666;

    ipc->local_allocation = 0; /* No allocation by default! :)*/
    ipc->str_length=0;
    ipc->str=0;

    InputParser_DefaultDelimeters(ipc);

    return ipc;
}

/*
  InputParser_Destroy..

  Safely destroys a non null ipc structure and deallocates all commited memory
*/
void InputParser_Destroy(struct InputParserC * ipc)
{
    if ( ipc == 0 ) { return; }

    /*fprintf(stderr,"InputParserC freeing delimeters\n");*/
    if ( ipc->delimeters != 0 )
    {
     free(ipc->delimeters);
     /*ipc->delimeters=0;*/
    }
    ipc->cur_delimeter_count=0;
    ipc->max_delimeter_count=0;

/*
    if ( ipc->container_start != 0 )
     {
       free(ipc->container_start);
       ipc->container_start=0;
     }
    if ( ipc->container_end != 0 )
     {
       free(ipc->container_end);
       ipc->container_end=0;
     }
     */
    ipc->cur_container_count = 0;
    ipc->max_container_count = 0;

    /*fprintf(stderr,"InputParserC freeing tokenlist\n");*/
    if ( ipc->tokenlist != 0 )
     {
      free(ipc->tokenlist);
      /*ipc->tokenlist=0;*/
     }
    ipc->tokens_max=0;
    ipc->tokens_count=0;

    /*fprintf(stderr,"InputParserC freeing local allocation\n");*/
    if ( ipc->local_allocation == 1 ) { if ( ipc->str!=0 ) free(ipc->str); }
    /*ipc->local_allocation=0;*/
    ipc->str_length=0;



    /* RESULT */
    ipc->guardbyte1.checksum = 0;
    ipc->guardbyte2.checksum = 0;
    ipc->guardbyte3.checksum = 0;
    ipc->guardbyte4.checksum = 0;

    /*fprintf(stderr,"InputParserC freeing ipc\n");*/
    free(ipc);


}

/* ........................................................
 Delimeters ........................................................
 ........................................................*/

/*
   CheckDelimeterNumOk..
   Checks if delimeter with number num has allocated space in memory
*/
inline unsigned char CheckDelimeterNumOk(struct InputParserC * ipc,int num)
{
    if ( ipc->max_delimeter_count <= num ) return 0;
    return 1;
}

/*
   SetDelimeter..
   Sets Delimeter number num with value tmp
*/
void InputParser_SetDelimeter(struct InputParserC * ipc,int num,char tmp)
{
    if (CheckDelimeterNumOk(ipc,num)==0) { return;}
    ipc->delimeters[num]=tmp;
}

/*
   GetDelimeter..
   Returns value of  Delimeter with number num
*/
char InputParser_GetDelimeter(struct InputParserC * ipc,int num)
{
    if (CheckDelimeterNumOk(ipc,num)==0) { return 0;}
    return ipc->delimeters[num];
}
/* .......................................................................
 .......................................................................
 .......................................................................*/




/*
   Selfchecks IPC instance and returns 1 if everything is ok , 0 if error..
*/
unsigned char InputParser_SelfCheck(struct InputParserC * ipc)
{
    if (CheckIPCOk(ipc)==0) { fprintf(stderr,"\n\n!!!!\nThis instance of InputParser is broken\n!!!!\n\n");
                              return 0; }
    return 1;
}

/*
   CheckWordNumOk..
   Checks if word with number num has allocated space in memory
*/
inline unsigned char CheckWordNumOk(struct InputParserC * ipc,unsigned int num)
{
  if ( CheckIPCOk(ipc)==0) { return 0; }
  if ( (ipc->tokenlist==0) || ( ipc->tokens_count <= num ) ) { return 0; }


  return 1;
}

/*
   InputParser_GetWord..
   Copies token with number (num) to c string (wheretostore) , variable storagesize contains the total size of wheretostore..!
*/
unsigned int InputParser_GetWord(struct InputParserC * ipc,unsigned int num,char * wheretostore,unsigned int storagesize)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }
    if ( storagesize < ipc->tokenlist[num].length+1 ) /* +1 gia to \0 */ return 0;


    unsigned int i=0;
    for ( i = ipc->tokenlist[num].token_start; i<ipc->tokenlist[num].token_start+ipc->tokenlist[num].length; i++ )
    wheretostore[i-ipc->tokenlist[num].token_start] = ipc->str[i];

    wheretostore[ipc->tokenlist[num].length] = 0;

    return ipc->tokenlist[num].length;
}


/*
   InputParser_WordCompareNoCase..
   Compares word (word) with token with number (num) , null terminating character is not required , NO CASE SENSITIVITY..!
*/
unsigned char InputParser_WordCompareNoCase(struct InputParserC * ipc,unsigned int num,char * word,unsigned int wordsize)
{
    /*fprintf(stderr,"InputParser_WordCompareNoCase( %u , %s , %u )",num,word,wordsize);*/
    if ( wordsize != InputParser_GetWordLength(ipc,num) ) { return 0; }
    /*if (  ipc->str_length <= ipc->tokenlist[num].token_start+wordsize ) { fprintf(stderr,"Erroneous input on InputParser_WordCompareNoCase leads out of array \n"); return 0; }*/

    unsigned int i=0;
    for ( i=0; i<wordsize; i++ )
    {
      if (toupper(ipc->str[ipc->tokenlist[num].token_start+i])!=toupper(word[i])) { /*fprintf(stderr," returning fail ");*/  return 0; }
    }

    /*fprintf(stderr," returning success ");*/
    return 1;
}

/*
   InputParser_WordCompareNoCase..
   Compares word (word) with token with number (num) , null terminating character is required , NO CASE SENSITIVITY..!
*/
unsigned char InputParser_WordCompareNoCaseAuto(struct InputParserC * ipc,unsigned int num,char * word)
{
    if (word==0) { return 0; }
    unsigned int wordsize=strlen(word);
    return InputParser_WordCompareNoCase(ipc,num,word,wordsize);
}



/*
   InputParser_WordCompare..
   Compares word (word) with token with number (num) , null terminating character is not required..!
*/
unsigned char InputParser_WordCompare(struct InputParserC * ipc,unsigned int num,char * word,unsigned int wordsize)
{
    if ( wordsize != InputParser_GetWordLength(ipc,num) ) { return 0; }
    /*if (  ipc->str_length <= ipc->tokenlist[num].token_start+wordsize ) { fprintf(stderr,"Erroneous input on InputParser_WordCompareNoCase leads out of array \n"); return 0; }*/

    unsigned int i=0;
    for ( i=0; i<wordsize; i++ )
    {
      if (ipc->str[ipc->tokenlist[num].token_start+i]!=word[i]) {  return 0; }
    }
    return 1;
}

/*
   InputParser_WordCompareNoCase..
   Compares word (word) with token with number (num) , null terminating character is required , NO CASE SENSITIVITY..!
*/
unsigned char InputParser_WordCompareAuto(struct InputParserC * ipc,unsigned int num,char * word)
{
    if (word==0) { return 0; }
    unsigned int wordsize=strlen(word);
    return InputParser_WordCompare(ipc,num,word,wordsize);
}


/*
   InputParser_GetUpcaseWord..
   Same with InputParser_GetWord , the result is converted to upcase..!
*/
unsigned int InputParser_GetUpcaseWord(struct InputParserC * ipc,unsigned int num,char * wheretostore,unsigned int storagesize)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }
    if ( storagesize < ipc->tokenlist[num].length+1 ) /* +1 gia to \0 */  return 0;

    unsigned int i=0;
    for ( i = ipc->tokenlist[num].token_start; i<ipc->tokenlist[num].token_start+ipc->tokenlist[num].length; i++ )
    wheretostore[i-ipc->tokenlist[num].token_start] = toupper(ipc->str[i]);

    wheretostore[ipc->tokenlist[num].length] = 0;

    return ipc->tokenlist[num].length;
}

/*
   InputParser_GetUpcaseWord..
   Same with InputParser_GetWord , the result is converted to lowercase..!
*/
unsigned int InputParser_GetLowercaseWord(struct InputParserC * ipc,unsigned int num,char * wheretostore,unsigned int storagesize)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }
    if ( storagesize < ipc->tokenlist[num].length+1 ) /* +1 gia to \0 */  return 0;

    unsigned int i=0;
    for ( i = ipc->tokenlist[num].token_start; i<ipc->tokenlist[num].token_start+ipc->tokenlist[num].length; i++ )
    wheretostore[i-ipc->tokenlist[num].token_start] = tolower(ipc->str[i]);

    wheretostore[ipc->tokenlist[num].length] = 0;

    return ipc->tokenlist[num].length;
}


/*
   InputParser_GetChar..
   Returns character (pos) from token (num)..!
*/
char InputParser_GetWordChar(struct InputParserC * ipc,unsigned int num,unsigned int pos)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }

    if ( pos>=ipc->tokenlist[num].token_start+ipc->tokenlist[num].length ) { return 0; }

    return ipc->str[ipc->tokenlist[num].token_start + pos ];
}

/*
   InputParser_GetWordInt..
   Same with InputParser_GetWord , if the result can be converted to a number , it returns this number
   else 0 is returned
*/
signed int InputParser_GetWordInt(struct InputParserC * ipc,unsigned int num)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }
    return Str2Int_internal(ipc->str,ipc->tokenlist[num].token_start,ipc->tokenlist[num].length);
}


/*
   InputParser_GetWordFloat..
   Same with InputParser_GetWord , if the result can be converted to a float number , it returns this number
   else 0.0 is returned
*/
float InputParser_GetWordFloat(struct InputParserC * ipc,unsigned int num)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0.0; }
    if (ipc->tokenlist[num].length == 0 ) { return 0.0; }


  float return_value=0.0;
  //Our string is a "string_segment" , and its last character ( which will be temporary become null ) is last_char_of_string_segment
  char * string_segment = ipc->str+ipc->tokenlist[num].token_start;
  char * last_char_of_string_segment = string_segment + ipc->tokenlist[num].length;
  char remember = 0;

   if (ipc->tokenlist[num].token_start + ipc->tokenlist[num].length < ipc->str_length)
   {
    remember = *last_char_of_string_segment;
    *last_char_of_string_segment = (char) 0; //Temporarily convert the string segment to a null terminated string
   }
   //else we are on the last part of the string so no reason to do the whole 0 remember thing..

    sscanf(string_segment,"%f",&return_value);


   if (ipc->tokenlist[num].token_start + ipc->tokenlist[num].length < ipc->str_length)
   {
    *last_char_of_string_segment = remember; //Restore string..
   }


  return return_value;
}


/*
   InputParser_GetChar..
   Returns total length of token (num)..!
*/
unsigned int InputParser_GetWordLength(struct InputParserC * ipc,unsigned int num)
{
    if ( CheckWordNumOk(ipc,num) == 0 ) { return 0; }
    return ipc->tokenlist[num].length;
}

/*
   InputParser_SeperateWords..
   Seperates words in c string (inpt) to tokens using delimiters set-up at structure ipc and keeps result at structure ipc
   if the c string will be erased before getting back the tokens you can set the keepcopy byte to 1 , this will allocate memory to
   keep a copy of the string..!
   the number returned is the total of tokens extracted!
*/
int InputParser_SeperateWords(struct InputParserC * ipc,char * inpt,char keepcopy)
{

  if (CheckIPCOk(ipc)==0) { return 0; }
  if  ( inpt == 0 ) return 0; /* NULL INPUT -> NULL OUTPUT*/

  unsigned int   STRING_END = strlen(inpt) ;
  int WORDS_SEPERATED = 0 , NEXT_SHOULD_NOT_BE_A_DELIMITER=1 , FOUND_DELIMETER ; /* Ignores starting ,,,,,string,etc*/

  if ( STRING_END == 0 ) { return 0; } /* NULL INPUT -> NULL OUTPUT pt 2*/

  /* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
   COPY STRING ( OR POINTER ) TO IPC STRUCTURE*/
  if ( keepcopy == 1 ) {  /* IF WE HAVE ALREADY ALLOCATED A STRING TO ipc->str , we should free it to prevent memory leaks!*/
                          if (ipc->local_allocation == 1)
                          {
                            if (ipc->str!=0)
                            { /* ipc->str contains a previous value!*/
                              free(ipc->str);
                              ipc->local_allocation = 0;
                            }
                          }
                          ipc->str = (char * ) malloc( sizeof(char) * (STRING_END+1) );
                          ipc->local_allocation = 1;
                          strncpy( ipc->str , inpt , STRING_END ) ;
                       } else
                       { ipc->str = inpt; }

  ipc->str_length = STRING_END;
  /* COPY STRING ( OR POINTER ) TO IPC STRUCTURE
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/


  register unsigned int i,z;
  ipc->tokens_count = 0 , ipc->tokenlist[0].token_start=0;
  for (i=0; i<STRING_END; i++)
  {
    FOUND_DELIMETER = 0;
    for (z=0; z<ipc->cur_delimeter_count; z++)
    {

    if ( inpt[i] == ipc->delimeters[z] )
      {
        FOUND_DELIMETER = 1;
        if (NEXT_SHOULD_NOT_BE_A_DELIMITER==0)
        {
         ipc->tokenlist[ipc->tokens_count].length = i - ipc->tokenlist[ipc->tokens_count].token_start;
         ipc->tokens_count+=1;
         ipc->tokenlist[ipc->tokens_count].token_start = i+1;
         WORDS_SEPERATED+=1;
         break;
        } else
        {
          ipc->tokenlist[ipc->tokens_count].token_start=i+1;
        }
      }
    }

    if (FOUND_DELIMETER == 0 ) NEXT_SHOULD_NOT_BE_A_DELIMITER=0; else
    if (FOUND_DELIMETER == 1 ) NEXT_SHOULD_NOT_BE_A_DELIMITER=1;

  }

   if (NEXT_SHOULD_NOT_BE_A_DELIMITER==0)
        {
         ipc->tokenlist[ipc->tokens_count].length = i - ipc->tokenlist[ipc->tokens_count].token_start;
         ipc->tokens_count+=1;
         ipc->tokenlist[ipc->tokens_count].token_start = i+1;
         WORDS_SEPERATED+=1;

        } else
  ipc->tokenlist[ipc->tokens_count].length = i - ipc->tokenlist[ipc->tokens_count].token_start;

  return WORDS_SEPERATED;
}

/*
 Same with InputParser_SeperateWords it does the (char *) type cast
*/
int InputParser_SeperateWordsCC(struct InputParserC * ipc,const char * inpt,char keepcopy)
{
  return InputParser_SeperateWords(ipc,(char * )inpt,keepcopy);
}

/*
 Same with InputParser_SeperateWords it does the (char *) type cast
*/
int InputParser_SeperateWordsUC(struct InputParserC * ipc,unsigned char * inpt,char keepcopy)
{
  return InputParser_SeperateWords(ipc,(char * )inpt,keepcopy);
}
