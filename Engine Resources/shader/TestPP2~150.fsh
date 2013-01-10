#version 150
precision highp float;

uniform sampler2D targetmap;

in vec2 texcoord;
out vec4 fragColor0;

void main()
{
	vec4 color = texture(targetmap, texcoord);
	float luminosity = (0.2125 * color.r) + (0.7154 * color.g) + (0.0721 * color.b);
	
	fragColor0 = vec4(luminosity, luminosity, luminosity, color.a);
}
