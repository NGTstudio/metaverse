/*=====================================================================
BikePhysics.h
-------------
Copyright Glare Technologies Limited 2023 -
=====================================================================*/
#pragma once


#include "PlayerPhysics.h"
#include "VehiclePhysics.h"
#include "PhysicsObject.h"
#include "PlayerPhysicsInput.h"
#include "Scripting.h"
#include "../physics/jscol_boundingsphere.h"
#include "../maths/Vec4f.h"
#include "../maths/vec3.h"
#include <vector>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>


class CameraController;
class PhysicsWorld;


struct BikePhysicsSettings
{
	GLARE_ALIGNED_16_NEW_DELETE

	Scripting::VehicleScriptedSettings script_settings;
	float bike_mass;
};


/*=====================================================================
BikePhysics
-----------

=====================================================================*/
class BikePhysics : public VehiclePhysics
{
public:
	GLARE_ALIGNED_16_NEW_DELETE

	BikePhysics(WorldObjectRef object, BikePhysicsSettings settings, PhysicsWorld& physics_world);
	~BikePhysics();

	WorldObject* getControlledObject() override { return world_object; }

	void startRightingVehicle() override;

	void userEnteredVehicle(int seat_index) override; // Should set cur_seat_index

	void userExitedVehicle() override; // Should set cur_seat_index

	virtual void playVehicleSummonedEffects() override; // To allow playing of special effects for summoning

	VehiclePhysicsUpdateEvents update(PhysicsWorld& physics_world, const PlayerPhysicsInput& physics_input, float dtime) override;

	Vec4f getFirstPersonCamPos(PhysicsWorld& physics_world) const override;

	Vec4f getThirdPersonCamTargetTranslation() const override;

	Matrix4f getBodyTransform(PhysicsWorld& physics_world) const override;

	// Sitting position is (0,0,0) in seat space, forwards is (0,1,0), right is (1,0,0)
	Matrix4f getSeatToWorldTransform(PhysicsWorld& physics_world) const override;

	Vec4f getLinearVel(PhysicsWorld& physics_world) const override;

	const Scripting::VehicleScriptedSettings& getSettings() const override { return settings.script_settings; }

	void updateDebugVisObjects(OpenGLEngine& opengl_engine, bool should_show) override;

private:
	Matrix4f getWheelToWorldTransform(PhysicsWorld& physics_world, int wheel_index) const;
	void removeVisualisationObs();

	BikePhysicsSettings settings;
	WorldObject* world_object;
	PhysicsWorld* m_physics_world;
	OpenGLEngine* m_opengl_engine;
	JPH::BodyID bike_body_id;

	float time_since_spawn; // For spawning special effect
	float righting_time_remaining;
	
	JPH::Ref<JPH::VehicleConstraint>		vehicle_constraint; // The vehicle constraint
	JPH::Ref<JPH::VehicleCollisionTester>	collision_tester; // Collision testers for the wheel

	float cur_steering_right;

	float smoothed_desired_roll_angle;

	float cur_target_tilt_angle;

	// Debug vis:
	Reference<GLObject> body_gl_ob;
	Reference<GLObject> wheel_attach_point_gl_ob[2];
	Reference<GLObject> wheel_gl_ob[2];
	Reference<GLObject> coll_tester_gl_ob[2];
	Reference<GLObject> contact_point_gl_ob[2];
	Reference<GLObject> contact_laterial_force_gl_ob[2];
	Reference<GLObject> righting_force_gl_ob;
	Reference<GLObject> desired_bike_up_vec_gl_ob;
	
	Vec4f last_desired_up_vec;
	Vec4f last_force_point;
	Vec4f last_force_vec;

	float last_roll_error;

	int steering_node_i;
	int back_arm_node_i;
	int front_wheel_node_i;
	int back_wheel_node_i;
	int upper_piston_left_node_i;
	int upper_piston_right_node_i;
	int lower_piston_left_node_i;
	int lower_piston_right_node_i;
};