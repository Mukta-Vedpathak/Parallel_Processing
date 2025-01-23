#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>

using namespace std;
using namespace std::chrono;

void free_image(struct image_t *image) {
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            delete[] image->image_pixels[i][j];
        }
        delete[] image->image_pixels[i];
    }
    delete[] image->image_pixels;
    free(image);
}

struct image_t* S1_smoothen(struct image_t *input_image)
{
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	new_image->image_pixels = new uint8_t**[new_image->height];
	for(int i = 0; i < new_image->height; i++)
	{
		new_image->image_pixels[i] = new uint8_t*[new_image->width];
		for(int j = 0; j < new_image->width; j++)
			new_image->image_pixels[i][j] = new uint8_t[3];
	}
	
	for(int i = 0; i<input_image->height ; i++){
		for(int j = 0; j<input_image->width; j++){

			int number_of_pixels = 0, r_sum = 0, g_sum = 0, b_sum = 0;

			for(int dist_y = -1; dist_y <= 1; dist_y++){
				for(int dist_x = -1; dist_x <= 1; dist_x++){
					int y = i + dist_y;
					int x = j + dist_x;
					if(x >= 0 && y >= 0 && x < input_image->width && y < input_image->height){
						r_sum += input_image->image_pixels[y][x][0];
						g_sum += input_image->image_pixels[y][x][1];
						b_sum += input_image->image_pixels[y][x][2];
						number_of_pixels++;
					}

				}
			}

			new_image->image_pixels[i][j][0] = r_sum/number_of_pixels;
			new_image->image_pixels[i][j][1] = g_sum/number_of_pixels; 
			new_image->image_pixels[i][j][2] = b_sum/number_of_pixels;
			
		}
	}
	return new_image;
}

struct image_t* S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	new_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));

	for(int i = 0; i<input_image->height; i++){
		new_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				new_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k];
			}
		}
	}
	return new_image;
}

struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	new_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	for(int i = 0; i<input_image->height; i++){
		new_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				new_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] + (1/9)*details_image->image_pixels[i][j][k];
			}
		}
	}
	return new_image;

}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
		exit(0);
	}

	int pipefd1[2];
	int pipefd2[2];
	char   buf;
	pid_t  cpid1;
	pid_t  cpid2;
	int n;

	duration<double> total_time(0);
	auto start = steady_clock::now();
	struct image_t *input_image = read_ppm_file(argv[1]);

	if (pipe(pipefd1) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	if (pipe(pipefd2) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}


	cpid1 = fork();
	if (cpid1 == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid1 == 0) {    
		cpid2 = fork();
		if (cpid2 == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		if (cpid2 == 0){
			close(pipefd2[1]);

			struct image_t* details_image = (struct image_t*)malloc(sizeof(struct image_t));
			
			for(int i=0; i<1000; i++){
					//printf("Child2 reading from pipe\n");

					if(read(pipefd2[0], &details_image->width, sizeof(details_image->width))<0){
						perror("fail");
					}
					//printf("Child2 reading image width=%d from pipe\n", details_image->width);

					if(read(pipefd2[0], &details_image->height, sizeof(details_image->height))<0){
						perror("fail");
					}
					//printf("Child2 reading image height=%d from pipe\n", details_image->height);

					details_image->image_pixels = new uint8_t**[details_image->height];
					for(int i=0; i<details_image->height; i++){
						details_image->image_pixels[i]=new uint8_t*[details_image->width];
						for(int j=0; j<details_image->width; j++){
							details_image->image_pixels[i][j]=new uint8_t[3];
						}
					}

					for(int i=0; i<details_image->height; i++){
						for(int j=0; j<details_image->width; j++){
							read(pipefd2[0], details_image->image_pixels[i][j], 3);
						}
					}

					//printf("Child2 reading pixels from pipe\n");
				}

			//printf("Detailed_image:success\n");

			struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
			write_ppm_file(argv[2], sharpened_image);
			//printf("Output Image generated\n");

			free_image(details_image);
			
			close(pipefd2[0]);

			auto end = steady_clock::now();
			total_time += end- start;

			cout<<"Total time: "<<total_time.count()<<endl;
			exit(EXIT_SUCCESS);
		}

		else{

			close(pipefd1[1]);          
			sleep(1);

			struct image_t* smoothened_image = (struct image_t*)malloc(sizeof(struct image_t));
			
			for(int i=0; i<1000; i++){
				//printf("Child1 reading from pipe\n");

				read(pipefd1[0], &smoothened_image->width, sizeof(smoothened_image->width));
				//printf("Child1 reading image width=%d from pipe\n", smoothened_image->width);

				read(pipefd1[0], &smoothened_image->height, sizeof(smoothened_image->height));
				//printf("Child1 reading image height=%d from pipe\n", smoothened_image->height);

				smoothened_image->image_pixels = new uint8_t**[smoothened_image->height];
				for(int i=0; i<smoothened_image->height; i++){
					smoothened_image->image_pixels[i]=new uint8_t*[smoothened_image->width];
					for(int j=0; j<smoothened_image->width; j++){
						smoothened_image->image_pixels[i][j]=new uint8_t[3];
					}
				}

				for(int i=0; i<smoothened_image->height; i++){
					for(int j=0; j<smoothened_image->width; j++){
						read(pipefd1[0], smoothened_image->image_pixels[i][j], 3);
					}
				}

				//printf("Child1 reading pixels from pipe\n");

				close(pipefd2[0]);

				struct image_t *details_image = S2_find_details(input_image, smoothened_image);

				//printf("Child1 writing image width=%d to pipe\n", details_image->width);
				write(pipefd2[1], &details_image->width, sizeof(details_image->width));
				
				//printf("Child1 writing image height=%d to pipe\n", details_image->height);
				write(pipefd2[1], &details_image->height, sizeof(details_image->height));
				
				//printf("Child1 writing pixels to pipe\n");
				for (int i = 0; i < details_image->height; i++) {
					for (int j = 0; j < details_image->width; j++) {
						write(pipefd2[1], details_image->image_pixels[i][j], 3);  
					}
				}
			}

			close(pipefd2[1]);          
			waitpid(cpid2, NULL, 0);               
			
			//printf("Smoothened_image:success\n");

			free_image(smoothened_image);
			close(pipefd1[0]);
		}

	} else {            
		close(pipefd1[0]);	
		close(pipefd2[0]);
		close(pipefd2[1]);
		sleep(2);

		for(int i=0; i<1000; i++){

			struct image_t *smoothened_image = S1_smoothen(input_image);
			
			//printf("Parent writing image width=%d to pipe\n", smoothened_image->width);
			write(pipefd1[1], &smoothened_image->width, sizeof(smoothened_image->width));
			
			//printf("Parent writing image height=%d to pipe\n", smoothened_image->height);
			write(pipefd1[1], &smoothened_image->height, sizeof(smoothened_image->height));
			
			//printf("Parent writing pixels to pipe\n");
			for (int i = 0; i < smoothened_image->height; i++) {
				for (int j = 0; j < smoothened_image->width; j++) {
					write(pipefd1[1], smoothened_image->image_pixels[i][j], 3);  
				}
			}

			free_image(smoothened_image);
		}
		close(pipefd1[1]);          
		waitpid(cpid1, NULL, 0);    

		free_image(input_image);          
	}
}