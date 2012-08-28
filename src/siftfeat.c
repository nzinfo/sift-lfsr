/*
  This program detects image features using SIFT keypoints. For more info,
  refer to:
  
  Lowe, D. Distinctive image features from scale-invariant keypoints.
  International Journal of Computer Vision, 60, 2 (2004), pp.91--110.
  
  Copyright (C) 2006-2010  Rob Hess <hess@eecs.oregonstate.edu>

  Note: The SIFT algorithm is patented in the United States and cannot be
  used in commercial products without a license from the University of
  British Columbia.  For more information, refer to the file LICENSE.ubc
  that accompanied this distribution.

  Version: 1.1.2-20100521
*/

#include "sift.h"
#include "imgfeatures.h"
#include "utils.h"
#include <highgui.h>
#include <unistd.h>
//#include <papi.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h> 
#include <dirent.h> 

#define GET_TIME(start, end, duration)                                     \
   duration.tv_sec = (end.tv_sec - start.tv_sec);                         \
   if (end.tv_usec >= start.tv_usec) {                                     \
      duration.tv_usec = (end.tv_usec - start.tv_usec);                   \
   }                                                                       \
   else {                                                                  \
      duration.tv_usec = (1000000L - (start.tv_usec - end.tv_usec));   \
      duration.tv_sec--;                                                   \
   }                                                                       \
   if (duration.tv_usec >= 1000000L) {                                  \
      duration.tv_sec++;                                                   \
      duration.tv_usec -= 1000000L;                                     \
   }
#define OPTIONS ":o:m:i:s:c:r:n:b:dxh"

#define EVN_NUM 4
//#define USE_PAPI 1
#define FISH 1

/*************************** Function Prototypes *****************************/

static void usage( char* );
static void arg_parse( int, char** );

/******************************** Globals ************************************/

char* pname;
char* img_file_name;
char* out_file_name = NULL;
char* out_img_name = NULL;
int intvls = SIFT_INTVLS;
double sigma = SIFT_SIGMA;
double contr_thr = SIFT_CONTR_THR;
int curv_thr = SIFT_CURV_THR;
int img_dbl = SIFT_IMG_DBL;
int descr_width = SIFT_DESCR_WIDTH;
int descr_hist_bins = SIFT_DESCR_HIST_BINS;
int display = 0;


/********************************** Main *************************************/

