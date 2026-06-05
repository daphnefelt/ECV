#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

// #define IMG_HEIGHT (372)
// #define IMG_WIDTH (580)
int width, height;

typedef double FLOAT;
typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

// Added this func to help get width and height from the ppm instead of hardcoding them
// Writes the vals into the width and height pointers, plus bc we are reading the header here we might as well save it and track the size
bool readPPMHeader(const char* filename, int *width, int *height, UINT8 *header, int *headerSize) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;

    char line[256];
    long pos;

    // Skip magic number (P6) and comment lines (added by GIMP)
    do {
        pos = ftell(f);
        fgets(line, sizeof(line), f);
    } while (line[0] == '#' || line[0] == 'P');
    fseek(f, pos, SEEK_SET); // go back to start of line with dimensions

    int maxVal; // dont actually need this value, but I need to get past it in the header to get to pixel data
    if (fscanf(f, "%d %d %d\n", width, height, &maxVal) != 3) { // 3 return would mean 3 sucesses
        fclose(f);
        return false;
    }

    *headerSize = ftell(f); // get the size of the header using current loc in file
    sprintf((char*)header, "P6 %d %d %d\n", *width, *height, maxVal); // build header w/o comments
    fclose(f);
    return true;
}

// PPM Edge Enhancement Code

#define K 4.0

FLOAT PSF[9] = {-K/8.0, -K/8.0, -K/8.0, -K/8.0, K+1.0, -K/8.0, -K/8.0, -K/8.0, -K/8.0};

int main(int argc, char *argv[])
{
    int fdin, fdout, bytesRead=0, bytesLeft, i, j;
    UINT64 microsecs=0, millisecs=0;
    FLOAT temp;
    
    if(argc < 3)
    {
       printf("Usage: ./sharpen input_file.ppm output_file.ppm\n");
       exit(-1);
    }
    else
    {
        if((fdin = open(argv[1], O_RDONLY, 0644)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("File opened successfully\n");

        if((fdout = open(argv[2], (O_RDWR | O_CREAT), 0666)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("Output file=%s opened successfully\n", "sharpen.ppm");
    }

    //printf("Reading header\n");
    UINT8 header[64];
    int headerSize;

    if (readPPMHeader(argv[1], &width, &height, header, &headerSize)) {
        printf("Width: %d, Height: %d\n", width, height);
    } else {
        printf("Failed to read header\n");
    }
    lseek(fdin, headerSize, SEEK_SET); // skip past header for pixel data

    // Now with dim info, set up arrays for RGB data and convolved RGB data
    int size = height * width;
    UINT8 R[size];
    UINT8 G[size];
    UINT8 B[size];
    UINT8 convR[size];
    UINT8 convG[size];
    UINT8 convB[size];

    // Read RGB data
    for(i=0; i<size; i++)
    {
        read(fdin, (void *)&R[i], 1); convR[i]=R[i];
        read(fdin, (void *)&G[i], 1); convG[i]=G[i];
        read(fdin, (void *)&B[i], 1); convB[i]=B[i];
    }

    // Skip first and last row, no neighbors to convolve with
    for(i=1; i<((height)-1); i++)
    {

        // Skip first and last column, no neighbors to convolve with
        for(j=1; j<((width)-1); j++)
        {
            temp=0;
            temp += (PSF[0] * (FLOAT)R[((i-1)*width)+j-1]);
            temp += (PSF[1] * (FLOAT)R[((i-1)*width)+j]);
            temp += (PSF[2] * (FLOAT)R[((i-1)*width)+j+1]);
            temp += (PSF[3] * (FLOAT)R[((i)*width)+j-1]);
            temp += (PSF[4] * (FLOAT)R[((i)*width)+j]);
            temp += (PSF[5] * (FLOAT)R[((i)*width)+j+1]);
            temp += (PSF[6] * (FLOAT)R[((i+1)*width)+j-1]);
            temp += (PSF[7] * (FLOAT)R[((i+1)*width)+j]);
            temp += (PSF[8] * (FLOAT)R[((i+1)*width)+j+1]);
	    if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convR[(i*width)+j]=(UINT8)temp;

            temp=0;
            temp += (PSF[0] * (FLOAT)G[((i-1)*width)+j-1]);
            temp += (PSF[1] * (FLOAT)G[((i-1)*width)+j]);
            temp += (PSF[2] * (FLOAT)G[((i-1)*width)+j+1]);
            temp += (PSF[3] * (FLOAT)G[((i)*width)+j-1]);
            temp += (PSF[4] * (FLOAT)G[((i)*width)+j]);
            temp += (PSF[5] * (FLOAT)G[((i)*width)+j+1]);
            temp += (PSF[6] * (FLOAT)G[((i+1)*width)+j-1]);
            temp += (PSF[7] * (FLOAT)G[((i+1)*width)+j]);
            temp += (PSF[8] * (FLOAT)G[((i+1)*width)+j+1]);
	    if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convG[(i*width)+j]=(UINT8)temp;

            temp=0;
            temp += (PSF[0] * (FLOAT)B[((i-1)*width)+j-1]);
            temp += (PSF[1] * (FLOAT)B[((i-1)*width)+j]);
            temp += (PSF[2] * (FLOAT)B[((i-1)*width)+j+1]);
            temp += (PSF[3] * (FLOAT)B[((i)*width)+j-1]);
            temp += (PSF[4] * (FLOAT)B[((i)*width)+j]);
            temp += (PSF[5] * (FLOAT)B[((i)*width)+j+1]);
            temp += (PSF[6] * (FLOAT)B[((i+1)*width)+j-1]);
            temp += (PSF[7] * (FLOAT)B[((i+1)*width)+j]);
            temp += (PSF[8] * (FLOAT)B[((i+1)*width)+j+1]);
	    if(temp<0.0) temp=0.0;
	    if(temp>255.0) temp=255.0;
	    convB[(i*width)+j]=(UINT8)temp;
        }
    }

    write(fdout, (void *)header, headerSize);

    // Write RGB data
    for(i=0; i<height*width; i++)
    {
        write(fdout, (void *)&convR[i], 1);
        write(fdout, (void *)&convG[i], 1);
        write(fdout, (void *)&convB[i], 1);
    }


    close(fdin);
    close(fdout);
 
}
