#ifndef TGA_READER_H
#define TGA_READER_H

#include "common.h"
#include "scene.h"
#include "geometry.h"
#include "texture.h"

// adapted from http://www.paulbourke.net/dataformats/tga/tgatest.c
namespace tga_reader {

    typedef struct {
       char  idlength;
       char  colourmaptype;
       char  datatypecode;
       short int colourmaporigin;
       short int colourmaplength;
       char  colourmapdepth;
       short int x_origin;
       short int y_origin;
       short width;
       short height;
       char  bitsperpixel;
       char  imagedescriptor;
    } HEADER;

    typedef struct {
       unsigned char r,g,b,a;
    } PIXEL;

    void getHeader(FILE* fptr, HEADER& h);
    void printHeader(HEADER& h);
    void MergeBytes(PIXEL *pixel, unsigned char *p, int bytes);

    Texture readTexture(std::string tgaFileName) {

        FILE *fptr;
        if ((fptr = fopen(tgaFileName.c_str(),"r")) == NULL) {
          printf("Failed to open texture file\n");
          exit(-1);
        }
        HEADER header;
        PIXEL *pixels;
        int n = 0, i, j;
        int bytes2read, skipover = 0;
        unsigned char p[5];

        getHeader(fptr, header);
        //printHeader(header);

        if ((pixels = (PIXEL*)malloc(header.width * header.height * sizeof(PIXEL))) == NULL) {
            printf("malloc of image failed\n");
            exit(-1);
        }
        for (i = 0; i < header.width * header.height; i++) {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
            pixels[i].a = 0;
        }

        /* What can we handle */
        if (header.datatypecode != 2 && header.datatypecode != 10) {
            printf("Can only handle image type 2 and 10\n");
            exit(-1);
        }
        /*if (header.bitsperpixel != 16 && 
            header.bitsperpixel != 24 && header.bitsperpixel != 32) {
            printf("Can only handle pixel depths of 16, 24, and 32\n");
            exit(-1);
        }*/
        if (header.bitsperpixel != 24) {
            printf("Can only handle pixel depths of 24\n");
            exit(-1);
        }
        if (header.colourmaptype != 0 && header.colourmaptype != 1) {
            printf("Can only handle colour map types of 0 and 1\n");
            exit(-1);
        }

        /* Skip over unnecessary stuff */
        skipover += header.idlength;
        skipover += header.colourmaptype * header.colourmaplength;
        fseek(fptr, skipover, SEEK_CUR);

        /* Read the image */
        bytes2read = header.bitsperpixel / 8;
        while (n < header.width * header.height) {
            if (header.datatypecode == 2) {                     /* Uncompressed */
                if (fread(p, 1, bytes2read, fptr) != bytes2read) {
                    printf("Unexpected end of file at pixel %d\n", i);
                    exit(-1);
                }
                MergeBytes(&(pixels[n]), p, bytes2read);
                n++;
            } else if (header.datatypecode == 10) {             /* Compressed */
                if (fread(p, 1, bytes2read + 1, fptr) != bytes2read + 1) {
                    printf("Unexpected end of file at pixel %d\n", i);
                    exit(-1);
                }
                j = p[0] & 0x7f;
                MergeBytes(&(pixels[n]), &(p[1]), bytes2read);
                n++;
                if (p[0] & 0x80) {         /* RLE chunk */
                    for (i = 0; i < j; i++) {
                        MergeBytes(&(pixels[n]), &(p[1]), bytes2read);
                        n++;
                    }
                } else {                   /* Normal chunk */
                    for (i = 0; i < j; i++) {
                        if (fread(p, 1, bytes2read, fptr) != bytes2read) {
                            printf("Unexpected end of file at pixel %d\n", i);
                            exit(-1);
                        }
                        MergeBytes(&(pixels[n]), p, bytes2read);
                        n++;
                    }
                }
            }
        }

        int px_idx;
        PIXEL current_pixel;
        std::vector<std::vector<Colour>> texels;
        floating r, g, b;
        
        for (i = header.height - 1; i >= 0; --i) {
            std::vector<Colour> row;

            for (j = 0; j < header.width; ++j) {
                // pixels array is ordered from bottom to top, but let's read pixels from top to bottom
                px_idx = i * header.width + j;
                current_pixel = pixels[px_idx];

                r = clamp((int)current_pixel.r / 255.0);
                g = clamp((int)current_pixel.g / 255.0);
                b = clamp((int)current_pixel.b / 255.0);

                row.push_back(Colour(r, g, b));

            }
            texels.push_back(row);
        }

        free(pixels);
        fclose(fptr);

        return Texture(texels);
    }

    void getHeader(FILE* fptr, HEADER& h) {
        h.idlength = fgetc(fptr);
        h.colourmaptype = fgetc(fptr);
        h.datatypecode = fgetc(fptr);
        fread(&h.colourmaporigin,2,1,fptr);
        fread(&h.colourmaplength,2,1,fptr);
        h.colourmapdepth = fgetc(fptr);
        fread(&h.x_origin,2,1,fptr);
        fread(&h.y_origin,2,1,fptr);
        fread(&h.width,2,1,fptr);
        fread(&h.height,2,1,fptr);
        h.bitsperpixel = fgetc(fptr);
        h.imagedescriptor = fgetc(fptr);
    }


    void printHeader(HEADER& h) {
        printf("ID length:         %d\n",h.idlength);
        printf("Colourmap type:    %d\n",h.colourmaptype);
        printf("Image type:        %d\n",h.datatypecode);
        printf("Colour map offset: %d\n",h.colourmaporigin);
        printf("Colour map length: %d\n",h.colourmaplength); 
        printf("Colour map depth:  %d\n",h.colourmapdepth);
        printf("X origin:          %d\n",h.x_origin);
        printf("Y origin:          %d\n",h.y_origin);
        printf("Width:             %d\n",h.width);
        printf("Height:            %d\n",h.height);
        printf("Bits per pixel:    %d\n",h.bitsperpixel);
        printf("Descriptor:        %d\n",h.imagedescriptor);
    }

    void MergeBytes(PIXEL *pixel, unsigned char *p, int bytes) {
        if (bytes == 4) {
            pixel->r = p[2];
            pixel->g = p[1];
            pixel->b = p[0];
            pixel->a = p[3];
        } else if (bytes == 3) {
            pixel->r = p[2];
            pixel->g = p[1];
            pixel->b = p[0];
            pixel->a = 0;
        } else if (bytes == 2) {
            pixel->r = (p[1] & 0x7c) << 1;
            pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
            pixel->b = (p[0] & 0x1f) << 3;
            pixel->a = (p[1] & 0x80);
        }
    }

}

#endif