//////////////////////////////////////////////////////////////////////////////
// Example illustrating the use of GCoptimization.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
//  Optimization problem:
//  is a set of sites (pixels) of width 10 and hight 5. Thus number of pixels is 50
//  grid neighborhood: each pixel has its left, right, up, and bottom pixels as neighbors
//  7 labels
//  Data costs: D(pixel,label) = 0 if pixel < 25 and label = 0
//            : D(pixel,label) = 10 if pixel < 25 and label is not  0
//            : D(pixel,label) = 0 if pixel >= 25 and label = 5
//            : D(pixel,label) = 10 if pixel >= 25 and label is not  5
// Smoothness costs: V(p1,p2,l1,l2) = min( (l1-l2)*(l1-l2) , 4 )
// Below in the main program, we illustrate different ways of setting data and smoothness costs
// that our interface allow and solve this optimizaiton problem

// For most of the examples, we use no spatially varying pixel dependent terms.
// For some examples, to demonstrate spatially varying terms we use
// V(p1,p2,l1,l2) = w_{p1,p2}*[min((l1-l2)*(l1-l2),4)], with
// w_{p1,p2} = p1+p2 if |p1-p2| == 1 and w_{p1,p2} = p1*p2 if |p1-p2| is not 1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "GCoptimization.h"

#define MAXWIDTH  640
#define MAXHEIGHT 480


using namespace std;

struct GraphPixel{
    int r, g, b;
    int edge;
};

struct SuperPixel{
    int r, g, b;
    int edgeCount;
    int x, y;
};


int w, h;
GraphPixel g[MAXWIDTH][MAXHEIGHT];

struct ForDataFn{
	int numLab;
	int *data;
};

int max(int x, int y){
    return x > y ? x : y;
}

int max(int x, int y, int z){
    return max(max(x, y), z);
}

int Rgb2Gray(GraphPixel p){
    return (int)(p.r * 0.299 + p.g * 0.587 + p.b * 0.114);
}

int ComputeCost(int i, int j, int l){
    int cost = 0;
    GraphPixel p = g[i][j];
    int gray = Rgb2Gray(p);
    if (l == 0){
        cost = max(gray - 10, 0) + 80;
    } else
    if (l == 1){
        cost = max(210 - gray, 0) + 10;
    } else
    if (l == 2){
        cost = max(max(p.r, p.g, p.b), max(255 - p.r, 255 - p.g, 255 - p.b)) * 2 / 3;
    } else
    if (l == 3){
        cost = p.edge ?  0: 300;
    } else
    {
        cost = 0;
    }
    return cost;
}

bool ReadImage(){
    FILE *fp;

    if ((fp = fopen("C:\\HuaWeiImage\\AlgorithmEvaluationOpenCV\\AlgorithmEvaluationOpenCV\\ImageOutput.txt", "r")) == NULL){
        printf("can't open the file.\n");
        return false;
    }

    fscanf(fp, "%d %d", &h, &w);
    for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++){
        fscanf(fp, "%d %d %d", &g[i][j].r, &g[i][j].g, &g[i][j].b);
    }
    for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++){
        fscanf(fp, "%d", &g[i][j].edge);
    }

    fclose(fp);
    return true;
}


int smoothFn(int p1, int p2, int l1, int l2)
{
	if ( (l1-l2)*(l1-l2) <= 4 ) return((l1-l2)*(l1-l2));
	else return(4);
}

int dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int numLab = myData->numLab;

	return( myData->data[p*numLab+l] );
}


void GraphCut(int width, int height, int num_pixels, int num_labels){
    int *result = new int[num_pixels];

    try{
        GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, num_labels);
        for (int l = 0; l < num_labels; l++)
        for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++){
            gc->setDataCost(i * width + j, l, ComputeCost(i, j, l));
        }
        int varity = 6;
        gc->setSmoothCost(0, 1, 100 / varity);
        gc->setSmoothCost(1, 0, 100 / varity);
        gc->setSmoothCost(0, 2, 100 / varity);
        gc->setSmoothCost(2, 0, 100 / varity);
        gc->setSmoothCost(1, 2, 10 / varity);
        gc->setSmoothCost(2, 1, 10 / varity);

        gc->setSmoothCost(0, 3, 100 / varity);
        gc->setSmoothCost(3, 0, 100 / varity);
        gc->setSmoothCost(1, 3, 100 / varity);
        gc->setSmoothCost(3, 1, 100 / varity);
        gc->setSmoothCost(2, 3, 6 / varity);
        gc->setSmoothCost(3, 2, 6 / varity);


        //gc->setLabelCost(0);
        printf("before: %d\n", gc->compute_energy());
        gc->swap();
        printf("after: %d\n", gc->compute_energy());
        FILE *fout;
        fout = fopen("C:\\HuaWeiImage\\AlgorithmEvaluationOpenCV\\AlgorithmEvaluationOpenCV\\ImageLabel.txt", "w");
 		for ( int  i = 0; i < num_pixels; i++){
            result[i] = gc->whatLabel(i);
            fprintf(fout, "%d\n",result[i]);
		}
		fclose(fout);
   }
    catch(GCException e){
        e.Report();
    }
    delete [] result;
}


