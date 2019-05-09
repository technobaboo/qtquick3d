// add enum defines
#define QDEMON_ENABLE_UV0 1
#define QDEMON_ENABLE_WORLD_POSITION 1
#define QDEMON_ENABLE_TEXTAN 1

vec3 texCoord0;

void main()
{

    // This is a bit silly, but the thing is that a buffer blit takes place on this
    // pass, and if you do a buffer blit on a pass that outputs to lower-resolution,
    // it only blits a smaller portion of the backbuffer that occupies that number of
    // pixels.  So we need a dummy no-op pass that is full-res in order to blit everything.
