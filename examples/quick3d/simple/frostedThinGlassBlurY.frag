#define QDEMON_ENABLE_UV0 1
#define QDEMON_ENABLE_WORLD_POSITION 1
#define QDEMON_ENABLE_TEXTAN 1

vec3 texCoord0;

uniform sampler2D BlurBuffer;
uniform sampler2D OriginBuffer;

void main()
{
    vec2 texSize = vec2( textureSize( BlurBuffer, 0 ) );
    texSize = vec2(1.0) / texSize;
    texCoord0.z = 0.0;
    texCoord0.xy = vec2(gl_FragCoord.xy * texSize);

    float sigma = clamp(blur_size * 0.5, 0.5, 100.0);
    int smpCount = int(ceil( sigma ));
    vec4 value = texture(BlurBuffer, texCoord0.xy);

    float wtsum = 1.0;
    for (int i = 1; i &lt;= smpCount; ++i)
    {
        // Base 2 Gaussian blur
        float wt = float(i) / (sigma * 0.5);
        wt = exp2( -wt*wt );
        vec2 texOfs = vec2(0, i) * texSize;
        vec4 posValue = texture(BlurBuffer, texCoord0.xy+texOfs);
        vec4 negValue = texture(BlurBuffer, texCoord0.xy-texOfs);
        posValue = posValue.a * posValue + (1.0 - posValue.a) * texture(OriginBuffer, texCoord0.xy+texOfs);
        negValue = negValue.a * negValue + (1.0 - negValue.a) * texture(OriginBuffer, texCoord0.xy-texOfs);

        value += wt * posValue;
        value += wt * negValue;
        wtsum += wt * 2.0;
    }

    gl_FragColor = (value / wtsum);
    gl_FragColor.a = 1.0;

    // No close paren because the generator adds it for us.
