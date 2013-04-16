//
//  rn_Particle.gsh
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 matProj;

in vec4 geoColor[1];
out vec4 fragColor;
out vec2 texCoord;

void main()
{
	vec4 base = gl_in[0].gl_Position;
	vec2 size = vec2(0.5);

	gl_Position = matProj * (base + vec4(size.x, size.y, 0.0, 0.0));
	texCoord = vec2(0.0, 1.0);
	fragColor = geoColor[0];
	EmitVertex();

	gl_Position = matProj * (base + vec4(-size.x, size.y, 0.0, 0.0));
	texCoord = vec2(0.0, 0.0);
	fragColor = geoColor[0];
	EmitVertex();

	gl_Position = matProj * (base + vec4(size.x, -size.y, 0.0, 0.0));
	texCoord = vec2(1.0, 1.0);
	fragColor = geoColor[0];
	EmitVertex();

	gl_Position = matProj * (base + vec4(-size.x, -size.y, 0.0, 0.0));
	texCoord = vec2(1.0, 0.0);
	fragColor = geoColor[0];
	EmitVertex();

	EndPrimitive();
}
