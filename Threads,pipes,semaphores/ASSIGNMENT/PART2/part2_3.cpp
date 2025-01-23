#include <iostream>       
#include <atomic>        
#include <thread>         
#include <vector>         
#include <sstream>        
#include "libppm.h"
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <cstdint>

using namespace std;
using namespace std::chrono;

atomic_flag lock1 = ATOMIC_FLAG_INIT;
atomic_flag lock2 = ATOMIC_FLAG_INIT;
atomic_flag lock3 = ATOMIC_FLAG_INIT;
atomic_flag lock4 = ATOMIC_FLAG_INIT;

struct image_t* smoothened_image = nullptr;
struct image_t* details_image = nullptr;
struct image_t* sharpened_image = nullptr;
struct image_t* read_smoothened_image = nullptr;

int ITER=1000;
int ITER_SMOOTH = 0;
int ITER_DETAIL = 0;
int ITER_SHARP = 0;

void free_image(struct image_t* image) {
    if (image == nullptr) return;

    for (int i = 0; i < image->height; ++i) {
        for (int j = 0; j < image->width; ++j) {
            free(image->image_pixels[i][j]);
        }
        free(image->image_pixels[i]); 
    }
    free(image->image_pixels); 
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

struct image_t* S2_read_smoothened(struct image_t* input_image, struct image_t *smoothened_image )
{
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = smoothened_image->width;
	new_image->height = smoothened_image->height;

	new_image->image_pixels = (uint8_t ***)malloc(smoothened_image->height * sizeof(uint8_t**));

	for(int i = 0; i<smoothened_image->height; i++){
		new_image->image_pixels[i] = (uint8_t **)malloc(smoothened_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
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

void* process_S1(void* input) {
	image_t* input_image = (image_t*)input;
	while(ITER_SMOOTH<ITER){
		while (lock1.test_and_set()){}
			//cout << "P1 started.\n";
			if (smoothened_image != nullptr) free_image(smoothened_image);
			smoothened_image = S1_smoothen(input_image);
			//cout << "P1 done.\n";
			lock2.clear();
		ITER_SMOOTH+=1;
	}
	return NULL;
}

void* process_S2(void* input) {
	image_t* input_image = (image_t*)input;
	while(ITER_DETAIL<ITER){
		while(lock2.test_and_set()){}
			//cout << "P2 started.\n";
			if (read_smoothened_image != nullptr) free_image(read_smoothened_image);
			read_smoothened_image= S2_read_smoothened(input_image, smoothened_image);
			lock1.clear();
		while(lock3.test_and_set()){}
			if (details_image != nullptr) free_image(details_image); 
    		details_image = S2_find_details(input_image, read_smoothened_image);
			//cout << "P2 done.\n";
			lock4.clear();
		ITER_DETAIL+=1;
	}
	return NULL;
}

void* process_S3(void* input) {
	image_t* input_image = (image_t*)input;
	while(ITER_SHARP<ITER){
		while(lock4.test_and_set()){}
			//cout << "P3 started.\n";
			if (sharpened_image != nullptr) free_image(sharpened_image);
    		sharpened_image = S3_sharpen(input_image, details_image);
			//cout << "P3 done.\n";
			lock3.clear();
		ITER_SHARP+=1;
	}
	return NULL;
}


int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
		exit(0);
	}
	 
	duration<double> total_time(0);
	auto start = steady_clock::now();
	struct image_t *input_image = read_ppm_file(argv[1]);
	//cout<<"Image read"<<endl;

	pthread_t thread1, thread2, thread3;
 	int iret1, iret2, iret3;

	lock1.clear();
	lock2.test_and_set();
	lock3.clear();
	lock4.test_and_set();

	iret1 = pthread_create(&thread1, NULL, process_S1,input_image);
 	iret2 = pthread_create(&thread2, NULL, process_S2,input_image);
  	iret3 = pthread_create(&thread3, NULL, process_S3,input_image);

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	pthread_join(thread3,NULL);

	write_ppm_file(argv[2], sharpened_image);
	//cout<<"Image written."<<endl;

	auto end = steady_clock::now();
	total_time += end- start;

	cout<<"Total time: "<<total_time.count()<<endl;

	return 0;
}