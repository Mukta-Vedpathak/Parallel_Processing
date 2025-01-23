#include<bits/stdc++.h>
#include <cstdint>
#include <iostream>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include <atomic>
#include <semaphore.h>

#include<unistd.h>
#include "libppm.h"
#include<chrono>


#define NUM_ITERS 1000 // macro for number of times the functions should execute


using namespace std;
using namespace std::chrono;


// checker fucntion that all sys call execute properly
void
assert_ok(long rv, string sys_call){
	if(rv == -1){
		fprintf(stderr, "Failed call: %s\n", sys_call.c_str());
		perror("Error: ");
		exit(1);
	}
}

struct image_t*
S1_smoothen(struct image_t *input_image)
{
	// TODO
	// remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.
	

	// space allocation for the structure image_t
	struct image_t* smoothened_image = (struct image_t*)malloc(sizeof(struct image_t));
	smoothened_image->width = input_image->width;
	smoothened_image->height = input_image->height;
	
	// memory allocation for image_pixels array in smoothened_image
	smoothened_image->image_pixels = new uint8_t**[smoothened_image->height];
	for(int i = 0; i < smoothened_image->height; i++)
	{
		smoothened_image->image_pixels[i] = new uint8_t*[smoothened_image->width];
		for(int j = 0; j < smoothened_image->width; j++)
			smoothened_image->image_pixels[i][j] = new uint8_t[3];
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
			smoothened_image->image_pixels[i][j][0] = r_sum/number_of_pixels; // for the red in rgb
			smoothened_image->image_pixels[i][j][1] = g_sum/number_of_pixels; // for the green in rgb
			smoothened_image->image_pixels[i][j][2] = b_sum/number_of_pixels; // for the blue in rgb
			
		}
	}
    return smoothened_image;
}

struct image_t*
S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
	// TODO
	struct image_t *details_image = (struct image_t*)malloc(sizeof(struct image_t));
	details_image->width = input_image->width;
	details_image->height = input_image->height;
	
	details_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		details_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			details_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				// detailed
				details_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k];
			}
		}
	}
    return details_image;
}

struct image_t*
S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
	// TODO
	struct image_t* sharpen_image = (struct image_t*)malloc(sizeof(struct image_t));
	sharpen_image->width = input_image->width;
	sharpen_image->height = input_image->height;
	
	sharpen_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**));
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		sharpen_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*));
		for(int j = 0; j<input_image->width; j++){
			sharpen_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t));
		}
	}
	
	for(int i = 1; i<input_image->height -1 ; i++){
		for(int j = 1; j<input_image->width-1; j++){
		
			for(int k = 0; k<3; k++){
				sharpen_image->image_pixels[i][j][k] = input_image->image_pixels[i][j][k] + (5)*details_image->image_pixels[i][j][k];
			}
		}
	}
    return sharpen_image;

}


