#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Clustering
void clustering(int k, double data[][2], double centroid[][2], int label[200]){
    double distance, shortest_distance;
    
	for(int i=0; i<200; i++){	
		shortest_distance = (data[i][0]-centroid[0][0])*(data[i][0]-centroid[0][0]) + (data[i][1]-centroid[0][1])*(data[i][1]-centroid[0][1]);
        label[i]= 0;
		for(int j=1; j<k;j++){		
			distance = (data[i][0]-centroid[j][0])*(data[i][0]-centroid[j][0]) + (data[i][1]-centroid[j][1])*(data[i][1]-centroid[j][1]);
            if (distance < shortest_distance){
				shortest_distance = distance;
                label[i]=j;
                }
		}
	}
}

//Calculate centroid
void calculate_center(int k, double data[][2], int label[], double centroid[][2]){
	int i;
	int number[k];
    double clusterSumOfX[k][2], clusterSumOfY[k][2];
    
    //Initialization
    for(i=0; i<k; i++){
        centroid[i][0]=0;
        centroid[i][1]=0;
        clusterSumOfX[i][0]=0;
        clusterSumOfX[i][1]=0;
        clusterSumOfY[i][0]=0;
        clusterSumOfY[i][1]=0;
        number[i] =0;
    }
    
	for(i=0; i<200; i++){
			number[label[i]] ++; //the number of data in a cluster
			clusterSumOfX[label[i]][0] += data[i][0]; //sum of x-coordinates of data in a cluster
			clusterSumOfY[label[i]][1] += data[i][1]; //sum of x-coordinates of data in a cluster
	}
    
	for(i=0; i<k;i++){
		centroid[i][0] = clusterSumOfX[i][0] / number[i];
		centroid[i][1] = clusterSumOfY[i][1] / number[i];
	}
    
}

int main(int argc, char *argv[])
{
    FILE *fp_in, *fp_out;
    FILE *gp;
	int input;
	int count= 0;
	double data[200][2];   // the number of data is 200
	int i, j;              //integer for iteration
	int k = atoi(argv[3]); //the number of clusters given in command line argument
	double centroid[k][2]; //array of centroid of every cluster
    int label[200];        //label of cluster which the data belongs to
    int old_label[200];    //array to keep previous label when updating label array
	
	//Input//
	fp_in = fopen(argv[1],"r");
	if(fp_in==NULL){
		printf("fail: cannot open the input-file. Change the name of input-file. \n");
		return -1;
	}
	
	while( (input=fscanf(fp_in, "%lf,%lf", &data[count][0], &data[count][1])) != EOF){
        label[count] = count % k; //distribute data into clusters randomly
		count++;
	}
	
	//K-means
    //Array initialization
    for(i=0; i<k; i++){
        centroid[i][0]= 0;
        centroid[i][1]= 0;
    }
    for(i=0; i<200; i++){
        old_label[i] = 0;
    }
    
    //Loop until label of data doesn't change
    while(memcmp(old_label, label, sizeof(label))!=0){
        calculate_center(k, data, label, centroid); //calculate centroid of every cluster
        memcpy(old_label, label, sizeof(label));    //copy the current label into an array
        clustering(k, data, centroid, label);       //cluster data again and update the current label
    }
    
	//Output//
	fp_out = fopen(argv[2],"w");
	if(fp_out==NULL){
		printf("fail: cannot open the output-file. Change the name of output-file.  \n");
		return -1;
	}

    
	for(i=0;i<200;i++){
		fprintf(fp_out, "%lf,%lf,%d\n", data[i][0], data[i][1], label[i]);
	}
    
    //Draw scatter plot using gnuplot
    gp = popen("gnuplot -persist","w");
    fprintf(gp, "set autoscale x\n");
    fprintf(gp, "set autoscale y\n");
    fprintf(gp, "array dataX[200]\n");
    fprintf(gp, "array dataY[200]\n");
    fprintf(gp, "array label[200]\n");/
    char points_color[6][15] = {"royalblue", "pink", "turquoise", "grey", "aquamarine", "khaki"};
    
    //Set different color for every cluster
    for(i=0; i<=k; i++){
        fprintf(gp, "set linetype %d linecolor rgb '%s'\n", i+1, points_color[i]);
    }
    
    //Data input to gnuplot
    for(i=0; i<200; i++){
        fprintf(gp,"dataX[%d] =%lf\n", i+1, data[i][0]);
        fprintf(gp,"dataY[%d] =%lf\n", i+1, data[i][1]);
        fprintf(gp,"label[%d] =%d\n", i+1, label[i]+1);
    }
    
    //Plot the data with different color for every cluster
    fprintf(gp, "plot label using (dataX[$1]):(dataY[$1]):2 with points pt 7 lc variable notitle\n");
    
    //exit gnuplot
    pclose(gp);
    
	return 0;
}
