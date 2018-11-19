
#version 330 core
layout( location = 0 ) in vec3 aPos; // the position variable has attribute position 0
//layout( location = 2 ) in vec2 aTexCoord; // the position variable has attribute position 0

out vec4 vertexColour; // specify a color output to the fragment shader

uniform float iTime;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform sampler2D iChannel0;

const float siz = 0.2;

void main()
{

    gl_PointSize = 1.0;

    vec3 transformedPos = aPos;

    transformedPos.xy = transformedPos.xy * 0.5 + 0.5;

    vec2 uv = transformedPos.xy;

    vec4 fld = texture( iChannel0, uv ).xyzw * 0.1;

    vec2 pos = fld.xy + aPos.xy;

    vertexColour = vec4( fld.w );

    gl_Position = vec4( pos, 0, 1 );

}
