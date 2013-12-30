//
//  rn_Water.vsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

#include "rn_Matrices.vsh"

uniform float time;

in vec3 attPosition;
out vec3 vertProjPos;
out vec3 vertPosition;
out vec2 vertTexcoord;

void main()
{
	vertPosition = (matModel * vec4(attPosition, 1.0)).xyz;
	vertTexcoord = vertPosition.xz*0.1+time*0.1;
	gl_Position = matProjViewModel * vec4(attPosition, 1.0);
	vertProjPos = gl_Position.xyw;
}
