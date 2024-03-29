#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>
#include <errno.h>

#include "mandelcalc.h"

#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;
pthread_mutex_t mtx1, mtx2, readyToDraw; //dilwsi mutexes


/* basic win management rountines */

static void openDisplay() {
	if (dsp == NULL) { 
		dsp = XOpenDisplay(NULL); 
	} 
}

static void closeDisplay() {
	if (dsp != NULL) { 
		XCloseDisplay(dsp); 
		dsp=NULL;
	}
}

void openWin(const char *title, int width, int height) {
	unsigned long blackC,whiteC;
	XSizeHints sh;
	XEvent evt;
	long evtmsk;
	
	whiteC = WhitePixel(dsp, DefaultScreen(dsp));
	blackC = BlackPixel(dsp, DefaultScreen(dsp));
	curC = blackC;
	
	win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);
	
	sh.flags=PSize|PMinSize|PMaxSize;
	sh.width=sh.min_width=sh.max_width=WinW;
	sh.height=sh.min_height=sh.max_height=WinH;
	XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);
	
	XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
	XMapWindow(dsp, win);
	do {
		XWindowEvent(dsp, win, StructureNotifyMask, &evt);
	} while (evt.type != MapNotify);
	
	gc = XCreateGC(dsp, win, 0, NULL);
	
}

void closeWin() {
	XFreeGC(dsp, gc);
	XUnmapWindow(dsp, win);
	XDestroyWindow(dsp, win);
}

void flushDrawOps() {
	XFlush(dsp);
}

void clearWin() {
	XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
	XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
	flushDrawOps();
	XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
	XDrawPoint(dsp, win, gc, x, WinH-y);
	flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
	XEvent evt;
	
	XSelectInput(dsp, win, ButtonPressMask);
	do {
		XNextEvent(dsp, &evt);
	} while (evt.type != ButtonPress);
	*x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
	XColor clr1,clr2;
	
	if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
		printf("failed\n"); return;
	}
	XSetForeground(dsp, gc, clr1.pixel);
	curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
	static char cname[128];
	
	if (v == maxIterations) {
		return("black");
	}
	else {
		sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
		return(cname);
	}
}



