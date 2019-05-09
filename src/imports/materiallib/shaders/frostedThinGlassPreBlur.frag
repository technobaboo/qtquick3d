#define QDEMON_ENABLE_UV0 1
#define QDEMON_ENABLE_WORLD_POSITION 1
#define QDEMON_ENABLE_TEXTAN 1

vec3 texCoord0;

uniform sampler2D OriginBuffer;

void main()
{
    vec2 texSize = vec2( textureSize( OriginBuffer, 0 ) );
    texSize = vec2(1.0) / texSize;
    texCoord0.z = 0.0;
    texCoord0.xy = vec2(gl_FragCoord.xy * 2.0 * texSize);

    float wtSum = 0.0;
    vec4 totSum = vec4(0.0);
    for (int ix = -1; ix <= 1; ++ix)
    {
       for (int iy = -1; iy <= 1; ++iy)
       {
        float wt = float(ix*ix + iy*iy) * 4.0;
        wt = exp2( -wt );
        vec2 texOfs = vec2(ix, iy) * texSize;
        totSum += wt * texture( OriginBuffer, texCoord0.xy + texOfs );
        wtSum += wt;
       }
    }

    totSum /= wtSum;
    gl_FragColor = totSum;
    // No close paren because the generator adds it for us.
