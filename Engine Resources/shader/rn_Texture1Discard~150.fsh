//
//  rn_Texture1Discard.fsh
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

#include "rn_Lightning.fsh"

uniform sampler2D mTexture0;

in vec2 outTexcoord;
out vec4 fragColor0;

void main()
{
	vec4 color0 = texture(mTexture0, outTexcoord);
	if(color0.a < 0.3)
		discard;

	fragColor0 = color0 * rn_Lightning();
}