int main(int argc, char **argv)
{
	// initializing the time variables for measuring execution times
	duration<double> total_time(0);
	time_point<steady_clock> sharpen_end;

	// read the input images and initialize the final image pointers
	struct image_t *input_image = read_ppm_file(argv[1]);
	struct image_t* smoothened_image;
	struct image_t* details_image;
	struct image_t* sharpened_image;

	// Creating shared spaces for the images and semaphores
	int fd1,  fd2;
	fd1 = shm_open("/smoothen-output", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd1, "open 1");
	fd2 = shm_open("/details-output", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd2, "open 2");
	int fd_sem1 = shm_open("/sem_1", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd_sem1, "open sem_1");
	int fd_sem2 = shm_open("/sem_2", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd2, "open sem_2");
	int fd_sem3 = shm_open("/sem_3", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd2, "open sem_3");
	int fd_sem4 = shm_open("/sem_4", O_CREAT|O_RDWR|O_TRUNC, 0666);
	assert_ok(fd2, "open sem_4");


	// Allocating space for the shared memory
	int rv_fd1 = ftruncate(fd1, input_image->height * input_image->width * 3 * sizeof(uint8_t));
	int rv_fd2 = ftruncate(fd2, input_image->height * input_image->width * 3 * sizeof(uint8_t));
	int rv_sem1 = ftruncate(fd_sem1, sizeof(sem_t));	
	int rv_sem2 = ftruncate(fd_sem2, sizeof(sem_t));
	int rv_sem3 = ftruncate(fd_sem3, sizeof(sem_t));
	int rv_sem4 = ftruncate(fd_sem4, sizeof(sem_t));

	// checking if the ftruncate call is working properly and memory is allocated
	assert_ok(rv_fd1, "ftruncate");
	assert_ok(rv_fd2, "ftruncate");
	assert_ok(rv_sem1, "ftruncate");
	assert_ok(rv_sem2, "ftruncate");
	assert_ok(rv_sem3, "ftruncate");
	assert_ok(rv_sem4, "ftruncate");

	// Mapping the shared memory for the transfering images
	uint8_t *smoothened_space = (uint8_t *)mmap(NULL, input_image->height * input_image->width * 3 * sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0); // contains the smoothened image
	uint8_t *detailed_space = (uint8_t *)mmap(NULL, input_image->height * input_image->width * 3 * sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0); // contain the detailed image

	// Mapping the semaphores for proper synchronization
	sem_t *smoothened_space_free_to_write = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd_sem1, 0)); // allocating shared memory to the two semaphores
	sem_t *smoothened_space_free_to_read = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd_sem2, 0));
	sem_t *detailed_space_free_to_write = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd_sem3, 0));
	sem_t *detailed_space_free_to_read = static_cast<sem_t *>(mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd_sem4, 0));
	
	// checking if the memory is mapped properly
	if(smoothened_space_free_to_write == MAP_FAILED || smoothened_space_free_to_read == MAP_FAILED || detailed_space_free_to_write == MAP_FAILED || detailed_space_free_to_read == MAP_FAILED){
		perror("MAP_FAILED");
		exit(1);
	}

	// initializing the semaphores
	sem_init(smoothened_space_free_to_write, 1, 1); // semaphore smoothened_space_free_to_write , multiple provcesses = true, initial value of atomic variable = 0
	sem_init(smoothened_space_free_to_read, 1, 0); // semaphore smoothened_space_free_to_read, multiple provcesses = true, initial value of atomic variable = 0 
	sem_init(detailed_space_free_to_write, 1, 1); // semaphore detailed_space_free_to_write, multiple provcesses = true, initial value of atomic variable = 0
	sem_init(detailed_space_free_to_read, 1, 0); // semaphore detailed_space_free_to_read, multiple provcesses = true, initial value of atomic variable = 0


	// Allocating spaces for the images that are to be recieved in the child processes
	struct image_t* recieved_smoothened_image = (struct image_t*)malloc(sizeof(struct image_t)); // allocating memory for the pointer struct image_t
	recieved_smoothened_image->width = input_image->width;
	recieved_smoothened_image->height = input_image->height;
	
	recieved_smoothened_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**)); // allocating space corresponding to 3d array
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		recieved_smoothened_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*)); // allocating space for the array
		for(int j = 0; j<input_image->width; j++){
			recieved_smoothened_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t)); // allocating space for each pixel
		}
	}

	struct image_t* recieved_details_image = (struct image_t*)malloc(sizeof(struct image_t)); // allocating memory for the pointer struct image_t
	recieved_details_image->width = input_image->width;
	recieved_details_image->height = input_image->height;
	
	recieved_details_image->image_pixels = (uint8_t ***)malloc(input_image->height * sizeof(uint8_t**)); // allocating space corresponding to 3d array
	
	//size allocation
	for(int i = 0; i<input_image->height; i++){
		recieved_details_image->image_pixels[i] = (uint8_t **)malloc(input_image->width * sizeof(uint8_t*)); // allocating space for the array
		for(int j = 0; j<input_image->width; j++){
			recieved_details_image->image_pixels[i][j] = (uint8_t *)malloc(3 * sizeof(uint8_t)); // allocating space for each pixel
		}
	}


	auto start_read = steady_clock::now();
	

	if(fork() == 0){
		// first child process : details
		for(int iter = 0; iter < NUM_ITERS; iter++){
			sem_wait(smoothened_space_free_to_read); // wait for parent to signal
			int offset = 0;
			// read the data from the smoothened space

			cout << "reading "<< iter << " from smoothened_space" << endl;
			for (int i = 0; i < input_image->height; i++)
			{
				for (int j = 0; j < input_image->width; j++)
				{
					for (int pixel = 0; pixel < 3; pixel++)
					{
						recieved_smoothened_image->image_pixels[i][j][pixel] = smoothened_space[offset];
						offset++;
					}
					
				}
				
			}
			// signal the parent, now it can write in the smoothened space
			sem_post(smoothened_space_free_to_write);

			cout << "S2_find details ITER = " << iter << endl;
 			// getting the details_image
			details_image = S2_find_details(input_image, recieved_smoothened_image);

			// Writing to the detailed_space shared memory
			sem_wait(detailed_space_free_to_write);

			offset = 0;
			for(int i = 0; i<input_image->height; i++){
				for (int j = 0; j < input_image->width; j++)
				{
					for(int pixel = 0; pixel <3; pixel++){
						detailed_space[offset] = details_image->image_pixels[i][j][pixel];
						offset++;
					}
				}
				
			}

			cout << "written "<< iter << " to detailed_space" << endl;
			// signal the next child, it can start reading from the detailed space
			sem_post(detailed_space_free_to_read); 

		}



		return 0; // terminate the process once task is done
	} 
	else {
		if(fork() == 0){
			
			// Second child: Sharpen
			for (int iter = 0; iter < NUM_ITERS; iter++)
			{
				// wait for the first child to write in the detailed_space
				sem_wait(detailed_space_free_to_read); 
				int offset = 0;
				
				cout << "reading " << iter << " from detailed_space" << endl;
				// read the contents from the detailed_space
				for (int i = 0; i < input_image->height; i++)
				{
					for (int j = 0; j < input_image->width; j++)
					{
						for (int pixel = 0; pixel < 3; pixel++)
						{
							recieved_details_image->image_pixels[i][j][pixel] = detailed_space[offset];
							offset++;
						}
						
					}
					
				}

				// signal the first child, reading is done from the detailed space
				sem_post(detailed_space_free_to_write);
				
				cout << "sharpening image ITER = " << iter << endl;
				// sharpen the details image recieved
				sharpened_image = S3_sharpen(input_image, recieved_details_image);

			}

			sharpen_end = steady_clock::now();

			
		

		} else {
			// Parent : Smoothen
			smoothened_image = S1_smoothen(input_image);
			for(int iter = 0; iter < NUM_ITERS; iter++){

				// wait untill the child has read from the smoothened_space
				sem_wait(smoothened_space_free_to_write); 
				int offset = 0;

				cout << "smoothening image ITER = " <<  iter << endl;

				// write the contents to the shared memory , smoothened_space
				for (int i = 0; i < input_image->height; i++)
				{
					for (int j = 0; j < input_image->width; j++)
					{
						for (int pixel = 0; pixel < 3; pixel++)
						{
							
							smoothened_space[offset] = smoothened_image->image_pixels[i][j][pixel];
							offset++;
						}
						
					}
					
				}
				cout<< "written " << iter << " to smoothened_space" << endl;
				// indicate the details(first child) that i am done!, now it can read from the smoothened_space
				sem_post(smoothened_space_free_to_read); 
			}

			wait(NULL); // wait for 1st child to finish
			wait(NULL); // wait for 2nd child to finish

			return 0;

		}
	}
	
	
	// End timing
	auto end_read = steady_clock::now();

	// Cleanup 	semaphores
	sem_destroy(smoothened_space_free_to_write);
	sem_destroy(smoothened_space_free_to_read);	
	sem_destroy(detailed_space_free_to_write);
	sem_destroy(detailed_space_free_to_read);


	// Cleanup shared memory
	munmap(smoothened_space_free_to_write, sizeof(sem_t));
	munmap(smoothened_space_free_to_read, sizeof(sem_t));
	munmap(detailed_space_free_to_write, sizeof(sem_t));
	munmap(detailed_space_free_to_read, sizeof(sem_t));
	munmap(smoothened_space, input_image->height * input_image->width * 3 * sizeof(uint8_t));
    munmap(detailed_space, input_image->height * input_image->width * 3 * sizeof(uint8_t));
    shm_unlink("/smoothen-output");
    shm_unlink("/details-output");
    shm_unlink("/smoothened_space_free_to_write");
    shm_unlink("/smoothened_space_free_to_read");
    shm_unlink("/detailed_space_free_to_write");
    shm_unlink("/detailed_space_free_to_read");



	total_time = end_read - start_read;



	cout << "Total time: " << total_time.count() << endl;
	
	
	return 0;
}
