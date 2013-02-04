#version 150
precision highp float;

in vec2 position;
in vec2 texcoord0;

out vec2 texcoord;
out vec2 blurTexcoords[14];

void main()
{
	texcoord = texcoord0;

	blurTexcoords[ 0] = texcoord + vec2(0.0, -0.028);
	blurTexcoords[ 1] = texcoord + vec2(0.0, -0.024);
	blurTexcoords[ 2] = texcoord + vec2(0.0, -0.020);
	blurTexcoords[ 3] = texcoord + vec2(0.0, -0.016);
	blurTexcoords[ 4] = texcoord + vec2(0.0, -0.012);
	blurTexcoords[ 5] = texcoord + vec2(0.0, -0.008);
	blurTexcoords[ 6] = texcoord + vec2(0.0, -0.004);
	blurTexcoords[ 7] = texcoord + vec2(0.0,  0.004);
	blurTexcoords[ 8] = texcoord + vec2(0.0,  0.008);
	blurTexcoords[ 9] = texcoord + vec2(0.0,  0.012);
	blurTexcoords[10] = texcoord + vec2(0.0,  0.016);
	blurTexcoords[11] = texcoord + vec2(0.0,  0.020);
	blurTexcoords[12] = texcoord + vec2(0.0,  0.024);
	blurTexcoords[13] = texcoord + vec2(0.0,  0.028);
	
	gl_Position = vec4(position, 0.0, 1.0);
}