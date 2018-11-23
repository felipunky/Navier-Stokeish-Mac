#version 330 core
precision mediump float;
out vec4 fragColor;

uniform int iFrame;
uniform float iTime;
uniform float iTimeDelta;
uniform float siz;
uniform float iDamping;
uniform float iDiffusion;
uniform float iVorticity;
uniform int iReload;
uniform int iColourFlag;
uniform int iNegativeFlag;
uniform vec2 iResolution;
uniform vec2 iVel;
uniform vec4 iMouse;
uniform vec4 iColour;
uniform vec4 iColourOne;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

const float dx = 0.5;
const float dt = dx * dx * 0.5;
float di = iDiffusion;
const float alp = ( dx * dx ) / dt;
const float rbe = 1.0 / ( 4.0 + alp );
float vo = iVorticity;

// We need this for our hash function
#define HASHSCALE1 .1031

// Dave Hoskin's hash one out two in
float hash( vec2 p )
{
    vec3 p3  = fract( vec3( p.xyx ) * HASHSCALE1 );
    p3 += dot( p3, p3.yzx + 19.19 );
    return fract( ( p3.x + p3.y ) * p3.z );
}

// Divides the 2D space in tiles than those tiles are asigned a random colour
// than we interpolate using GLSL's mix() function to interpolate to combine
// the different random values of each tile into a 2D texture.
float noise( vec2 uv )
{
    
    vec2 lv = fract( uv );
    lv = lv * lv * ( 3.0 - 2.0 * lv );
    vec2 id = floor( uv );
    
    float bl = hash( id );
    float br = hash( id + vec2( 1, 0 ) );
    float b = mix( bl, br, lv.x );
    
    float tl = hash( id + vec2( 0, 1 ) );
    float tr = hash( id + vec2( 1 ) );
    float t = mix( tl, tr, lv.x );
    
    float c = mix( b, t, lv.y );
    
    return c;
    
}


float cur( vec2 uv )
{
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    float top = texture( iChannel0, vec2( x, y + ypi ) ).r;
    float lef = texture( iChannel0, vec2( x - xpi, y ) ).r;
    float rig = texture( iChannel0, vec2( x + xpi, y ) ).r;
    float dow = texture( iChannel0, vec2( x, y - ypi ) ).r;
    
    float dY = ( top - dow ) * 0.5;
    float dX = ( rig - lef ) * 0.5;
    
    return dX * dY;
}

vec2 vor( vec2 uv )
{
    
    vec2 pre = uv;
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    vec2 dir = vec2( 0 );
    dir.y = ( cur( vec2( x, y + ypi ) ) ) - ( cur( vec2( x, y - ypi ) ) );
    dir.x = ( cur( vec2( x + xpi, y ) ) ) - ( cur( vec2( x - xpi, y ) ) );
    
    dir = normalize( dir );
    
    if( length( dir ) > 0.0 )
        
        uv -= dt * vo * cur( uv ) * dir;
    
    return uv;
    
}

vec2 dif( vec2 uv )
{
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    vec2 cen = texture( iChannel0, uv ).xy;
    vec2 top = texture( iChannel0, vec2( x, y + ypi ) ).xy;
    vec2 lef = texture( iChannel0, vec2( x - xpi, y ) ).xy;
    vec2 rig = texture( iChannel0, vec2( x + xpi, y ) ).xy;
    vec2 dow = texture( iChannel0, vec2( x, y - ypi ) ).xy;
    
    return ( di * rbe ) * ( top + lef + rig + dow + alp * cen ) * rbe;
    
}

float dis( vec2 uv, vec2 mou )
{
    
    return length( uv - mou );
    
}

float cir( vec2 uv, vec2 mou, float r )
{
    
    float o = smoothstep( r, r - 0.05, dis( uv, mou ) );
    
    return o;
    
}

vec2 adv( vec2 uv, vec2 p, vec2 mou )
{
    
    vec2 vel = iVel / iResolution;
    
    // Eulerian.
    vec2 pre = texture( iChannel1, vor( uv ) ).xy;
    if( cir( p, mou, siz ) > 0.0 )
    {
        
        if( iMouse.z > 0.5 )
            
            pre = 3.0 * vel;
        
        if( iMouse.w > 0.5 )
            
            pre *= 0.0;
        
    }
    
    pre = iTimeDelta * dt * pre;
    
    uv -= pre;
    
    return uv;
    
}

vec4 forc( vec2 uv, vec2 p, vec2 mou )
{
    
    vec4 col = vec4( 0 );
    
    if( iFrame <= 5 || iReload == 1 )
        
        col += 0.05 * texture( iChannel2, uv );
    
    if( cir( p, mou, siz ) > 0.0 )
    {
        
        if( iMouse.z > 0.5 )
        {
            
            if( iColourFlag == 1 )
                
                col += 0.07 * vec4( noise( uv + iTime * 0.5 ), noise( uv + 2.0 + iTime * 0.5 ), noise( uv + 1.0 + iTime * 0.5 ), 1 );
            
            if( iColourFlag == 0 )
                
                col += 0.07 * iColour;
        }
        
        if( iMouse.w > 0.5 )
        {
            
            if( iNegativeFlag == 0 )
                
                col += 0.07 * iColourOne;
            
            if( iNegativeFlag == 1 )
                
                col -= 0.07;
            
        }
        
    }
    
    return col;
    
}

vec2 div( vec2 uv )
{
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    float cen = texture( iChannel0, uv ).a;
    float top = texture( iChannel0, vec2( x, y + ypi ) ).r;
    float lef = texture( iChannel0, vec2( x - xpi, y ) ).r;
    float rig = texture( iChannel0, vec2( x + xpi, y ) ).r;
    float dow = texture( iChannel0, vec2( x, y - ypi ) ).r;
    
    float dX = ( rig - lef ) * 0.5;
    float dY = ( top - dow ) * 0.5;
    
    return vec2( dX, dY );
    
}

vec2 pre( vec2 uv )
{
    
    vec2 pre = uv;
    
    uv -= ( di * dx * dx ) * div( uv );
    
    return uv;
    
}

vec2 vel( vec2 uv )
{
    
    vec2 pr = pre( uv );
    vec2 die = div( uv );
    
    uv += dt * die - pr;
    
    return uv;
    
}

vec4 fin( vec2 uv, vec2 p, vec2 mou )
{
    
    vec4 col = vec4( 0.0 ); float dam = 1.0; vec4 colO = vec4( 0 ); vec2 pre = uv;
    
    uv = adv( uv, p, mou );
    uv -= dt * ( vel( uv ) * dif( uv ) );
    
    col += forc( uv, p, mou );
    colO = texture( iChannel0, uv ) + col;
    //dam *= 0.99;
    //colO *= dam;
    if( pre.y < 0.00 || pre.x < 0.00 || pre.x > 1.0 || pre.y > 1.0 ) colO *= 0.0;
    
    return colO * iDamping;
    
}

void main( )
{
    
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 p = gl_FragCoord.xy / iResolution.y;
    
    vec2 mou = iMouse.xy / iResolution.y;
    
    vec4 colO = fin( uv, p, mou );
    
    fragColor = colO;
    
}
