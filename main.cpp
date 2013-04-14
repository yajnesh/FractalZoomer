/********************************************
 * 
 * Author: Yajnesh T
 * yajnesh.t@gmail.com
 * github.com/yajnesh
 * 
 * You may freely modify/redistribute any part of this project

 * ******************************************/





//FIXME: RESIZE WINDOW DOESN'T WORK, good luck finding the bug

#include<stdio.h>
#include<math.h>

#include <stdlib.h>
#include <GL/glut.h> 
#include<complex>
#include <omp.h>// this prog is parallelized B-)

#define NUM_OF_CORES 8
using namespace std;
int width=800, height=600; // initial window size
int W=width,H=height; //i will waste two more variables for no reason! 
int windowID;

GLdouble minX = -2.2f, maxX = 0.8f, minY = -1.5f, maxY = 1.5f; // complex plane boundaries
//GLdouble minX = -0.773162f, maxX = -0.608094f, minY = -0.605595f, maxY = -0.440527f; // complex plane boundaries
//GLdouble minX = -2.0f, maxX = 2.0f, minY = -2.0f, maxY = 2.0f; // complex plane boundaries
GLdouble stepX = (maxX - minX)/(GLdouble)width; // because "max" is so mainstream,i added "maxX" for Maximum effect :P
GLdouble stepY = (maxY - minY)/(GLdouble)height;
GLdouble centreX,centreY; //current centre points 
bool mouseDown=false; //mouse button down
int zoom=0;//1=zoom In , -1=zoom Out ,0 = nothing

GLfloat black[] = {0.0f, 0.0f, 0.0f}; // black color
const int paletteSize = 128;
GLfloat palette[paletteSize][3];
GLfloat ***pixel_color; // collection of entire W*H pixel value.

bool fullScreen=false;
bool reCalculate=true;

GLdouble radius = 5.0f;
int colorDelta=0;
double cycleDelta=1.6f;
GLdouble ZoomFactor=0.1; // amount of zoomin and zoomout
GLdouble  julia_factor=0.35;
GLdouble julia_delta=0.000400;

void rotate(int angle);
void reDraw(void);
void createPalette(void);
int mandel_power=2; //z^2+C
int fract_count=1; 


void allocateMemory() {

	int i;
			
	pixel_color= (GLfloat ***) malloc (H*sizeof(GLfloat**));

	// comma is my style
	if(!pixel_color) printf("Memory Allocation Failed"),exit(0);

	for(i=0;i<H;i++)
		if(!(pixel_color[i]= (GLfloat **) malloc (W*sizeof(GLfloat*)))) printf("Memory Allocation Failed"),exit(0);
		
		
	printf("\nAllocated memory for W=%d H=%d\n",W,H);
}
void freeMemory() {
	int i;
	//being a good boy! 
	for(i=0;i<H;i++)
		free(pixel_color[i]);
	free(pixel_color);
	
	printf("\nDE-allocated memory for W=%d H=%d\n",W,H);
}
GLfloat *mandelbrot2(GLdouble u, GLdouble v)
{
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
		{
			tempRe =  re*re - im*im + u;
			im = re * im * 2 + v;
			re = tempRe;
			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
		}
		return black;
}
GLfloat *mandelbrot3(GLdouble u, GLdouble v)
{

	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;
	
	for(int i=0; i < paletteSize; i++)
		{
			tempRe = re*re*re - re*im*im - 2*re*im*im + u;
			im = re*re*im -im*im*im+ 2*re*re*im+ v;
			re = tempRe;
			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
		}
		return black;
}
GLfloat *mandelbrotN(GLdouble u, GLdouble v)
{

	GLdouble re = u;
	GLdouble im = v;
	GLdouble r,theta,power_r,theta_r;

	for(int i=0; i < paletteSize; i++)
	{
		r=sqrt(re*re + im*im);
		theta=atan(im/re);

		power_r=pow(r,mandel_power);
		theta_r=theta*mandel_power;
			
		re=power_r*cos(theta_r) +u;
		im=power_r*sin(theta_r) +v;
			
			
		if( (re*re + im*im) > radius )
		{
			return palette[i];
		}
	}
		
		return black;
}
GLfloat *burningShip(GLdouble u, GLdouble v)
{
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
	{
		tempRe = fabs( re*re - im*im + u);
		im = fabs(re * im * 2 + v);
		re = tempRe;
		if( (re*re + im*im) > radius )
		{
			return palette[i];
		}
	}
	return black;
}
GLfloat *Z2mZpC(GLdouble u, GLdouble v)
{
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
	{
		tempRe =  re*re - im*im + u -re;
		im = re * im * 2 + v -im;
		re = tempRe;
		if( (re*re + im*im) > radius )
		{
			return palette[i];
		}
	}
	return black;
}

