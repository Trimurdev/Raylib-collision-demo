#version 330

/*
 * This file is part of Collision Demo using raylib.
 *
 * You are free to copy, modify, and distribute this file, even for commercial purposes, all without asking permission.
 * For more information, please refer to the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication:
 * https://creativecommons.org/publicdomain/zero/1.0/
 */
//Input parameters
in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;

out vec4 finalColor;

//Uniform colors
uniform vec4 colDiffuse;
uniform vec3 viewPos;

//Probably shouldn't be hardcoded in
vec3 LightPos = vec3(4,8,4);
vec3 LightColor = vec3(0.1,0.1,0.1);



void main()
{
   
    vec4 texelColor = colDiffuse;
    //Diffuse
	vec3 viewDir = normalize(viewPos - fragPosition);
	vec3 lightDir = normalize(LightPos - fragPosition);
	float NdotL = max(0.1f,dot(fragNormal, lightDir));
    float spec = pow(max(0.0, dot(viewDir, reflect(-(lightDir), fragNormal))), 16.0); // 16 refers to shine
	
	vec3 diffuse = texelColor.rgb *  NdotL;

    vec3 specular = texelColor.rgb*spec;

    finalColor = vec4((diffuse+specular),1);
}