#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main(int argc, char* argv[]) {
   FILE *fp;
   fp = fopen(argv[1], "r");
   int threadNum = atoi(argv[2]);
   if (!fp) {
    printf("Failed to open file: %s\n", argv[1]);
    return 1;
   }
   int numOfvar =0;
   double relativeError =0;
   fscanf(fp, "%d", &numOfvar);
   
   fscanf(fp, "%lf", &relativeError);

   double* oldVar=calloc(numOfvar,(sizeof(double)));
   
   for(int i=0;i<numOfvar;i++){
      fscanf(fp, "%lf",&oldVar[i]);
   }
   int** coefficient = malloc(numOfvar * sizeof(int*));
   for(int i=0;i<numOfvar;i++){
      coefficient[i]=malloc(numOfvar*(sizeof(int)));
   }
   int* constant = malloc(numOfvar*(sizeof(int)));
 
   for(int i=0;i<numOfvar;i++){
      for(int j=0;j<=numOfvar;j++){
         if(j==numOfvar){
            fscanf(fp,"%d",&constant[i]);
         }else{
         fscanf(fp,"%d",&coefficient[i][j]);}
      }
   }
   
   fclose(fp);
   // multithreading task
   int indicator =1;
   int iteration =0;
   double stime, etime;
   stime = omp_get_wtime();
   #pragma omp parallel num_threads(threadNum)
{
    int indicator_local = 0;
    double* newVar = calloc(numOfvar, sizeof(double));
    for( int i=0;i<numOfvar;i++){
        newVar[i]=10000;
    }
    double* error = calloc(numOfvar, sizeof(double));
    while(indicator) {
        indicator_local = 0;
        #pragma omp for reduction(|:indicator)
        for (int i = 0; i < numOfvar; i++) {
            double result = 0;
            for (int j = 0; j < numOfvar; j++) {
                if (i != j) {
                    result += coefficient[i][j] * oldVar[j];
                }
            }
            newVar[i] = (constant[i] - result) / coefficient[i][i];
            error[i] = fabs(newVar[i] - oldVar[i])/fabs(newVar[i]);

            if (error[i] > relativeError) {
                indicator_local = 1;
            }
        }

        #pragma omp critical
        {   
            for(int i=0;i<numOfvar;i++){
                if(newVar[i]!=10000){
                oldVar[i]=newVar[i];
                }
            }

            indicator = indicator_local;
            indicator_local = 0;
            iteration++;
        }

        #pragma omp barrier
    }
}
   etime = omp_get_wtime();
   double time =etime-stime;
   free(coefficient);
   free(constant);
   //output

   char filename[50];
   sprintf(filename, "%d.sol", numOfvar);
   FILE *file = fopen(filename, "w");
   for( int i=0;i<numOfvar;i++){
   fprintf(file,"%lf\n",oldVar[i]);
   }
   fclose(file);
   printf("%lf ", time);
   //print iteration on screen
   printf("%d ",iteration);

return 0;
}