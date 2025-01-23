#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>

#define NUM_ITERS 10 // the number of times we want to conduct the experiment
using namespace std;
using namespace std::chrono;

struct image_t* S1_smoothen(struct image_t *input_image)
{
	// TODO
	// remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.
	

	// space allocation for the structure image_t
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	// // memory allocation for image_pixels array in new_image
	// new_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	// //size allocation
	// for(int i = 0; i<input_image->height; i++){
	// 	new_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
	// 	for(int j = 0; j<input_image->width; j++){
	// 		new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
	// 	}
	// }
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

			// for travesing all the elements on y-axis around the target
			for(int dist_y = -1; dist_y <= 1; dist_y++){
				// for traversing all the elements on x-axis around the target
				for(int dist_x = -1; dist_x <= 1; dist_x++){
					// traversal logic
					int y = i + dist_y;
					int x = j + dist_x;
					// checking if the pixel is within the image
					if(x >= 0 && y >= 0 && x < input_image->width && y < input_image->height){
						r_sum += input_image->image_pixels[y][x][0];
						g_sum += input_image->image_pixels[y][x][1];
						b_sum += input_image->image_pixels[y][x][2];
						number_of_pixels++;
					}

				}
			}

			// calculating the final rgb value of the image after smoothening
			new_image->image_pixels[i][j][0] = r_sum/number_of_pixels; // for the red in rgb
			new_image->image_pixels[i][j][1] = g_sum/number_of_pixels; // for the green in rgb
			new_image->image_pixels[i][j][2] = b_sum/number_of_pixels; // for the blue in rgb
			
		}
	}
	return new_image;
}

struct image_t* S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
	// TODO
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	new_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		new_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				// detailed
				new_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k];
			}
		}
	}
	return new_image;
}

struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
	// TODO
	struct image_t* new_image = (struct image_t*)malloc(sizeof(struct image_t));
	new_image->width = input_image->width;
	new_image->height = input_image->height;
	
	new_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		new_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			new_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				new_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] + (5)*details_image->image_pixels[i][j][k];
			}
		}
	}
	return new_image;

}

// free the space
// void free_image(struct image_t * image){
// 	for(int i = 0; i<image->height; i++){
// 		for(int j = 0;j<image->width; j++){
// 			free(image->image_pixels[i][j]); // free the ith-jth pixel
// 		}
// 		free(image->image_pixels[i]); // free the ith row
// 	}
// 	free(image->image_pixels); // free the image_pixels attribute (bcs its an array)
// 	free(image); // free the image structure itself
// }

int main(int argc, char **argv)
{

	// repeating the experiment of time taken by each of the 5 phases
	int number_of_experiments = 5;
	// duration<double> avg_read_time(0), avg_S1_time(0), avg_S2_time(0), avg_S3_time(0), avg_write_time(0);

	struct image_t *input_image = read_ppm_file(argv[1]);


	struct image_t* smoothened_image;
	struct image_t* details_image;
	struct image_t* sharpened_image;
	
	
	auto start_read = steady_clock::now();
	for(int i = 0; i<NUM_ITERS; i++){

		

		// taking the sum of the total S1 phase (smoothening)
		smoothened_image = S1_smoothen(input_image);

		// taking the sum of the total S2 phase (find_details)
		details_image = S2_find_details(input_image, smoothened_image);

		// taking the sum of the total S1 phase (smoothening)
		sharpened_image = S3_sharpen(input_image, details_image);

	}

	auto end_read = steady_clock::now();
	duration<double> total_time(0);
	total_time = end_read - start_read;

	
	write_ppm_file(argv[2], sharpened_image);

	// printing the results of the average time taken in all the 5 phases 
	cout << "Total time: " << total_time.count() << endl;
	
	return 0;
}