/*
 * This file is part of Collision Demo using raylib.
 *
 * You are free to copy, modify, and distribute this file, even for commercial purposes, all without asking permission.
 * For more information, please refer to the CC0 1.0 Universal (CC0 1.0) Public Domain Dedication:
 * https://creativecommons.org/publicdomain/zero/1.0/
 */
#version 330

//Parameters provided by raylib
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

//Parameters provided by raylib
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

//Output
out vec3 Position;
out vec2 TexCoord;
out vec3 Normal;



void main()
{
   
    Position = vec3(matModel*vec4(vertexPosition, 1.0));
    TexCoord = vertexTexCoord;
    Normal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}