GLfloat *Z2pCdZmC(GLdouble u, GLdouble v)
{
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0,d;

	for(int i=0; i < paletteSize; i++)
	{
		d=(re+u)*(re+u) + (im+v)*(im+v);
		

		tempRe =  re*re - im*im + u ;
		im = (re * im * 2 + v)/d ;
		re = tempRe/d;


		
		if( (re*re + im*im) > radius )
		{
			return palette[i];
		}
	}
	return black;
}
GLfloat *Z3mZ2pZpc(GLdouble u, GLdouble v)
{

	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;
	

			tempRe =  re*re - im*im + u;
			im = re * im * 2 + v;
			re = tempRe;


	for(int i=0; i < paletteSize; i++)
	{
			tempRe = re*re*re - re*im*im - 2*re*im*im -( re*re - im*im) + u;
			im = re*re*im -im*im*im+ 2*re*re*im -(re * im * 2)+ v;
			re = tempRe;


			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
	}
		return black;
}

GLfloat *Z2SRZpCZCIZ(GLdouble u, GLdouble v)
{
		//(Z^2)sin(Re Z) + C Z cos(Im Z) + C
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
	{
			tempRe =  re*re - im*im ;
			im = (re * im * 2 )*sin(re) + (v*im*cos(im)) +v;
			re = tempRe*sin(re) + u*re*cos(im) +u;
			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
	}
		return black;
}

	
GLfloat *Oh_fish(GLdouble u, GLdouble v)
{
		//(Z^2)sin(Re Z) + C Z cos(Im Z) + C
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
	{
			tempRe =  re*re - im*im ;
			im = (re * im * 2 )*cos(re) + (v*im*sin(im)) +v;
			re = tempRe*cos(re) + u*re*sin(im) +u;
			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
	}
		return black;
}

GLfloat *julia(GLdouble u, GLdouble v)
{
	GLdouble re = u;
	GLdouble im = v;
	GLdouble tempRe=0.0;

	for(int i=0; i < paletteSize; i++)
		{
			tempRe =  re*re - im*im + julia_factor;
			im = re * im * 2 + julia_factor;
			re = tempRe;
			if( (re*re + im*im) > radius )
			{
				return palette[i];
			}
		}
		return black;
}




GLfloat* calculateColor(GLdouble u, GLdouble v)
{
	switch(fract_count)
	{
		case 1: //MandelBrot
			switch(mandel_power)
			{
						
				case 2:	return(mandelbrot2(u,v));	//optimized ^2 & ^3 mandelbraot
				case 3: return(mandelbrot3(u,v));
				default: return(mandelbrotN(u,v)); //genereralized mandelbrot, REAL CPU killer, takes a lot of time.... :-(
			}
		break;

		case 2:
			
			return(julia(u,v));		
			break;
			
		case 3:
		
			return(burningShip(u,v));
			
			break;

			
		case 4: 
			return(Z2mZpC(u,v)); //  Z^2 - Z + C
			
			break;
		case 5:
			return(Z3mZ2pZpc(u,v)); // Z^3 -Z^2 +Z +C
			break;
		case 6:
			return(Z2SRZpCZCIZ(u,v)); //(Z^2)sin(Re Z) + C Z cos(Im Z) + C
			break;
		case 7:
			return(Oh_fish(u,v));		//(Z^2)cos(Re Z) + C Z sin(Im Z) + C
			break;
		case 8:
			return(Z2pCdZmC(u,v)); //(Z^2 + C) / (Z-C)   ; lol nice short hand.. thank u thanku..
			break;
		

		default: printf("\n Not defined Yet\n");
				return(mandelbrot2(u,v)); 
	}

}


