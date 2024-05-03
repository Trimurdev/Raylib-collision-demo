/*
 * This file is part of Collision Demo using raylib.
 *
 * You are free to copy, modify, and distribute this file, even for commercial purposes, all without asking permission.
 * For more information, please refer to the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication:
 * https://creativecommons.org/publicdomain/zero/1.0/
 */
#ifndef COLLISION_H
#define COLLISION_H

#include "raylib.h" //Accsess the mesh data
#include "raymath.h"//Vector mathh

#include <stdlib.h>//Memory operations
#include <float.h>//FLT_MAX

/*Warning
I am not very good at c, i made this file as a quick demo, this file
might have some foramting and coding errors, i didn't make this into a source and header file, since 
it is a quick demo, i might do that in the c++ implementation
*/


typedef struct {
    Vector3 *transformedPoints;// Array of points definig the transofrmed collider
    Vector3 *notTransformed;// Array of points defining the collider
    int numPoints;      // Number of points in the array
	Vector3 *normals; //Normal array, helps with detection
	int numNormals; //Number of normals
} Collider;

void GetNormals(Mesh mesh,Vector3 * normals) {
    for (int i = 0; i < mesh.vertexCount; i+=3) {//Loop trough the normals and add them to the array
        normals[i/3] = (Vector3){mesh.normals[i],mesh.normals[i+1],mesh.normals[i+2]};
    }
}

//Fogot to add a check to check if the mesh inst empty :P
void SetupColliderMesh(Collider *c, Mesh mesh) {
    c->numPoints = mesh.vertexCount; //Set the numPoints as num vertices
    c->notTransformed = (Vector3 *)malloc(mesh.vertexCount * sizeof(Vector3));//Init the non transofrmed array
    c->transformedPoints = (Vector3 *)malloc(mesh.vertexCount * sizeof(Vector3));//Init the transofrmed array
    int vertex = 0; //Init vertex counter
    for (int i = 0; i < c->numPoints; i++) {
        c->notTransformed[i] = (Vector3){mesh.vertices[vertex], mesh.vertices[vertex + 1], mesh.vertices[vertex + 2]}; // Create the vertex position using mesh data
        vertex += 3; // Add 3 since raylib uses a float array for vertices instead of Vector3, each float is either x, y or z 
    }
   
    /*The tirangle and normal count are the same, for this case, otherwise when using smoothshading
    i recommend to use sizeof function on the normals array*/
    c->numNormals = mesh.triangleCount; 
    c->normals = (Vector3 *)malloc(mesh.triangleCount * sizeof(Vector3)); //Init normal array
    GetNormals(mesh,*&c->normals);//Set the normal data
}


void GetMinMax(Collider b, Vector3 axis, float *min, float *max) {
    *min = Vector3DotProduct(b.transformedPoints[0], axis); //Initialize min as the first vertex
    *max = *min;  // Initialize max with the same value as min

    for (int i = 1; i < b.numPoints; i++) { //Loop trough the vertices
        //Find the dot product between the vertex and the axis
        float dot = Vector3DotProduct(b.transformedPoints[i], axis);
        //Check if the dot is smaller then min, if it is set the new min
        if (dot < *min) {
            *min = dot;
        }
        //Check if the dot is bigger then max, if it is set the new max
        if (dot > *max) {
            *max = dot;
        }
    }
}

Vector3 GetMiddlePoint(Vector3 *vertices,int numVertices){
	float x=0,y=0,z=0;//Init variables
	for(int i = 0; i < numVertices;i++){//Iterate trought the vertices and get the sum
       //Add all the vertex values togerher
       x+= vertices[i].x; 
       y+= vertices[i].y;
       z+= vertices[i].z;
	}
	return(Vector3){x/numVertices,y/numVertices,z/numVertices}; //Divide the values to get the average(Middle)
} 

bool CheckCollision(Collider a, Collider b, Vector3 *normal) {
    *normal = (Vector3){0, 0, 0}; //Init normal vector
    float depth = FLT_MAX; //Init depth as the max value it can be

    for (int i = 0; i < a.numNormals; i++) {
        float min1, max1, min2, max2;
       GetMinMax(a, a.normals[i], &min2, &max2);
       GetMinMax(b, a.normals[i], &min1, &max1);

        if (max1 < min2 || max2 < min1) {
            return false;  // No collision on this axis
        } else {
            float axisDepth = fminf(max2 - min1, max1 - min2);
            if (axisDepth < depth) {
                depth = axisDepth;
                *normal = a.normals[i];
            }
        }
    }

    for (int i = 0; i < b.numNormals; i++) {
        float min1, max1, min2, max2;
        GetMinMax(a, b.normals[i], &min2, &max2);
        GetMinMax(b, b.normals[i], &min1, &max1);

        if (max1 < min2 || max2 < min1) {
            return false;  // No collision on this axis
        } else {
            float axisDepth = fminf(max2 - min1, max1 - min2);
            if (axisDepth < depth) {
                depth = axisDepth;
                *normal = b.normals[i];
            }
        }
    }

    // Simplify direction calculation
    Vector3 direction = Vector3Subtract(GetMiddlePoint(a.transformedPoints, a.numPoints), GetMiddlePoint(b.transformedPoints,b.numPoints));

    if (Vector3DotProduct(direction, *normal) < 0.0f) {
        *normal = Vector3Negate(*normal);
    }
    *normal = Vector3Scale(*normal, depth);
    return true;
}

//Not optimal, doesn't include rotation and scaling, it's a simple demo
void UpdateCollider(Vector3 parent,Collider *c){
    //Loop trough the points and add them to the 
    for(int i = 0; i < c->numPoints;i++){
        c->transformedPoints[i] = Vector3Add( c->notTransformed[i],parent);
    }
}

void UnloadCollider(Collider *collider){
    free(collider->normals);
    free(collider->transformedPoints);
    free(collider->notTransformed);
}


#endif

