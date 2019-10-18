
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//Structure to keep track of indices when sorting
struct distanceAndIndex{
    double distance;
    int index;
};

//Structure that binds the point's coordinates with its lof value(see line 120)
struct point{
    double x, y;
    double lof;
};

//Calculate distance between two points
double calcDistance(double x1, double x2, double y1, double y2){
    double distance;
    distance = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    return distance;
}

//Compare function for qsort based on the distance
int compareDistance(const void *a, const void *b){
    struct distanceAndIndex *a1 = (struct distanceAndIndex *)a; //cast it into distAndIndex type then deref to get the real value of it
    struct distanceAndIndex *a2 = (struct distanceAndIndex *)b;
    if ((*a1).distance> (*a2).distance)
        return 1;
    else if ((*a1).distance < (*a2).distance)
        return -1;
    else
        return 0;
    }

//Compare function for qsort based on the value of lof
int compareLof(const void *a, const void *b){
struct point *a1 = (struct point *)a; //cast it into distAndIndex type then deref to get the real value of it
struct point *a2 = (struct point *)b;
if ((*a1).lof> (*a2).lof)
    return -1;
else if ((*a1).lof < (*a2).lof)
    return 1;
else
    return 0;
}

//Calculate distance between point(x, y) and every other points, then store it in struct distanceAndIndex
//qsort based on the distance to find k-neighbours & k-distance of point(x, y)
void findNearestNeighbours(int k, double x, double y, double data[][2], double* kDistance, int* kNeighboursIndex){
    struct distanceAndIndex distance[200];
    for(int i=0; i<200; i++){
        distance[i].distance = calcDistance(x, data[i][0], y, data[i][1]);
        distance[i].index = i;
    }
    qsort(distance, 200, sizeof(distance[0]), compareDistance);
    
    *kDistance = distance[k].distance;
    //distance[0] refer to the point(x, y)
    for(int j=1; j<=k ; j++){
        kNeighboursIndex[j-1] = distance[j].index;
        
    }
}

//Calculate value of Local Reach Density(lrd) of point given as argument
double calcLrd(int k, double kDistance, int* kNeighboursIndex, int kNeighboursNumber){
    double reachDist, lrd, totalReachDist;
    totalReachDist = kDistance*k;   //if point a is within kNeighbours of point b, the reach-dist(a,b) will be the kDistance of b
    lrd = kNeighboursNumber/totalReachDist;
    return lrd;
}

//Calculate value of Local Outlier Factor(lof) of point given as argument
double calcLof(double ownLrd, int* kNeighboursIndex, double* lrd, int kNeighboursNumber){
    double lof, totalNearestLrd=0.0;
    for(int j=0; j<kNeighboursNumber ;j++){
        totalNearestLrd += lrd[kNeighboursIndex[j]];
    }
    lof = (totalNearestLrd/kNeighboursNumber)/ownLrd;
    return lof;
}


int main(int argc, char *argv[])
{
    FILE *fp_in, *fp_out;
    FILE *gp;
    int input;
    int count= 0;
    double data[200][2];    // the number of data is 200
    int k = atoi(argv[3]);  //the number of clusters given in command line argument
    double kDistance[200];  //array to stores value of k-distance of every point
    int kNeighboursIndex[200][k];   //array to stores k-neighbours of every point   //k is fixed by now but need to improve later
    double lrd[200];        //array of value of lrd of every point
    int kNeighboursNumber[200]; //array
    
    //Input//
    fp_in = fopen(argv[1],"r");
    if(fp_in==NULL){
        printf("fail: cannot open the input-file. Change the name of input-file. \n");
        return -1;
    }
    
    while( (input=fscanf(fp_in, "%lf,%lf", &data[count][0], &data[count][1])) != EOF){
        count++;
    }
    
    //Loop to calculate Lrd for every point
    for(int i=0; i<200; i++){
        findNearestNeighbours(k, data[i][0], data[i][1], data, &kDistance[i], kNeighboursIndex[i]); //k-neighbours & k-distance
        kNeighboursNumber[i] = (sizeof(kNeighboursIndex[i])/sizeof(kNeighboursIndex[i][0])); //the number of k-neighbours
        lrd[i] = calcLrd(k, kDistance[i], kNeighboursIndex[i], kNeighboursNumber[i]);
        
    }
    
    //Binds x y coordinates with its lof value in structure point
    struct point points[200];
    for(int j=0; j<200; j++){
        points[j].x = data[j][0];
        points[j].y = data[j][1];
        points[j].lof = calcLof(lrd[j], kNeighboursIndex[j], lrd, kNeighboursNumber[j]);
    }
    
    //Sort based on lof value to get the top 10
    qsort(points, 200, sizeof(points[0]), compareLof);
    
    //Output//
    fp_out = fopen(argv[2],"w");
    if(fp_out==NULL){
        printf("fail: cannot open the output-file. Change the name of output-file.  \n");
        return -1;
    }
    
    for(int i=0;i<200;i++){
        fprintf(fp_out, "%lf,%lf,%lf\n", points[i].x, points[i].y, points[i].lof);
    }
    
    //Draw scatter plot using gnuplot
    gp = popen("gnuplot -persist","w");
    fprintf(gp, "set autoscale x\n");
    fprintf(gp, "set autoscale y\n");
    fprintf(gp, "array dataX[200]\n");
    fprintf(gp, "array dataY[200]\n");
    fprintf(gp, "array lof[200]\n");
    fprintf(gp, "set linetype 1 linecolor rgb 'royalblue'\n");
    fprintf(gp, "set linetype 2 linecolor rgb 'pink'\n");
    
    //Data input to gnuplot
    for(int i=0; i<200; i++){
        fprintf(gp,"dataX[%d] =%lf\n", i+1, points[i].x);
        fprintf(gp,"dataY[%d] =%lf\n", i+1, points[i].y);
        fprintf(gp,"lof[%d] =%f\n", i+1, points[i].lof);
    }
    
    //Differentiate point color according to the value of lof
    //Top 10 points with highest lof will be in royalblue color
    fprintf(gp, "plot lof using (dataX[$1]):(dataY[$1]):($1 > 10? 2:1) with points pt 7 lc variable notitle\n");
    
    //Exit gnuplot
    pclose(gp);
    
    return 0;
}

