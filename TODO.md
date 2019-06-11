# TODO

## UIP Exporter
Add Support for translateing Slides -> States
See if it's possible to translate known custom materials

## Effects
Add support for Post Processing Effects (applied to View3D ouput)

## Quick
Make sure it is possible to have QQuickItems inside of View3D Item (normally this is the scene, but you may want to stack items on top as well).
QML Inside of Scene as DemonImage texture source

## View3D
Add flag for rendering directly to the window 
Move the blend mode's out of SceneEnvironment/View3D.  Instead these are a special kind of shader effect applied between Qt Quick Items, add examples
Add support for picking
Add skybox mode for backround

## Assimp Plugin
Handle non-unique id names case
Import Keyframe animations
GLTF2 Support: Add additional materials/texture import code and test

## Rendering
Get direction shadows working better.
Dynamic Geometry
Add flag for marking models static (needed for batching)
Support Static mesh batching
Multi-MEsh support (instanced rendering)
Support the bakeing of lights
QSGTexture -> QDemonImage integration (needed for rendering Qt Quick inside of 3D)

## Longerterm Investigation
Mesh Morphing / Bone-rigged Animations
3D Partical Engine
Collision / Physics Integration
RenderPlugin (Spatial RenderNode)
