/*
    James William Fletcher (github.com/mrbid)
        October 2022 - December 2023

    Converts ASCII PLY (.ply) file to C OpenGL buffers.
    
    - Auto detects if index buffer is GLubyte, GLushort or GLuint.
    - This is not particularly fast. Not intended for large files.
    - This version only exports PLY files with: index, vertex, color & normal
    - This also works with the latest Blender (4.x) PLY exporter, which no longer
      start the index array at 0,1,2.

    - Notes for speed improvement:
    1. Load PLY Binary not ASCII

    Compile: cc ptf.c -lm -Ofast -o ptf
    
    Usage: ./ptf filename_noextension
        (loads filenames from the 'ply/' directory)
    
    Example: ./ptf porygon
        (loads 'ply/porygon.ply' and outputs 'porygon.h' into the cwd)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUFF 1161326592 //387108864 //24194304 //6048576 //1048576

int main(int argc, char** argv)
{
    // start time
    time_t st = time(0);

    // ensure an input file is specified
    if(argc < 2)
    {
        printf("Please specify an input file.\n");
        return 0;
    }

    // take the input file name and strip any supplied extension
    char name[32] = {0};
    strcat(name, argv[1]);
    char* p = strstr(name, ".");
    if(p != NULL){*p = 0x00;}

    // generate the read file path (reads .ply files from a local `ply/` directory)
    char readfile[32] = {0};
    strcat(readfile, "../ply/");
    strcat(readfile, name);
    strcat(readfile, ".ply");

    // pre-init our buffers
    char* vertex_array = calloc(1, MAX_BUFF);
    char* index_array = calloc(1, MAX_BUFF);
    char* normal_array = calloc(1, MAX_BUFF);
    char* color_array = calloc(1, MAX_BUFF);
    if(vertex_array == NULL || index_array == NULL || normal_array == NULL || color_array == NULL)
    {
        printf("Failed to allocate memory.\n");
        return 0;
    } 

    // open our ASCII PLY file for reading
    unsigned int numvert=0, numind=0, maxind=0;
    unsigned int mode = 0;
    printf("Open: %s\n", readfile);
    FILE* f = fopen(readfile, "r");
    while(f == NULL)
    {
        f = fopen(readfile, "r");
        sleep(1);
    }

    // do the conversion
    char line[256];
    char add[256];
    while(fgets(line, 256, f) != NULL)
    {
        //printf("%s\n",line);
        if(strcmp(line, "end_header\n") == 0)
        {
            mode = 1;
            continue;
        }

        // load vertex, normal, color
        if(mode == 1)
        {
            float vx,vy,vz,nx,ny,nz,r,g,b;
            if(sscanf(line, "%f %f %f %f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz, &r, &g, &b) == 9)
            {
                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                sprintf(add, "%g,%g,%g,", nx, ny, nz);
                strcat(normal_array, add);

                sprintf(add, "%.3g,%.3g,%.3g,", 0.003921568859f*r, 0.003921568859f*g, 0.003921568859f*b);
                strcat(color_array, add);
            }
            else
            {
                mode = 2;
            }
        }

        // load index
        if(mode == 2)
        {
            unsigned int n,x,y,z;
            if(sscanf(line, "%u %u %u %u", &n, &x, &y, &z) == 4)
            {
                char add[256];
                sprintf(add, "%u,%u,%u,", x, y, z);
                strcat(index_array, add);
                if(x > maxind){maxind = x;}
                if(y > maxind){maxind = y;}
                if(z > maxind){maxind = z;}
                numind += 3;
            }
        }
    }
    
    // close PLY file
    fclose(f);
    
    // remove trailing comma's
    vertex_array[strlen(vertex_array)-1] = 0x00;
    normal_array[strlen(normal_array)-1] = 0x00;
    index_array[strlen(index_array)-1] = 0x00;
    color_array[strlen(color_array)-1] = 0x00;

    // output the resultant file
    char outfile[256];
    sprintf(outfile, "../../high/%s.h", name);
    f = fopen(outfile, "w");
    while(f == NULL)
    {
        f = fopen(outfile, "w");
        sleep(1);
    }

    fprintf(f, "\n#ifndef %s_H\n#define %s_H\n\nconst GLfloat %s_vertices[] = {%s};\n", name, name, name, vertex_array);
    fprintf(f, "const GLfloat %s_normals[] = {%s};\n", name, normal_array);
    fprintf(f, "const GLfloat %s_colors[] = {%s};\n", name, color_array);
    
    if(maxind <= 255)
    {
        fprintf(f, "const GLubyte %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (UBYTE)\n", name);
    }
    else if(maxind <= 65535)
    {
        fprintf(f, "const GLushort %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (USHORT)\n", name);
    }
    else
    {
        fprintf(f, "const GLuint %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (UINT)\n", name);
    }

    fprintf(f, "void register_%s()\n", name);
    fprintf(f, "{\n");
    fprintf(f, "    esBind(GL_ARRAY_BUFFER, &esModelArray[esModelArray_index].vid, %s_vertices, sizeof(%s_vertices), GL_STATIC_DRAW);\n", name, name);
    fprintf(f, "    esBind(GL_ARRAY_BUFFER, &esModelArray[esModelArray_index].nid, %s_normals, sizeof(%s_normals), GL_STATIC_DRAW);\n", name, name);
    fprintf(f, "    esBind(GL_ARRAY_BUFFER, &esModelArray[esModelArray_index].cid, %s_colors, sizeof(%s_colors), GL_STATIC_DRAW);\n", name, name);
    fprintf(f, "    esBind(GL_ELEMENT_ARRAY_BUFFER, &esModelArray[esModelArray_index].iid, %s_indices, sizeof(%s_indices), GL_STATIC_DRAW);\n", name, name);
    
    if(maxind <= 255)       {fprintf(f, "    esModelArray[esModelArray_index].itp = GL_UNSIGNED_BYTE;\n");}
    else if(maxind <= 65535){fprintf(f, "    esModelArray[esModelArray_index].itp = GL_UNSIGNED_SHORT;\n");}
    else                    {fprintf(f, "    esModelArray[esModelArray_index].itp = GL_UNSIGNED_INT;\n");}
    
    fprintf(f, "    esModelArray[esModelArray_index].ni = %s_numind;\n", name);
    fprintf(f, "    esModelArray_index++;\n");
    fprintf(f, "}\n\n");

    fprintf(f, "#endif\n");

    fclose(f);
    const float mins = ((float)(time(0)-st))/60.f;
    if(mins > 0.001f){printf("Time Taken: %.2f mins\n", mins);}
    return 0;
}
