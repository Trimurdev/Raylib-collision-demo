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
#include <memory.h>
#include <float.h>//FLT_MAX

/*Warning
I am not very good at c, i made this file as a quick demo, this file
might have some foramting and coding errors, i didn't make this into a source and header file, since 
it is a quick demo, i might do that in the c++ implementation
*/

typedef struct {
    Vector3 *vertices;// Array of vertices definig the base collider
    Vector3 *t_vertices;// Array of vertices defining the transformed collider
    int *indices; 

    int numVertices;  // Number of vertices in the array
    int numIndices; // Number of indices

    BoundingBox aabb; //Axis aligned bounding box for broad phase
} Collider;


typedef struct{
    Vector3 normal;
    float depth;
    bool collided;
}BasicCollisionInfo;

//Sets up the collider
void SetupColliderMesh(Collider* c, Mesh mesh);

//Gets the min and max of the axis
void GetMinMax(Collider* b, Vector3 axis, float* min, float* max);

//Gets the middle point of the collider
Vector3 GetMiddlePoint(Vector3* vertices,int numVertices);

//Helper function to test all the axies and detect any overlap
bool TestFaceAxes(Collider* a, Collider* b, float* depth, Vector3* bestAxis);

//Test collision
BasicCollisionInfo CheckCollision(Collider* a, Collider* b);

//Not optimal, doesn't include rotation and scaling
void UpdateCollider(Vector3 parent,Collider *c);

//Applys the transform matrix to the points and the normals, this includes rotation and scaling
void UpdateCollider(Collider* c,Matrix transform);

//Unloads the collider
void UnloadCollider(Collider *c);


