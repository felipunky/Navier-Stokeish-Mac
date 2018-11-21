#version 330 core
out vec4 fragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform sampler2D iChannel4;

void main()
{
    
    vec2 uv = gl_FragCoord.xy / iResolution;
    
    fragColor = texture( iChannel1, uv );// * vec4( 0.5, 0.2, 2.0, 1.0 );
    fragColor += 0.02 * texture( iChannel2, uv );
    fragColor += texture( iChannel3, uv );
    //fragColor = texture( iChannel3, uv );
    //fragColor = texture( iChannel4, uv );
    
}
