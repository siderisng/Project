typedef struct {
  long double reBeg,reEnd;  /* range of real values */
  long double imBeg,imEnd;  /* range of imaginary values */
  int rePixels,imPixels;    /* number of pixels per range */
  int maxIterations;        /* iteration cut off value */
  int slices;               /* number of sub-computations */
} mandelPars;


typedef struct {
  long double reBeg,reStep;  
  long double imBeg,imStep;
  int rePixels,imPixels;
  int maxIterations;
  int *res; /* array where to store result values */
  int *rdy; /* set 1 when done */
} sliceMPars;
//ergaliothiki
typedef struct{
	mandelPars *mand;  //diktis sto mandelPars
	int rd,*res,count,*level;
	//rd: katastasi etimotitas
	//res:analisi
        //count:arithmos protereotitas
	//level:dixni se pia epanalipsi imaste
	sliceMPars *p;
       //diktis sto sliceMPars

}Toolbox;
/* assume imPixels % slices == 0 */

extern pthread_mutex_t mtx1, mtx2, readyToDraw;
extern void * calcMandel(void * Tool);