void SetupColliderMesh(Collider* c, Mesh mesh) {
    c->numVertices = mesh.vertexCount; //Set the numVertices as num vertices
    
    if(mesh.indices){
        c->numIndices = mesh.triangleCount * 3;
        c->indices = (int* )malloc(mesh.triangleCount * 3 * sizeof(int));
        memcpy(c->indices, mesh.indices, mesh.triangleCount * 3 * sizeof(int));
    }else{ // Fill the indices by the vertices since no index data is available
        c->indices = (int* )malloc(mesh.vertexCount * sizeof(int));
        for (int i = 0; i < mesh.vertexCount; i++)
        {
            c->indices[i] = i;
        }
        c->numIndices = mesh.vertexCount ;
    }
   
    
    c->vertices = (Vector3 *)malloc(mesh.vertexCount * sizeof(Vector3));//Init the transformed array
    c->t_vertices = (Vector3 *)malloc(mesh.vertexCount * sizeof(Vector3));//Init the transformed array
    c->aabb.min = (Vector3){FLT_MAX,FLT_MAX,FLT_MAX};
    c->aabb.max = (Vector3){-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for (int i = 0; i < c->numVertices; i++) {
        c->vertices[i] = (Vector3){mesh.vertices[i * 3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2]}; // Create the vertex position using mesh data
        c->t_vertices[i] = (Vector3){mesh.vertices[i * 3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2]}; // Create the vertex position using mesh data
    
        c->aabb.min = Vector3Min(c->aabb.min,c->vertices[i]);
        c->aabb.max = Vector3Max(c->aabb.max,c->vertices[i]);
    }

}

void GetMinMax(Collider* b, Vector3 axis, float* min, float* max) {
    *min = Vector3DotProduct(b->t_vertices[0], axis); //Initialize min as the first vertex
    *max = *min;  // Initialize max with the same value as min

    for (int i = 1; i < b->numVertices; i++) { //Loop trough the vertices
        //Find the dot product between the vertex and the axis
        float dot = Vector3DotProduct(b->t_vertices[i], axis);
        //Check if the dot product is smaller then min, if it is set the new min
        if (dot < *min) {
            *min = dot;
        }
        //Check if the dot product is bigger then max, if it is set the new max
        if (dot > *max) {
            *max = dot;
        }
    }
}

Vector3 GetMiddlePoint(Vector3* vertices,int numVertices){
	float x=0,y=0,z=0;//Initialize variables
	for(int i = 0; i < numVertices;i++){//Iterate trought the vertices and get the sum
       //Add all the vertex values togerher
       x+= vertices[i].x; 
       y+= vertices[i].y;
       z+= vertices[i].z;
	}
	return(Vector3){x/numVertices,y/numVertices,z/numVertices}; //Divide the values to get the average(Middle)
} 

bool TestFaceAxes(Collider* a, Collider* b, float* depth, Vector3* bestAxis)
{
    //Loop trough all the vertices and calculate the normals
    for (int i = 0; i < a->numIndices; i += 3)
    {
        //Grab the right indexes
        int i0 = a->indices[i];
        int i1 = a->indices[i + 1];
        int i2 = a->indices[i + 2];

        //Get the normal vectors
        Vector3 v0 = a->t_vertices[i0];
        Vector3 v1 = a->t_vertices[i1];
        Vector3 v2 = a->t_vertices[i2];


        //Calculate the normal/axis
        Vector3 edge1 = Vector3Subtract(v1, v0);
        Vector3 edge2 = Vector3Subtract(v2, v0);

        Vector3 axis = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

        //Skip if invalid axis
        if (Vector3LengthSqr(axis) < 1e-6f) continue;

        //Test overlap
        float minA, maxA, minB, maxB;
        GetMinMax(a, axis, &minA, &maxA);
        GetMinMax(b, axis, &minB, &maxB);


        if (maxA < minB || maxB < minA)
            return false;

        //Calculate and return the depth
        float d = fminf(maxA - minB, maxB - minA);
        if (d < *depth)
        {
            *depth = d;
            *bestAxis = Vector3Normalize(axis);
        }
    }
    return true;
}

BasicCollisionInfo CheckCollision(Collider* a, Collider* b) {
    BasicCollisionInfo result = {0};
    result.depth = FLT_MAX;
    result.normal  = {0};

    // Face normals
    if (!TestFaceAxes(a, b, &result.depth, &result.normal)) return result;
    if (!TestFaceAxes(b, a, &result.depth, &result.normal)) return result;

    // Fix direction
    Vector3 centerA = GetMiddlePoint(a->t_vertices, a->numVertices);
    Vector3 centerB = GetMiddlePoint(b->t_vertices, b->numVertices);

    Vector3 dir = Vector3Subtract(centerA, centerB);

    if (Vector3DotProduct(dir, result.normal) < 0)
        result.normal = Vector3Negate(result.normal);

    result.collided = true;

    return result;
}

void UpdateCollider(Vector3 parent,Collider *c){
    //Loop trough the points and add them to the 
    c->aabb.min = (Vector3){FLT_MAX,FLT_MAX,FLT_MAX};
    c->aabb.max = (Vector3){-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for(int i = 0; i < c->numVertices;i++){
        c->t_vertices[i] = Vector3Add( c->vertices[i],parent);

        c->aabb.min = Vector3Min(c->aabb.min,c->t_vertices[i]);
        c->aabb.max = Vector3Max(c->aabb.max,c->t_vertices[i]);
    }
}

void UpdateCollider(Collider* c,Matrix transform){
    //Loop trough the points and add them to the 
    c->aabb.min = (Vector3){FLT_MAX,FLT_MAX,FLT_MAX};
    c->aabb.max = (Vector3){-FLT_MAX,-FLT_MAX,-FLT_MAX};
    for(int  i = 0; i < c->numVertices;i++){
        c->t_vertices[i] = Vector3Transform(c->vertices[i],transform);

        c->aabb.min = Vector3Min(c->aabb.min,c->t_vertices[i]);
        c->aabb.max = Vector3Max(c->aabb.max,c->t_vertices[i]);
    }
}

void UnloadCollider(Collider *c){
    //Free all the arrays
    free(c->indices);
    free(c->vertices);
    free(c->t_vertices);
}

#endif