int main( int argc, char** argv )
{
#ifdef USE_PAPI
  long_long values1[EVN_NUM], values2[EVN_NUM];
  
  if(PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
	  printf("library init error\n");
int EventCodes[EVN_NUM]={PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_LD_INS, PAPI_SR_INS};
if(PAPI_query_event(PAPI_L2_ICA) != PAPI_OK)
	 printf("not support\n");
int start_err;
  if((start_err=PAPI_start_counters((int*)EventCodes, EVN_NUM)) != PAPI_OK){
	  if(start_err==PAPI_EINVAL)
		  printf("1");
	  else if(start_err==PAPI_ESYS)
		  printf("2");
	  else if(start_err==PAPI_ENOMEM)
		  printf("3");
	  else if(start_err==PAPI_EISRUN)
		  printf("4");
	  else if(start_err==PAPI_ECNFLCT)
		  printf("5");
	  else if(start_err==PAPI_ENOEVNT)
		  printf("6");

	  printf("start error\n");
  }

  int read_err;	
 
  if((read_err = PAPI_read_counters(values1, EVN_NUM)) != PAPI_OK){
	  //if(read_err==PAPI_EINVAL)
		//  printf("1");
	  printf("read counters error\n");
	}
#endif
//struct timeval  bd_tick_x, bd_tick_e, bd_tick_d;
 //gettimeofday(&bd_tick_x, 0);
  IplImage* img;
  struct feature* features;
  int n = 0, count=0;

  arg_parse( argc, argv );

#ifdef FISH
  DIR *dp;
  struct dirent *dirp;

  char buf[100];
  char img_dir[100];
  char out_dir[100];
  float ix1,iy1,ix2,iy2;
  int out_num;
  FILE *input_file=fopen(img_file_name,"r");
  fgets(buf,100,input_file);
  sscanf(buf,"%s",img_dir);
  printf("%s\n",img_dir);
  fgets(buf,100,input_file);
  sscanf(buf,"%s",out_dir);
  printf("%s\n",out_dir);
  fgets(buf,100,input_file);
  sscanf(buf,"%f %f",&ix1,&iy1);
  printf("%f %f\n",ix1,iy1);
  fgets(buf,100,input_file);
  sscanf(buf,"%f %f",&ix2,&iy2);
  printf("%f %f\n",ix2,iy2);
  fgets(buf,100,input_file);
  //sscanf(buf,"%d",&out_num);
  //printf("%d\n",out_num);
  out_num=atoi(out_file_name);

  fclose(input_file);
//#ifdef zero 
  //if ( (dp = opendir(img_dir)) == NULL) 
    //error("can't open %s", img_dir);
  
  //chdir(img_dir);
  //while ( (dirp = readdir(dp)) != NULL) 
  	//if(dirp->d_type != DT_DIR){
		//fprintf(stderr, "%s \n", dirp->d_name);
		img = cvLoadImage( out_img_name, 1 );
  		if( ! img )
    		fatal_error( "unable to load image from %s", out_img_name);
 		n = _sift_features( img, &features, intvls, sigma, contr_thr, curv_thr,
		      img_dbl, descr_width, descr_hist_bins );
 		//fprintf( stderr, "%d\n", n );

  		FILE *file;
  		char* outfile=(char*)malloc(sizeof(char)*100);
  		sprintf(outfile, "%s/%d.txt", out_dir, out_num);
		
  		//fprintf(stderr, "%s\n", dirp->d_name);
  		if(!(file=fopen(outfile, "w")))
  			fprintf(stderr, "open error\n");

		fprintf(file, "%s\n", out_img_name);
 		fclose(file);
		/*
		int j=0, k=0, d=features[0].d;
   		fprintf(file, "%d %d\n", n,d);
  		for(j;j<n;j++){
			for(k;k<d-1;k++){
				fprintf(file, "%d ", (int)features[j].descr[k]);
			}
			fprintf(file, "%d\n", (int)features[j].descr[d-1]);
			k=0;
  		}
  		*/

		export_features( outfile, features, n, ix1, iy1, ix2, iy2 );
		free(outfile);
  		count++;

//	}
 // closedir(dp);
//#endif
  exit(0);
#endif
/*************************************************************/
  int i=1;
  for(i;i<2;i++){
  //fprintf( stderr, "Finding img%d SIFT features...\n", i );
  char* load_img = (char*)malloc(sizeof(char)*100);
  sprintf(load_img, "%s", img_file_name);
  img = cvLoadImage( load_img, 1 );
  if( ! img )
    fatal_error( "unable to load image from %s", img_file_name );
  n = _sift_features( img, &features, intvls, sigma, contr_thr, curv_thr,
		      img_dbl, descr_width, descr_hist_bins );
  fprintf( stderr, "%d\n", n );

  FILE *file;
  char* outfile=(char*)malloc(sizeof(char)*100);
  sprintf(outfile, "1.txt");	
  //fprintf(stderr, "%s\n", outfile);
  if(!(file=fopen(outfile, "w")))
  	fprintf(stderr, "open error\n");
 
  int j=0, k=0, d=features[0].d;
   fprintf(file, "%d %d\n", n,d);
  for(j;j<n;j++){
	for(k;k<d-1;k++){
		fprintf(file, "%d ", (int)features[j].descr[k]);
	}
	fprintf(file, "%d\n", (int)features[j].descr[d-1]);
	k=0;
  }
  fclose(file);
  
  
  if( display )
    {
      draw_features( img, features, n );
      display_big_img( img, img_file_name );
      cvWaitKey( 0 );
    }

  if( out_file_name != NULL )
    export_features( out_file_name, features, n, ix1, iy1,ix2,iy2 );

  if( out_img_name != NULL )
    cvSaveImage( out_img_name, img, NULL );
   free(load_img);
  }
   
#ifdef USE_PAPI
  if(PAPI_stop_counters(values2, EVN_NUM) != PAPI_OK)
	  printf("stop error\n");

  int ii=0;
  for(ii;ii<EVN_NUM;ii++){
  	printf("%lld\n", values2[ii]);
  }
  /*
  if(PAPI_cleanup_eventset(EventSet) != PAPI_OK)
	  erintf("\n cleanup error\n");
 
  if(PAPI_destroy_eventset(&EventSet) != PAPI_OK)
	  printf("destroy error\n");
  */	
  PAPI_shutdown();
#endif
  //gettimeofday(&bd_tick_e, 0);
  //GET_TIME(bd_tick_x, bd_tick_e, bd_tick_d);
  //cout<< "SIFT took: " << bd_tick_d.tv_sec*1000 + bd_tick_d.tv_usec/1000 << " ms" << std::endl;
  //printf("SIFT took: %d ms\n", bd_tick_d.tv_sec*1000 + bd_tick_d.tv_usec/1000);
  return 0;
}


/************************** Function Definitions *****************************/

// print usage for this program
static void usage( char* name )
{
  fprintf(stderr, "%s: detect SIFT keypoints in an image\n\n", name);
  fprintf(stderr, "Usage: %s [options] <img_file>\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h               Display this message and exit\n");
  fprintf(stderr, "  -o <out_file>    Output keypoints to text file\n");
  fprintf(stderr, "  -m <out_img>     Output keypoint image file (format" \
	  " determined by extension)\n");
  fprintf(stderr, "  -i <intervals>   Set number of sampled intervals per" \
	  " octave in scale space\n");
  fprintf(stderr, "                   pyramid (default %d)\n",
	  SIFT_INTVLS);
  fprintf(stderr, "  -s <sigma>       Set sigma for initial gaussian"	\
	  " smoothing at each octave\n");
  fprintf(stderr, "                   (default %06.4f)\n", SIFT_SIGMA);
  fprintf(stderr, "  -c <thresh>      Set threshold on keypoint contrast" \
	  " |D(x)| based on [0,1]\n");
  fprintf(stderr, "                   pixel values (default %06.4f)\n",
	  SIFT_CONTR_THR);
  fprintf(stderr, "  -r <thresh>      Set threshold on keypoint ratio of" \
	  " principle curvatures\n");
  fprintf(stderr, "                   (default %d)\n", SIFT_CURV_THR);
  fprintf(stderr, "  -n <width>       Set width of descriptor histogram" \
	  " array (default %d)\n", SIFT_DESCR_WIDTH);
  fprintf(stderr, "  -b <bins>        Set number of bins per histogram" \
	  " in descriptor array\n");
  fprintf(stderr, "                   (default %d)\n", SIFT_DESCR_HIST_BINS);
  fprintf(stderr, "  -d               Toggle image doubling (default %s)\n",
	  SIFT_IMG_DBL == 0 ? "off" : "on");
  fprintf(stderr, "  -x               Turn off keypoint display\n");
}



/*
  arg_parse() parses the command line arguments, setting appropriate globals.
  
  argc and argv should be passed directly from the command line
*/
static void arg_parse( int argc, char** argv )
{
  //extract program name from command line (remove path, if present)
  pname = basename( argv[0] );

  //parse commandline options
  while( 1 )
    {
      char* arg_check;
      int arg = getopt( argc, argv, OPTIONS );
      if( arg == -1 )
	break;

      switch( arg )
	{
	  // catch unsupplied required arguments and exit
	case ':':
	  fatal_error( "-%c option requires an argument\n"		\
		       "Try '%s -h' for help.", optopt, pname );
	  break;

	  // read out_file_name
	case 'o':
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  out_file_name = optarg;
	  break;

	  // read out_img_name
	case 'm':
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  out_img_name = optarg;
	  break;
	  
	  // read intervals
	case 'i':
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is an integer
	  intvls = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read sigma
	case 's' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  sigma = strtod( optarg, &arg_check );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires a floating point argument\n" \
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read contrast_thresh
	case 'c' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  contr_thr = strtod( optarg, &arg_check );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires a floating point argument\n" \
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read curvature_thresh
	case 'r' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  curv_thr = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read descr_width
	case 'n' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  descr_width = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read descr_histo_bins
	case 'b' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  descr_hist_bins = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read double_image
	case 'd' :
	  img_dbl = ( img_dbl == 1 )? 0 : 1;
	  break;

	  // read display
	case 'x' :
	  display = 0;
	  break;

	  // user asked for help
	case 'h':
	  usage( pname );
	  exit(0);
	  break;

	  // catch invalid arguments
	default:
	  fatal_error( "-%c: invalid option.\nTry '%s -h' for help.",
		       optopt, pname );
	}
    }

  // make sure an input file is specified
  if( argc - optind < 1 )
    fatal_error( "no input file specified.\nTry '%s -h' for help.", pname );

  // make sure there aren't too many arguments
  if( argc - optind > 1 )
    fatal_error( "too many arguments.\nTry '%s -h' for help.", pname );

  // copy image file name from command line argument
  img_file_name = argv[optind];
}
