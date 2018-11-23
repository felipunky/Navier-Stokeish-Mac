#version 330 core
precision mediump float;
layout( location = 0 ) in vec3 aPos; // the position variable has attribute position 0
//layout( location = 2 ) in vec2 aTexCoord; // the position variable has attribute position 0

//out vec4 vertexColour; // specify a color output to the fragment shader

uniform float iTime;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

void main()
{
    
    gl_PointSize = 1.0;
    
    vec3 transformedPos = aPos;
    
    transformedPos.xy = transformedPos.xy * 0.5 + 0.5;
    
    vec2 uv = transformedPos.xy;
    
    vec4 fld = texture( iChannel0, uv );
    
    vec2 pos = ( fld.xy + aPos.xy );
    
    //pos += texture( iChannel1, uv ).zw;
    
    //vertexColour = texture( iChannel2, uv );
    
    gl_Position = vec4( pos, 0, 1 );
    
}
