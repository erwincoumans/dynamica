# Introduction #
Disney Animation Studios contributed their in-house Maya plugin called 'Dynamica' to the Bullet project. This Dynamica plugin was used for physics special effects in the creation of the movie Bolt.

# Features #
  * Dynamica is cross-platform and works on Windows, Linux and Mac OSX
  * Dynamica runs on Maya 8, 2008, 2009, 2010 and 2011.
  * Dynamica can simulate rigid body dynamics within Maya
  * The plugin exports to COLLADA Physics format and the native .bullet binary physics file format
  * Constraint support for point to point, hinge, slider and generic 6 degree of freedom constraints

# Basics #

The functionality is provided by four types of custom Maya nodes that interact with each other: dSolver, dRigidBody, dRigidBodyArray, dCollisionShape. The different nodes can be created thru the Dynamica UI. This UI can be accessed by clicking on the Dynamica icon in the EfxToolsLumiere shelf.

## dSolver ##

There is only one of this nodes per scene. It controls the global parameters of the simulation, such as time step, library used for the simulation, ecc. It is created automatically every time a Rigid Body or a Rigid Body array is created.

## dRigidBody ##

There can be multiple rigid bodies in the scene. It has different attributes that control it's behavior, such as mass, damping, initial position/velocity, ecc. It can be active or passive. An active rigid body is controlled by the forces and collisions in the simulation. A passive rigid body (sometimes called Kinematic) is just keyframed.
## dCollisionShape ##

Represents the shape that is used to compute the collisions between rigid bodies. Every rigid body connects to a collision shape. Best practice is to use a collision shape that is good enough to represent the desired collision behavior. The simpler the collision shape, the faster the simulation. Currently the collision shape available are: sphere, box, infinite plane, convex hull, mesh. The convex hull collision shape and the mesh connect directly to a Maya mesh. The mesh collision shape is more precise for concave meshes, otherwise the convex hull shape should be used, because it's faster.
## dRigidBodyArray ##

It's like a rigid body but can contain multiple rigid bodies sharing the same collision shape and simulation parameters. It's the node of choice when the rigid bodies have the same shape and speed is required. The attributes that control the initial configuration are: NumRigidBodies, InitialPosition (Multi), InitialRotation (Multi), InitialVelocity (Multi), InitialSpin (Multi)
# Quick Start #

The following are simple examples that show the basic functionalities of Dynamica. All the command can be accessed thru the Dynamica UI from the EfxToolsLumiere shelf.
## Simple rolling sphere ##

In the dynamica UI, click on "Create passive plane". It creates an infinite plane centered in (0,0,0). With the rotation manipulator, tilt the plane a little bit.
Deselect the plane. (Important: when creating a rigid body, if something is selected, the newly created rigid body has it's initial position and rotation set to the one of the selected object). Click on "Create active sphere". It creates an active rigid body with a sphere as collision shape, centered in (0,0,0). With the move manipulator, move it up to (0,10,0).
Now roll back the time slider to the first frame and hit play (Remember to extend the animation range as necessary).
In general, it's good to roll back the animation to the first frame every time new objects are created, to allow everything to synch up. By taking a look at the hypergraph for the rigid body, it's possible to have a better idea on how things are organized.
## Arbitrary Mesh ##

Create a polygonal mesh in Maya, for example, an helix. With the helix still selected, click on "Create active Mesh". A new rigid body is created, with the helix as input to the Mesh collision shape. Alternatively, "Create active Convex Hull" could be used. The convex hull provides faster simulations at the expense of less precise collisions (if the input mesh is convex, the is no difference between Convex Hull and Mesh collision shapes in term of precision). Then repeat the same step of the Simple rolling sphere example.
## Rigid Body Array ##

Create a maya polygonal mesh in Maya, for example a torus. Resize the torus so that the size of the bounding box is around 1x1x1. With the torus still selected, hit "Create Active Rigid Body Array" in the UI. As small dialog box appears, asking for the dimensions of the array and the offset between the rigid bodies. Enter (10, 20, 10) for the dimensions and (2.5, 2.5, 2.5) as offset.
Create a passive plane as before, then translate the rigid body array so that all donuts are above the plane.
Roll back the simulation, then hit play.
The initial configuration of the rigid body array can be changed by modifying the proper attributes, for example with a mel script. The following is an example mel script that changes the initial configuration of a rigid body array:

```
proc setConfiguration()
{
    //the name of the rigid body array node
    string $rigidBodyArray = "myRigidBodyArray";    

    //set the number of bodies
    setAttr ($rigidBodyArray + ".numBodies") (10 * 10 * 10);
    for($i = 0; $i < 10; $i++) {
        for($j = 0; $j < 10; $j++) {
            for($k = 0; $k < 10; $k++) {
                //set the initial position
                setAttr ($children[0] + ".initialPosition[" + string($i + 10 * $j + 100 * $k) + "]") ($i * 2) (10 + $k * 2) ($j * 2);

                //set the initial rotation to (0, 0, 0)
                setAttr ($children[0] + ".initialRotation[" + string($i + 10 * $j + 100 * $k) + "]") 0 0 0;
            }
        }
    }
}
```

# Kinematic objects #

Kinematic objects are handled by setting the keyframing the parent transform of a passive rigid body. If the keyframed animation is really fast, it's possible that some object would leak thru the kinematic object. To reduce the problem, try increasing the number of substeps in the Solver tab of the Dynamica UI or in the dSolver1 node. Note how the substeps attribute is keyable. This way, it's possible to set a different number of substeps all along the simulation, depending on how complicated it is at each time step, trading speed for accuracy.
Sample scenes

In the subdirectory scenes/ there are some sample setups.
## Online instruction movies for usage of the Maya Plugin ##

You can download (right-click - save) or watch those instruction movies on-line.
  * [Maya Plugin Intro](http://www.bulletphysics.com/ftp/pub/test/index.php?dir=physics/movies/&file=bullet_intro.mp4) (28Mb)
  * [Using Maya Force Fields](http://www.bulletphysics.com/ftp/pub/test/index.php?dir=physics/movies/&file=bullet_fields.mp4) (17Mb)
  * [Creating Donuts](http://www.bulletphysics.com/ftp/pub/test/index.php?dir=physics/movies/&file=bullet_donut.mp4) (11Mb)
  * [Creating Peanuts](http://www.bulletphysics.com/ftp/pub/test/index.php?dir=physics/movies/&file=bullet_peanuts.mp4), used in upcoming Bolt Movie (29Mb)
  * [Part of Bolt trailer with Peanuts shot](http://www.bulletphysics.com/ftp/pub/test/index.php?dir=physics/movies/&file=BoltPeanutsShot.mov) (26Mb)
## Online support forums ##

Visit the Bullet forums for support and feedback: http://bulletphysics.org
and see http://bulletphysics.org/mediawiki-1.5.8/index.php/Maya_Dynamica_Plugin