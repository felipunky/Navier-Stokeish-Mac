#version 330 core

precision lowp float;

in vec4 vertexColour;
out vec4 fragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

void main()
{

    fragColor = vertexColour;

}
