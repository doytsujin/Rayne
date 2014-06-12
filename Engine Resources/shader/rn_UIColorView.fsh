//
//  rn_UIAlphaBackground.fsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

uniform vec4 diffuse;
in vec2 vertTexcoord;

out vec4 fragColor0;

void main()
{
	float blendFactor = 8.0 * (vertTexcoord.x - vertTexcoord.y) + 0.5;
	blendFactor = clamp(blendFactor, 0.0, 1.0);
	blendFactor = blendFactor * blendFactor * (3.0 - 2.0 * blendFactor);
	fragColor0.rgb = mix(vec3(1.0), vec3(0.0), blendFactor);
	fragColor0.rgb = mix(fragColor0.rgb, diffuse.rgb, diffuse.a);
	fragColor0.a = 1.0;
}