////////////////////////////////////////////////////////////////////////////////
// smoothness and data costs are set up one by one, individually
// grid neighborhood structure is assumed
//
void GridGraph_Individually(int width,int height,int num_pixels,int num_labels)
{

	int *result = new int[num_pixels];   // stores result of optimization



	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);

		// first set up data costs individually
		for ( int i = 0; i < num_pixels; i++ )
			for (int l = 0; l < num_labels; l++ )
				if (i < 25 ){
					if(  l == 0 ) gc->setDataCost(i,l,0);
					else gc->setDataCost(i,l,10);
				}
				else {
					if(  l == 5 ) gc->setDataCost(i,l,0);
					else gc->setDataCost(i,l,10);
				}

		// next set up smoothness costs individually
		for ( int l1 = 0; l1 < num_labels; l1++ )
			for (int l2 = 0; l2 < num_labels; l2++ ){
				int cost = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;
				gc->setSmoothCost(l1,l2,cost);
			}

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(-1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++){
            result[i] = gc->whatLabel(i);
//            printf("%d: %d\n", i, result[i]);
		}
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
void GridGraph_DArraySArray(int width,int height,int num_pixels,int num_labels)
{

	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for ( int l1 = 0; l1 < num_labels; l1++ )
		for (int l2 = 0; l2 < num_labels; l2++ )
			smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;


	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);
		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] smooth;
	delete [] data;

}
////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
void GridGraph_DfnSfn(int width,int height,int num_pixels,int num_labels)
{

	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}


	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);

		// set up the needed data to pass to function for the data costs
		ForDataFn toFn;
		toFn.data = data;
		toFn.numLab = num_labels;

		gc->setDataCost(&dataFn,&toFn);

		// smoothness comes from function pointer
		gc->setSmoothCost(&smoothFn);

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] data;

}
////////////////////////////////////////////////////////////////////////////////
// Uses spatially varying smoothness terms. That is
// V(p1,p2,l1,l2) = w_{p1,p2}*[min((l1-l2)*(l1-l2),4)], with
// w_{p1,p2} = p1+p2 if |p1-p2| == 1 and w_{p1,p2} = p1*p2 if |p1-p2| is not 1
void GridGraph_DArraySArraySpatVarying(int width,int height,int num_pixels,int num_labels)
{
	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for ( int l1 = 0; l1 < num_labels; l1++ )
		for (int l2 = 0; l2 < num_labels; l2++ )
			smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;

	// next set up spatially varying arrays V and H

	int *V = new int[num_pixels];
	int *H = new int[num_pixels];


	for ( int i = 0; i < num_pixels; i++ ){
		H[i] = i+(i+1)%3;
		V[i] = i*(i+width)%7;
	}


	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);
		gc->setDataCost(data);
		gc->setSmoothCostVH(smooth,V,H);
		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] smooth;
	delete [] data;


}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood is set up "manually"
//
void GeneralGraph_DArraySArray(int width,int height,int num_pixels,int num_labels)
{

	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for ( int l1 = 0; l1 < num_labels; l1++ )
		for (int l2 = 0; l2 < num_labels; l2++ )
			smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;


	try{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_pixels,num_labels);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);

		// now set up a grid neighborhood system
		// first set up horizontal neighbors
		for (int y = 0; y < height; y++ )
			for (int  x = 1; x < width; x++ )
				gc->setNeighbors(x+y*width,x-1+y*width);

		// next set up vertical neighbors
		for (int y = 1; y < height; y++ )
			for (int  x = 0; x < width; x++ )
				gc->setNeighbors(x+y*width,x+(y-1)*width);

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] smooth;
	delete [] data;

}
////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood is set up "manually". Uses spatially varying terms. Namely
// V(p1,p2,l1,l2) = w_{p1,p2}*[min((l1-l2)*(l1-l2),4)], with
// w_{p1,p2} = p1+p2 if |p1-p2| == 1 and w_{p1,p2} = p1*p2 if |p1-p2| is not 1

void GeneralGraph_DArraySArraySpatVarying(int width,int height,int num_pixels,int num_labels)
{
	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for ( int l1 = 0; l1 < num_labels; l1++ )
		for (int l2 = 0; l2 < num_labels; l2++ )
			smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;


	try{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_pixels,num_labels);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);

		// now set up a grid neighborhood system
		// first set up horizontal neighbors
		for (int y = 0; y < height; y++ )
			for (int  x = 1; x < width; x++ ){
				int p1 = x-1+y*width;
				int p2 =x+y*width;
				gc->setNeighbors(p1,p2,p1+p2);
			}

		// next set up vertical neighbors
		for (int y = 1; y < height; y++ )
			for (int  x = 0; x < width; x++ ){
				int p1 = x+(y-1)*width;
				int p2 =x+y*width;
				gc->setNeighbors(p1,p2,p1*p2);
			}

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] smooth;
	delete [] data;


}
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    if (ReadImage()) printf("Load.\n"); else return 0;
    int width = w;
	int height = h;
	int num_pixels = width*height;
	int num_labels = 4; // 0-空闲区域 1-假面板 2-杂物 3-设备 4-机柜 5-外界

    GraphCut(width, height, num_pixels, num_labels);






	// smoothness and data costs are set up one by one, individually
	//GridGraph_Individually(width,height,num_pixels,num_labels);

	// smoothness and data costs are set up using arrays
	//GridGraph_DArraySArray(width,height,num_pixels,num_labels);

	// smoothness and data costs are set up using functions
	//GridGraph_DfnSfn(width,height,num_pixels,num_labels);

	// smoothness and data costs are set up using arrays.
	// spatially varying terms are present
	//GridGraph_DArraySArraySpatVarying(width,height,num_pixels,num_labels);

	//Will pretend our graph is
	//general, and set up a neighborhood system
	// which actually is a grid
	//GeneralGraph_DArraySArray(width,height,num_pixels,num_labels);

	//Will pretend our graph is general, and set up a neighborhood system
	// which actually is a grid. Also uses spatially varying terms
	//GeneralGraph_DArraySArraySpatVarying(width,height,num_pixels,num_labels);

	printf("\n  Finished %d (%d) clock per sec %d",clock()/CLOCKS_PER_SEC,clock(),CLOCKS_PER_SEC);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////

