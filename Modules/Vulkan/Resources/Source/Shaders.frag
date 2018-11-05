//
//  Shaders.glsl
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 450

vec3 light_position = vec3(1.0, 1.0, 1.0);

layout (location = 0) in vec2 inTexcoords;
layout (location = 1) in vec3 inNormals;
layout (location = 2) in vec4 inAmbient;
layout (location = 3) in vec4 inDiffuse;

layout (location = 0) out vec4 fragColor0;

layout (set = 0, binding = 1) uniform sampler linearSampler;
layout (set = 0, binding = 2) uniform texture2D colorMap;

void main()
{
	vec4 color = texture(sampler2D(colorMap, linearSampler), inTexcoords);
	color *= inAmbient + inDiffuse * clamp(dot(normalize(inNormals), normalize(light_position)), 0.0, 1.0);
	fragColor0 = color;
}