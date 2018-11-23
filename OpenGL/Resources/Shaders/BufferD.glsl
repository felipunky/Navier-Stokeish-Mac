#version 330 core
precision mediump float;
in vec4 vertexColour;
out vec4 fragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

void main()
{
    
    vec2 uv = gl_FragCoord.xy / iResolution;
    
    vec4 fin = vertexColour;
    
    fin += texture( iChannel1, uv ) * 0.99;
    
    fragColor = fin;
    
    //fragColor = vertexColour;
    
}