//****************************************
void repaint()
{   // function called to repaint the window
	int i,j;
	GLdouble x ,y;
	
	//The calculation is separated from coloring process in order to effieciently manage color-cycling B-)
	// No need to calculate the entire fractal again, just to change the color.
	
	if(reCalculate) {
	
		stepX = (maxX-minX)/(GLdouble)W; // calculate new value of step along X axis
		stepY = (maxY-minY)/(GLdouble)H; // calculate new value of step along Y axis


		//and, add one tea-spoon of parallelization for taste
		omp_set_num_threads(NUM_OF_CORES);
		
		#pragma omp parallel for schedule(dynamic, 100) private(i,j,x,y)
		for(i=0;i<height;i++) {
				
				y=minY+i*stepY; //are you a math-Wizard? please remove this heavy multiplication :(
				
				for(x = minX,j=0; x <= maxX; x += stepX) 
					pixel_color[i][j++]=calculateColor(x,y);	
		}
	
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen buffer
	
	glBegin(GL_POINTS); // start drawing in single pixel mode

	for(i=0;i<height;i++) 
		for(j=0;j<width;j++) {
			glColor3fv(pixel_color[i][j]+colorDelta); // set color
			glVertex3f(minX+j*stepX, minY+i*stepY, 0.0f); // put pixel on screen (buffer) - [ 1 ]
	}
			
	
	
	glEnd(); // end drawing
	
	glutSwapBuffers(); // swap the buffers - [ 2 ]

	printf("\nrefreshed, recalculate= %d\n",reCalculate);
	

	
	
}

//****************************************
void reshape (int w, int h)
{ // function called when window size is changed

	freeMemory();
	reCalculate=true;
	W=w; H=h;
	allocateMemory();
	stepX = (maxX-minX)/(GLdouble)w; // calculate new value of step along X axis
	stepY = (maxY-minY)/(GLdouble)h; // calculate new value of step along Y axis
	glViewport (0, 0, (GLsizei)w, (GLsizei)h); // set new dimension of viewable screen
	//printf("\n %f, %f\n",stepX,stepY);
	
	
	glutPostRedisplay(); // repaint the window
}

//****************************************
void keyFunction(unsigned char key, int x, int y)
{ // function to handle key pressing
	switch(key)
	{
		case 'F': // pressing F is turning on/off fullscreen mode, 
		case 'f':freeMemory();
				if(fullScreen)
				{
					printf("\nExiting Full Screen");
					glutReshapeWindow(width,height); // sets default window size
					GLsizei windowX = (glutGet(GLUT_SCREEN_WIDTH)-width)/2;
					GLsizei windowY = (glutGet(GLUT_SCREEN_HEIGHT)-height)/2;
					glutPositionWindow(windowX, windowY); // centers window on the screen
					fullScreen = false;
				}
				else
				{
					printf("\nEntering Full Screen");
					fullScreen = true;
					
					glutFullScreen(); // go to fullscreen mode
				}
				reCalculate=true;
				
				allocateMemory();
				glutPostRedisplay();
				break;
		case 27 : // escape key - close the program
				printf("\nQuit");
				freeMemory();
				glutDestroyWindow(windowID);
				exit(0); // Any other decent way to exit?
				break;
		case 'R':	
			    printf("\n Hard Reset");
				minX = -2.2f, maxX = 0.8f, minY = -1.5f, maxY = 1.5;
				radius = 5.0f;
				colorDelta=0;
				ZoomFactor=0.1; 
				zoom=0;
				mandel_power=2;
				fract_count=1;
				reCalculate=true;

				
				cycleDelta=1.6;
				freeMemory();
				allocateMemory();
				createPalette();
				reDraw();
				break;
		case 'r': 
				printf("\n Reset");
				minX = -2.2f, maxX = 0.8f, minY = -1.5f, maxY = 1.5;
				zoom=0;

				reDraw();
				reCalculate=true;
				break;
		case 'Z': 
				ZoomFactor+=0.01;
				printf("\n ZoomFactor+ %lf",ZoomFactor);
				break;
		case 'z':
				ZoomFactor-=0.01; 
				printf("\n ZoomFactor+ %lf",ZoomFactor);
				break;
		case 'C':
				radius+=0.1;
				printf("\n Radius+ %lf",radius);
				reCalculate=true;
				reDraw();
				break;
		case 'c':
				radius-=0.1;
				printf("\n Radius- %lf",radius);
				reCalculate=true;
				reDraw();
				break;
		case 'Q':
				colorDelta+=1;
				printf("\n colorDelta+ %d",colorDelta);
				reCalculate=false;
				reDraw();
				break;
		case 'q':
				colorDelta-=1;
				printf("\n colorDelta- %d",colorDelta);
				reCalculate=false;
				reDraw();
				break;
		case 'W':
				cycleDelta+=0.8;
				printf("\n cycleDelta+ %lf",cycleDelta);
				createPalette();
				reCalculate=false;
				reDraw();
				break;
		case 'w':
				cycleDelta-=0.8;
				printf("\n cycleDelta- %lf",cycleDelta);
				createPalette();
				reCalculate=false;
				reDraw();
				break;
		case '.':
				fract_count=1; //mandelbrot
				mandel_power++;
				printf("\n mandel_power+ %d",mandel_power);
				reCalculate=true;
				reDraw();
				break;
		case ',':
				fract_count=1; //mandelbrot
				if(--mandel_power<2) mandel_power=2 ;
				printf("\n mandel_power- %d",mandel_power);
				reCalculate=true;
				reDraw();
				break;
		case ']':
				fract_count++; 
				printf("\n fract_count+ %d",fract_count);
				reCalculate=true;
				reDraw();
				break;
		case '[':
				fract_count--;
				printf("\n fract_count- %d",fract_count);
				reCalculate=true;
				reDraw();
				break;
		case '>':
				fract_count=2; //julia
				julia_factor+=julia_delta;
				printf("\n julia_factor+ %lf",julia_factor);
				reCalculate=true;
				reDraw();
				break;
		case '<':
				fract_count=2; //julia
				julia_factor-=julia_delta;
				printf("\n julia_factor- %lf",julia_factor);
				reCalculate=true;
				reDraw();
				break;
		case '`':
				julia_delta+=0.0001;
				printf("\n julia_delta+ %lf",julia_delta);
				break;
		case '~':
				
				julia_delta-=0.0001;
				printf("\n julia_delta+ %lf",julia_delta);
				break;
				

		}
}


void createPalette()
{
	int j;
	//FILE *fp=fopen("delme.txt","w");


	for(int i=0; i < 32; i++)
	{
		j=i;
		i+=cycleDelta;
		palette[j][0] = fmod((8*i)/(GLdouble)255 ,1.0)  ;
		palette[j][1] = fmod((128-4*i)/(GLdouble)255 ,1.0)    ;
		palette[j][2] = fmod((255-8*i)/(GLdouble)255 ,1.0)   ;
		
		palette[32+j][0] = (GLdouble)1   ;
		palette[32+j][1] = fmod((8*i)/(GLdouble)255 ,1.0)   ;
		palette[32+j][2] = fmod((GLdouble)0  ,1.0)  ;
		
		palette[64+j][0] = fmod((128-4*i)/(GLdouble)255 ,1.0)   ;
		palette[64+j][1] = (GLdouble)1    ;
		palette[64+j][2] = fmod((8*i)/(GLdouble)255 ,1.0)   ;
	
		palette[96+j][0] = (GLdouble)0    ;
		palette[96+j][1] = fmod((255-8*i)/(GLdouble)255 ,1.0)   ;
		palette[96+j][2] = fmod((8*i)/(GLdouble)255 ,1.0)   ;
	
		i=j;
	//	fprintf(fp,"\n %f %f %f %f %f %f %f %f %f %f %f %f ",palette[i][0],palette[i][1],palette[i][2],palette[i+32][0],palette[i+32][1],palette[i+32][2],palette[i+64][0],palette[i+64][1],palette[i+64][2],palette[i+96][0],palette[i+96][1],palette[i+96][2]);
	}


}
void IdleFunction()
{
	if(mouseDown==true)
	{
		if(zoom==1)
		{
			minX+=ZoomFactor;
			maxX-=ZoomFactor;
			minY+=ZoomFactor;
			maxY-=ZoomFactor;
		}
		else if(zoom==-1)
		{
			minX-=ZoomFactor;
			maxX+=ZoomFactor;
			minY-=ZoomFactor;
			maxY+=ZoomFactor;
		}
		reCalculate=true;
		reDraw();
	}

}


void mouse(int btn, int state, int x, int y)
{

	y=H-y; //invert y-axis

	
	reCalculate=true;
	
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		printf("\nZoom In");
		mouseDown = true;
		zoom=1;
	}
	else 
	if(btn==GLUT_MIDDLE_BUTTON&&state==GLUT_DOWN) 
	{
		printf("\nPan");
		centreX=minX+stepX*x;
		centreY=minY+stepY*y;

		minX=centreX - (maxX-minX)/2.0f;
		maxX=centreX + (maxX-minX)/2.0f;
		minY=centreY - (maxY-minY)/2.0f;
		maxY=centreY + (maxY-minY)/2.0f;
		zoom=0;
		reDraw();
	}
	else
	if(btn==GLUT_RIGHT_BUTTON&& state==GLUT_DOWN) 
	{
		printf("\n Zoom Out");
		mouseDown = true;
		zoom=-1;
	}
	else
		mouseDown = false;

	printf("\n x %d,y %d,minX %lf,maxX %lf,minY %lf,maxY %lf",x,y,minX, maxX, minY, maxY);
}



void reDraw()
{
	glLoadIdentity();
	glOrtho(minX, maxX, minY, maxY, ((GLdouble)-1), (GLdouble)1);
	glutPostRedisplay();
}


//****************************************
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	allocateMemory();
	createPalette();
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	GLsizei windowX = (glutGet(GLUT_SCREEN_WIDTH)-width)/2;
	GLsizei windowY = (glutGet(GLUT_SCREEN_HEIGHT)-height)/2;
	glutInitWindowPosition(windowX, windowY);
	glutInitWindowSize(width, height);
	windowID = glutCreateWindow("Fractal ZoomeR");

	glShadeModel(GL_SMOOTH); //smooth
	glEnable(GL_DEPTH_TEST); //and deep
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(minX, maxX, minY, maxY, ((GLdouble)-1), (GLdouble)1);

	// set the event handling methods
	glutDisplayFunc(repaint);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyFunction);
	glutMouseFunc(mouse);
	glutIdleFunc(IdleFunction);

	glutMainLoop();


	return 0;
}
