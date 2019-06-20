# TODO

## Asset Import framework
Add the pumbing for using plugins options (QVariantMap) + command line arguments for balsam

## UIP Exporter
Add Support for translateing Slides -> States
See if it's possible to translate known custom materials
Fix folder creation (models/maps/materials) for modern UIA projects

## Effects
Add support for Post Processing Effects (applied to View3D ouput, and requires FBO)

## Quick
Make sure it is possible to have QQuickItems inside of View3D Item (normally this is the scene, but you may want to stack items on top as well).
QML Inside of Scene as DemonImage texture source

## View3D
Move the blend mode's out of SceneEnvironment/View3D.  Instead these are a special kind of shader effect applied between Qt Quick Items, add examples
Add support for picking

## Assimp Plugin
Import Keyframe animations
GLTF2 Support: Add additional materials/texture import code and test

## Rendering
Check multisampling support (some modes require FBO's)
Add Progressive AA (requires FBO)
Add Temporal AA (requires FBO)
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
