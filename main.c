/*
 * This file is part of Collision Demo using raylib.
 *
 * You are free to copy, modify, and distribute this file, even for commercial purposes, all without asking permission.
 * For more information, please refer to the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication:
 * https://creativecommons.org/publicdomain/zero/1.0/
 */


#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"
#include <stdio.h>
#include "Collision.h"
#define GLSL_VERSION  330


void DrawCollider(Collider* c);

int main(void)
{
    
    const int screenWidth = 800;
    const int screenHeight = 450;
    
	SetConfigFlags(FLAG_MSAA_4X_HINT); 
    InitWindow(screenWidth, screenHeight, "Raylib collision demo");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
	MaximizeWindow();
    //Create the camera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.0f, 10.0f };  
    camera.target = (Vector3){ 0.0f, 1.0f, 1.0f };      
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 90.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            
    //Create player variables
    Vector3 PlayerPosition = { 0.0f, 2.0f, 0.0f };
    Vector3 PlayerVelocity = { 0.0f, 0.0f, 0.0f };
    float playerSpeed = 5.0f;
	//Physics values
	float DampeningValue = 0.24f;
	float Gravity = 9.7f;
    //Set the fps
    SetTargetFPS(-1);  
	//Load in the shader
	Shader shader = LoadShader(TextFormat("src/shaders/shader.vs", GLSL_VERSION),
                               TextFormat("src/shaders/shader.fs", GLSL_VERSION));
    //Tell the shader where the viewPosition location is
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	
	//Load in models
    //Only works with blender meshes since raylib creates the meshes with more normals and triangulates them a bit weird
	Model Cylinder = LoadModel("src/models/cylinder.obj"); 
	Model Floor = LoadModel("src/models/cube.obj");
	Model Ramp = LoadModel("src/models/Ramp.obj");
	Model Sphere = LoadModel("src/models/sphere.obj");
	//Cylinder material setup
	Cylinder.materials[0].shader = shader;
	Cylinder.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
    //Floor material setup
    Floor.materials[0].shader = shader;
    Floor.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    //Ramp material setup
    Ramp.materials[0].shader = shader;
    Ramp.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    //Sphere material setup
    Sphere.materials[0].shader = shader;
    Sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;


	

	//Create the player collider
	Collider Player;
    //Setup player collider
	SetupColliderMesh(&Player,GenMeshCylinder(0.4f,2,6));
	
    //Define the num colliders/objects
	const int NumColliders = 4;
    
	Collider colliders[NumColliders];//Create the collider array
	Vector3 positions[NumColliders];//Create the positions array
    
    //Set all the positions
	positions[0] = (Vector3){-2,1.0f,-2};
	positions[1] = (Vector3){2,1,2};
	positions[2] = (Vector3){0,2,0};
	positions[3] = (Vector3){2,2,-1};
    
   
	//Setup all the collider for the meshes
	SetupColliderMesh(&colliders[0],Ramp.meshes[0]);
	SetupColliderMesh(&colliders[1],Cylinder.meshes[0]);
	SetupColliderMesh(&colliders[2],Floor.meshes[0]);
	SetupColliderMesh(&colliders[3],Sphere.meshes[0]);
    
    //Update the colliders
    for(int i = 0; i < NumColliders;i++)UpdateCollider(positions[i],&colliders[i]);
   
	
    //Disable the cursor so it doesn't get in the way
    DisableCursor();

    //Debug
    bool showDebug = false;
    
    // Main game loop
    while (!WindowShouldClose())    
    {
        float dt = GetFrameTime();
        UpdateCamera(&camera,CAMERA_FIRST_PERSON);

        //Toggle debug
        if(IsKeyPressed(KEY_F3)){
            showDebug = !showDebug;
        }

	
		//Take care of the physics
		PlayerVelocity.y -= Gravity*GetFrameTime(); // Gravity
		PlayerVelocity = Vector3Scale(PlayerVelocity,DampeningValue); //Dampen the velocity
		PlayerPosition = Vector3Add(PlayerPosition,PlayerVelocity);

		//Camera management
        Vector3 Forward = GetCameraForward(&camera);
        Vector3 Right = GetCameraRight(&camera);
        
		
		camera.target = Vector3Add(PlayerPosition,Forward);
		camera.position = PlayerPosition;
        
		//Player movement
		Forward.y = 0;//Set the y to 0 
     
        //Check for player input
		if(IsKeyDown(KEY_W))PlayerPosition = Vector3Add(PlayerPosition,Vector3Scale(Forward,dt * playerSpeed));
		if(IsKeyDown(KEY_S))PlayerPosition = Vector3Subtract(PlayerPosition,Vector3Scale(Forward,dt * playerSpeed));
		
		if(IsKeyDown(KEY_A))PlayerPosition = Vector3Subtract(PlayerPosition,Vector3Scale(Right,dt * playerSpeed));
		if(IsKeyDown(KEY_D))PlayerPosition = Vector3Add(PlayerPosition,Vector3Scale(Right,dt * playerSpeed));
		
		if(IsKeyDown(KEY_SPACE) ){
		PlayerVelocity.y += 4.4f * dt;//Add velocity
		PlayerPosition.y += 4.3f * dt;//Add a bit of position since i set the gravity to 0 when in collision
		}
        
		//Update shader camera position
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
		
		//Take care of the collision
		UpdateCollider(Vector3Subtract(PlayerPosition,(Vector3){0,1,0}),&Player);
        //Loop trough the colliders
		for(int i = 0; i < NumColliders;i++){
			//Update the collider
			//UpdateCollider(positions[i],&colliders[i]); Not needed since the colliders are static
			//Check the collision between the player and other colliders

            if(CheckCollisionBoxes(Player.aabb,colliders[i].aabb)){ //Broadphase 
                BasicCollisionInfo info = CheckCollision(&Player,&colliders[i]);
                if(info.collided){
                    PlayerPosition = Vector3Add(PlayerPosition,Vector3Scale(info.normal,info.depth));
                }
            }
            
		
		}
        
        
        BeginDrawing();//Begin drawing

            ClearBackground(RAYWHITE);//Clear backround

            BeginMode3D(camera);//Set the mode to 3d using our camera
			    //Draw all the models, models are all offset otherwise use position array
                DrawModel(Floor,positions[2],1,WHITE);
                DrawModel(Cylinder,positions[1],1,WHITE);
                DrawModel(Ramp,positions[0],1,WHITE);
                DrawModel(Sphere,positions[3],1,WHITE);
                //Draw the grid
                DrawGrid(10, 1.0f);		

                //Draw debug wireframes
                if(showDebug){
                    DrawCollider(&Player);
                    for (size_t i = 0; i < NumColliders; i++)
                    {
                        DrawCollider(&colliders[i]);
                    }
                }
               
                
            EndMode3D();
               DrawFPS(10, 10);//Draw fps
        EndDrawing();
      
    }
    //Clear up all the data
    //Shaders
    UnloadShader(shader);
    //Models
    UnloadModel(Floor);
    UnloadModel(Cylinder);
    UnloadModel(Ramp);
    UnloadModel(Sphere);
    //Free the colliders
    for(int i = 0; i < NumColliders;i++)UnloadCollider(&colliders[i]);
    
    
    CloseWindow();       

    return 0;
}


void DrawCollider(Collider* c){
    DrawBoundingBox(c->aabb,RED);

    for (size_t i = 0; i < c->numIndices; i+=2)
    {
        DrawLine3D(c->t_vertices[c->indices[i]],c->t_vertices[c->indices[i+1]],GREEN);
    }
    
}