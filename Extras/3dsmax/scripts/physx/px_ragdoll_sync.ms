--px_ragdoll_sync.ms is for sync pose info of the max biped system with corresponding physX hulls. 
--Author : Hypark Piao
--Contact : hpiao@nvidia.com
--Copyright (C) 2008-2009 nVIDIA Corp.
--For conditions of distribution and use, see copyright notice in http://www.nvidia.com/PhysX.

filein "px_base.ms"

--purpose: Find the corresponding hull node for a given biped node.
--@param bipNode : a node of the biped system.
--@return the handle of the object, undefined for fail
function find_hullNode bipNode =
(	
	local hullName = "hull_" + bipNode.name as string
	hull = findobject hullName
	return hull
)

--purpose: Call this in simulation step for ragdoll.
--details:   The way is to sync & update user selected Biped model's posture with hulls 
--             &joints representation in PhysX scene by traversing 3dMax Biped system's bone
--             system's hierarchically and assign the  corresponding rotation quaternion info to each bones.
--@param bipObj : the root node of a biped system.
--@return null
function sync_ragdoll bipObj = 
(
	--if not biped root then return false
	if ( classOf(bipObj)!=Biped_Object )  do
	(
		messagebox( "error: the object is bot a biped root!" )
		return false
	)
	
	nNode = biped.maxNumNodes bipObj
	nLink = biped.maxNumLinks bipObj

	--traverse the biped hiarachy to sync
	for i = 1 to nNode do 
	( 
		curNode = biped.getNode bipObj i

		if curNode != undefined do
		( 
			for j = 1 to nLink do
			( 
				curLink = biped.getNode bipObj i link:j
				if curLink != undefined do
				(
					--assign the correct PRS info to selected Biped.
					local hull = find_hullNode curLink
					if hull != undefined do
					(
						local qtn = conjugate hull.rotation
						biped.setTransform curLink #rotation qtn true
					)
				)
			)
		)
	)
	
	--get the pelvis node
	local bipPelvis = biped.getNode bipObj #pelvis
	local hullPelvis = find_hullNode bipPelvis 
	if hullPelvis!=undefined do
		biped.setTransform bipObj #pos hullPelvis.pos true
	
	return true
)