int main(int argc, char *argv[]) {
	mandelPars pars;
	int i,k,x,y,level,*res,check;
	int xoff,yoff, thrCheck;
	long double reStep,imStep;
	Toolbox **Tool; //pinakas apo diktes stin "ergaliothiki"
	pthread_t *thrP; // gia tin dimiourgia twn threads pinakas p_thread_t
	
	printf("\n");
	printf("This program starts by drawing the default Mandelbrot region\n");
	printf("When done, you can click with the mouse on an area of interest\n");
	printf("and the program will zoom at this point\n");
	printf("\n");
	printf("Press enter to continue\n");
	getchar();
	
	pars.rePixels = WinW; /* never changes */
	pars.imPixels = WinH; /* never changes */
	
	/* default mandelbrot region */
	
	pars.reBeg = (long double) -2.0;
	pars.reEnd = (long double) 1.0;
	pars.imBeg = (long double) -1.5;
	pars.imEnd = (long double) 1.5;
	
	reStep = (pars.reEnd - pars.reBeg) / pars.rePixels;
	imStep = (pars.imEnd - pars.imBeg) / pars.imPixels;
	
	printf("enter max iterations (50): ");
	scanf("%d",&pars.maxIterations);
	printf("enter no of slices: ");
	scanf("%d",&pars.slices);
	
	/* adjust slices to divide win height */
	
	while (WinH % pars.slices != 0) { pars.slices++;}
	
	/* allocate result and ready arrays */
	
	res = (int *) malloc(sizeof(int)*pars.rePixels*pars.imPixels);
	
	
	/* open window for drawing results */
	
	openDisplay();
	openWin(argv[0], WinW, WinH);
	
	
	level = 1;
	//desmevoume xwro gia to pinaka apo ergalia
	

    if (pthread_mutex_init(&mtx1, NULL) != 0)			//arxikopoihsh mutex mtx1 kai kleidwma
    {
        perror ("Mutex error");
        return 1;
    }
    pthread_mutex_lock (&mtx1);
	
    if (pthread_mutex_init(&mtx2, NULL) != 0)			//arxikopoihsh mutex mtx2 kai kleidwma
    {
        perror ("Mutex error");
        return 1;
    }
	pthread_mutex_lock (&mtx2);
	
	 if (pthread_mutex_init(&readyToDraw, NULL) != 0)		//arxikopoihsh mutex readyToDraw kai kleidwma
    {
        perror ("Mutex error");
        return 1;
    }
    pthread_mutex_lock (&readyToDraw);
	if (NULL== (Tool=(Toolbox**)malloc(sizeof(Toolbox)*pars.slices))){		//desmefsi xwrou gia to pinaka apo toolbox
		perror ("Memory Error");
	}
	
	if (NULL== (thrP=(pthread_t*)malloc(sizeof(pthread_t)*pars.slices))){		//desmefsi xwrou gia ton pinaka apo threads
		perror ("Memory Error");
	}
	//desmevoume xwro gia to kathe ergalio pou tha xriastoume
	for (i=0; i<pars.slices; i++){
		
		if (NULL== (Tool[i]=(Toolbox*)malloc(sizeof(Toolbox)*pars.slices))){	//desmefsi gia kathe tool[i]
			perror ("Memory Error");
		}
		
		
		
		if (NULL==(Tool[i]->mand=(mandelPars*)malloc(sizeof(mandelPars)))){	//desmefsi gia kathe tool[i]->mand
			
			perror ("Memory error");
		}
		Tool[i]->mand=&pars;
		
		
		
		
		if (NULL==(Tool[i]->res=(int*)malloc(sizeof(int)))){			//desmefsi gia kathe tool[i]->res
			perror ("Memory error");
		}
		Tool[i]->res= res;
		Tool[i]->count=i;
		//dinoume se kathe tool mia sira protereotitas
		Tool[i]->rd=-1;	
		//arxikopoisi ton thread
		
		Tool[i]->level= &level;
		
		

		thrCheck = pthread_create( &thrP[i], NULL,calcMandel , (void *)Tool[i]);		//dimiourgia tou kathe thread
		if(thrCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n",thrCheck);
			exit(EXIT_FAILURE);
		}
		
	}
	
	
	while (1) {
		
		clearWin();
		
		printf("computing for level %d:\n",level);
		printf("reBeg=%Lf reEnd=%Lf reStep=%Lf\n",pars.reBeg,pars.reEnd,reStep);
		printf("imBeg=%Lf imEnd=%Lf imStep=%Lf\n",pars.imBeg,pars.imEnd,imStep);
		printf("maxIterations=%d\n",pars.maxIterations);

		pthread_mutex_unlock (&mtx1); 			//kseblokaroume to prwto thread kai ksekiname ton ypologismo twn slices
		
		
		/* busywait (and draw results) until all slices done */
		//efoson kathe thread exei epitixei to skopo tou
		//sxediazoume tis sintetagmenes tou
		pthread_mutex_lock (&readyToDraw);		//blokarei edw mexri na oloklirwthei o ypologismos twn slices
		
		k=0;
		while (k!=pars.slices) {
			for (i=0; i<pars.slices; i++) {
				
				
					
				
					
					for (y=i*(pars.imPixels/pars.slices); y<(i+1)*(pars.imPixels/pars.slices); y++) {
						
						for (x=0; x<pars.rePixels; x++) {
							setColor(pickColor(res[y*pars.rePixels+x],pars.maxIterations));
							drawPoint(x,y);
							
						}
					}
					
					k++; Tool[i]->rd=2;
			
		}
		}
		
		
		
		/* get next focus/zoom point */
		
		
		getMouseCoords(&x,&y);
		xoff = WinW/2 - x;
		yoff = WinH/2 - (WinH-y);
		
		/* adjust focus */
		
		pars.reBeg = pars.reBeg - xoff*reStep;
		pars.reEnd = pars.reBeg + reStep*pars.rePixels;
		pars.imBeg = pars.imBeg - yoff*reStep;
		pars.imEnd = pars.imBeg + reStep*pars.imPixels;
		
		/* zoom in */
		
		reStep = reStep*ZoomStepFactor; 
		imStep = imStep*ZoomStepFactor;
		pars.reBeg = pars.reBeg + (pars.reEnd-pars.reBeg)/2 - reStep*pars.rePixels/2;
		pars.reEnd = pars.reBeg+reStep*pars.rePixels;
		pars.imBeg = pars.imBeg + (pars.imEnd-pars.imBeg)/2 - imStep*pars.imPixels/2;
		pars.imEnd = pars.imBeg+imStep*pars.imPixels;
		pars.maxIterations = pars.maxIterations*ZoomIterationFactor;
		
		level++;
		
	} 
	
	/* never reach this point; for cosmetic reasons */
	
	free(res);
	
	
	closeWin();
	closeDisplay();
	
}
