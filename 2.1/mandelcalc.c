
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "mandelcalc.h"
/* sub-region parameters */


/* the mandelbrot test */
static int progress=0;//dilwnei pios exei sira
long double imBeg; 

static int isMandelbrot(double re, double im, int maxIterations) {
	long double zr = 0.0, zi = 0.0;
	long double xsq = 0.0, ysq = 0.0;
	long double cr = re, ci = im;
	int n = 0;
	
	while(xsq + ysq < 4.0 && n < maxIterations) {
		xsq = zr * zr;
		ysq = zi * zi;
		zi = 2 * zr * zi + ci;
		zr = xsq - ysq + cr;
		n++;
	}
	return(n);
	
}

/* perform mandelbrot computation for a sub-region */

static void computeMandelbrot(sliceMPars *p) {
	long double re,im; 
	int x,y;
	
	im = p->imBeg;
	for (y=0; y<p->imPixels; y++) {
		re = p->reBeg;
		for (x=0; x<p->rePixels; x++) {
			p->res[y*p->rePixels+x] = isMandelbrot (re,im,p->maxIterations);
			re = re + p->reStep;
		}
		im = im + p->imStep;
	}
	
	*(p->rdy)=1;
	
}

/* perform computation for an entire region */

void * calcMandel (void *Tool) {
	
	Toolbox *TOOL= (Toolbox*)Tool;
	
	long double reStep = (TOOL->mand->reEnd - TOOL->mand->reBeg) / TOOL->mand->rePixels;
	long double imStep = (TOOL->mand->imEnd - TOOL->mand->imBeg) / TOOL->mand->imPixels;
	int sliceImPixels = TOOL->mand->imPixels / TOOL->mand->slices;
	int sliceTotPixels = sliceImPixels * TOOL->mand->rePixels;
	int check, flag=0;
	
	TOOL->p = (sliceMPars *)malloc(sizeof(sliceMPars)); 
	/* allocate array for sub-region parameters */
	while (1){
		
		pthread_mutex_lock(&mtx1);	//kathe thread blokarei edw kai perimenei na ksekleidwthei (to prwto apo to mandel.c kai kathe epomeno apo to proigoumeno tou)
		flag=0;
		//arxikopoiisi metavlitwn meta apo kathe zoom
		if (*(TOOL->level)>1){
			
			reStep = (TOOL->mand->reEnd - TOOL->mand->reBeg) / TOOL->mand->rePixels;
			imStep = (TOOL->mand->imEnd - TOOL->mand->imBeg) / TOOL->mand->imPixels;
			sliceImPixels = TOOL->mand->imPixels / TOOL->mand->slices;
			sliceTotPixels = sliceImPixels * TOOL->mand->rePixels;
		}
		
		
		//arxikopoiisi imBeg meta apo kathe zoom
		if (progress==0){imBeg = TOOL->mand->imBeg;}
		
		
		
		//arxikopoiisi parametrwn
		TOOL->p->reBeg = TOOL->mand->reBeg;
		TOOL->p->reStep = reStep;
		TOOL->p->rePixels = TOOL->mand->rePixels;
		TOOL->p->imBeg = imBeg;
		
		TOOL->p->imStep = imStep;
		TOOL->p->imPixels = sliceImPixels;
		TOOL->p->maxIterations = TOOL->mand->maxIterations;
		
		TOOL->p->res = &(TOOL->res[(progress)*sliceTotPixels]);
		TOOL->p->rdy =&(TOOL->rd);
		imBeg =  imBeg + imStep*sliceImPixels;
		
		
		
		//dinoume protereotita sto epomeno thread
		progress=(++progress)%TOOL->mand->slices;
		if (progress!=0){
			pthread_mutex_unlock(&mtx1);		//kseblokaroume to epomeno thread
		}else{
			pthread_mutex_unlock (&mtx2);		//kseblokaroume ton ypologismo tou kathe TOOL->p
			
		}
		pthread_mutex_lock (&mtx2);			//blokarei kai perimenei na oloklirwthei i arxikopoihsh sto tool[i]->p
		
		
		//Ipologismos sintetagmenon
		computeMandelbrot(TOOL->p); 
		printf (" slice: %d ready\n", (1+progress));
		/* release parameter array */
		
		progress=(++progress)%TOOL->mand->slices;
		
		
		
		if (progress==0){
			pthread_mutex_unlock(&readyToDraw);	//dinoume to elefthero na sxediasei
			
			
			
		}
		else{
			pthread_mutex_unlock(&mtx2);		//kseblokaroume to proigoumeno thread
			
		}
		
	}
}
