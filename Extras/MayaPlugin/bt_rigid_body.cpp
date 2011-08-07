#include "collision_shape_impl.h"
#include "bt_rigid_body.h"
#include "solver_impl.h"

void bt_rigid_body_t::register_name(solver_impl_t* solver, const char* objectName)
{
	solver->register_name(body(),objectName);
	solver->register_name(body()->getCollisionShape(),objectName);
}
