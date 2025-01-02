#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct shape {
    float x[4];
    float y[4];
    int n;
    float area;
    struct shape *next;
} shape;

shape* read(char *filename) {

    FILE *fp = fopen(filename, "r");

    shape *head = NULL;
    shape *tail = NULL;

    char line[256];
    char garbage[100];

    while (fgets(line, sizeof(line), fp)) {

        shape *new_shape = (shape*)malloc(sizeof(shape));

        new_shape->next = NULL;

        sscanf(line, "%s%f%f%f%f", garbage, &new_shape->x[0], &new_shape->x[1], &new_shape->x[2], &new_shape->x[3]);
        
        fgets(line, sizeof(line), fp);
        
        sscanf(line, "%s %f %f %f %f", garbage, &new_shape->y[0], &new_shape->y[1], &new_shape->y[2], &new_shape->y[3]);

        if (new_shape->x[3] == 0 && new_shape->y[3] == 0) {

            new_shape->n = 3;
        }
        
        else {

            new_shape->n = 4;
        }

        if (head == NULL) {

            head = tail = new_shape;
        }
        
        else {

            tail->next = new_shape;
            tail = new_shape;
        }
    }

    fclose(fp);
    return head;
} 

float calculate_triangle_area(float x[], float y[]) {

    return 0.5 * fabs(x[0]*y[1] - x[1]*y[0] + x[1]*y[2] - x[2]*y[1] + x[2]*y[0] - x[0]*y[2]);
}

float calculate_rectangle_area(float x[], float y[]) {

    return fabs((x[1] - x[0]) * (y[2] - y[1]));
}

void calc_areas(shape *head) {

    while (head != NULL) {

        if (head->n == 3) {

            head->area = calculate_triangle_area(head->x, head->y);
        }
        
        else if (head->n == 4) {

            head->area = calculate_rectangle_area(head->x, head->y);
        }

        head = head->next;
    }
}

void print_shapes(shape *head) {

    while (head != NULL) {

        printf("Shape with %d vertices:\n", head->n);

        for (int i = 0; i < head->n; ++i) {

            printf("(%f, %f) ", head->x[i], head->y[i]);
        }

        printf("\nArea: %f\n\n", head->area);
        head = head->next;
    }
}

void free_shapes(shape *head) {

    while (head != NULL) {

        shape *temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    char filename[100];
    shape *shapes_list;

    printf("Please enter a file name: ");
    scanf("%s", filename);

    shapes_list = read(filename);
    calc_areas(shapes_list);
    print_shapes(shapes_list);
    free_shapes(shapes_list);

    return 0;
}


