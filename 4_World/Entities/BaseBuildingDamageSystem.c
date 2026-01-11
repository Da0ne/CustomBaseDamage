#ifdef SERVER

#define BBDS_ENABLE_DEBUG_LOGGING //Uncomment to enable debug scripts prints

/*
* Constants for damage properties & settings
* FALL OFF POWER: 1.0 = linear, >1 = steeper drop (less damage when further away), <1 = flatter
* Changing falloffPower to higher value will cause a significant damage drop off based on distance!
*/
const float BBDS_FALLOFF_POWER_CLAYMORE = 8.0;
const float BBDS_FALLOFF_POWER_PLASTIC  = 8.0;
const float BBDS_FALLOFF_POWER_GRENADES = 10.6;
const float BBDS_FALLOFF_POWER_40MM 	= 9.0;
const float BBDS_FALLOFF_POWER_OTHER 	= 10.6;

typedef Param2<vector, vector> BBDSPosParams;

class BaseBuildingDamageSystem
{
	static ref BaseBuildingDamageSystem m_Instance;
	static bool CreateInst = Init();
	static bool Init()
	{
		m_Instance = new BaseBuildingDamageSystem();
		return true;
	}

	void BaseBuildingDamageSystem()
	{
		g_Game.m_ExplosionEffectsExInvoker.Insert(ExplosionEffectsEx);
	}

	/*
	* Engine event fired when launching an explosive from a weapon explodes on target/world
	*/
	void ExplosionEffectsEx(Object src, HitInfo hitInfo)
	{
		string ammoType = hitInfo.GetAmmoType();
		ammoType.ToLower();

		if (ammoType == "explosion_40mm_ammo")
		{
			array<BaseBuildingBase> hitTargets = BaseBuildingDamageSystem.FetchBaseBuildingTargets(hitInfo.GetPosition(), BaseBuildingDamageSystem.GetSplashDamageDistance(ammoType));
			
			if (!hitTargets || hitTargets.Count() <= 0)
				return;

			//validate target has been hit by this explosive
			foreach(BaseBuildingBase target : hitTargets)
			{
				if (!target)
					continue;

				if (!target.FindUnprocessedDamageSrc(src))
				{
					Print(string.Format("ExplosionEffectsEx -> target %1 was not processed by engine damage, applying custom!", target));
					target.ApplyEstimateDamage(hitInfo.GetPosition(), ammoType, BBDS_FALLOFF_POWER_40MM);
				}
			}
		}
	}

	/*
	* SceneGetEntitiesInBox is faster than triggers and GetObjectsAtPosition3D :)
	* worst case scenario the scan range is 30 meters~
	*/
	static array<BaseBuildingBase> FetchBaseBuildingTargets(vector pos, float range)
	{
		//Calculate search dynamic range
		float dynamicRange = CalculateMaxSplashDistance(pos, range);
		if (dynamicRange > 1.0){
			range = dynamicRange;
		}

		Print("FetchBaseBuildingTargets :: RANGE -> " + range);

		array<BaseBuildingBase> entities = {};
		vector minPos = pos - Vector(range, range / 2, range);
		vector maxPos = pos + Vector(range, range / 2, range);
		
		array<EntityAI> nearestObjects = {};
		DayZPlayerUtils.SceneGetEntitiesInBox(minPos, maxPos, nearestObjects, QueryFlags.DYNAMIC);

		for (int i = nearestObjects.Count() - 1; i >= 0; --i)
		{
			BaseBuildingBase entity;
			if (Class.CastTo(entity, nearestObjects[i]))
			{
				vector objPos = entity.GetPosition();
				float distance = vector.Distance(objPos, pos);
				if (distance < range)
				{
					entities.Insert(entity);
				}
			}
		}
		return entities;
	}

	/*
	* Fetch entities with cone targeting, this could potentially be performance heavy
	* when there are 50+ entities within the cones viewing angles...
	* ONLY used with Claymore deployment/setup 
	*/
	static array<BaseBuildingBase> FetchBaseBuildingTargetsCone(ExplosivesBase src, vector pos, vector dir, float dist, float verticalAngleDeg, float horizontalAngleDeg)
	{
		if (!src)
			return NULL;

		if (dist <= 0)
			return NULL;

		vector fwd = dir;
		fwd[1] = 0;
		if (fwd.LengthSq() < 0.0001)
			return NULL;

		fwd.Normalize();

		float halfVertRad = verticalAngleDeg * Math.DEG2RAD;
		float halfHeight  = Math.Tan(halfVertRad) * dist;

		float minH = -halfHeight;
		float maxH =  halfHeight;

		array<Object> hits = {};
		array<BaseBuildingBase> outEntities = {};

		DayZPlayerUtils.GetEntitiesInCone(pos, fwd, horizontalAngleDeg, dist, minH, maxH, hits);

		float cosH = Math.Cos(horizontalAngleDeg * Math.DEG2RAD);

		//collision mask
		int layers = 0;
		layers |= PhxInteractionLayers.TERRAIN;
		layers |= PhxInteractionLayers.ROADWAY;
		layers |= PhxInteractionLayers.ITEM_LARGE;
		layers |= PhxInteractionLayers.BUILDING;
		layers |= PhxInteractionLayers.VEHICLE;
		layers |= PhxInteractionLayers.RAGDOLL;

		// Normalize full 3D direction for the ray nudge
		vector rayDir = dir;
		if (rayDir.LengthSq() > 0.0001)
			rayDir.Normalize();
		else
			rayDir = fwd; // fallback

		foreach (Object o : hits)
		{
			BaseBuildingBase bbb;
			if (!Class.CastTo(bbb, o))
				continue;

			//check if this entity already added to result out, skip!
			if (outEntities.Find(bbb) != INDEX_NOT_FOUND)
				continue;

			vector to = bbb.GetPosition() - pos;
			float dSq = to.LengthSq();
			if (dSq > dist * dist)
				continue;

			vector toXZ = to;
			toXZ[1] = 0;
			if (toXZ.LengthSq() < 0.0001)
				continue;

			toXZ.Normalize();

			float dotH = vector.Dot(fwd, toXZ);
			if (dotH < cosH)
				continue;

			float horiz = Math.Sqrt((to[0] * to[0]) + (to[2] * to[2]));
			float pitchAbsDeg = Math.AbsFloat(Math.Atan2(to[1], horiz) * Math.RAD2DEG);
			if (pitchAbsDeg > verticalAngleDeg)
				continue;

			//Extended logic with BulletRaycasting for more accuracy
			//Ray start = cone origin (pos), ray end = target "center" point on the building.
			vector rayStart = pos + (rayDir * 0.05); //small nudge to avoid starting in collider

			vector rayEnd = bbb.GetPosition(); // fallback
			vector minMax[2];
			if (bbb.GetCollisionBox(minMax))
			{
				//minMax is in model space, convert center to world
				vector centerMS = (minMax[0] + minMax[1]) * 0.5;
				rayEnd = bbb.ModelToWorld(centerMS);
			}
			else
			{
				//if no collision box, at least aim a bit above pivot
				rayEnd = rayEnd + Vector(0, 1.0, 0);
			}

			vector contact_pos, contact_dir;
			float  hitFraction;
			Object hitObject;

			bool hit = DayZPhysics.RayCastBullet(rayStart, rayEnd, layers, src, hitObject, contact_pos, contact_dir, hitFraction);
			if (!hit || !hitObject)
				continue;

			//accept only if the FIRST hit along the ray is this BaseBuildingBase (or something parented to it)
			bool hitThisBBB = false;

			BaseBuildingBase hitBBBDirect;
			if (Class.CastTo(hitBBBDirect, hitObject))
			{
				hitThisBBB = (hitBBBDirect == bbb);
			}
			else
			{
				//walk up hierarchy for EntityAI hits (attachments/proxies)
				EntityAI hitEnt = EntityAI.Cast(hitObject);
				while (hitEnt)
				{
					BaseBuildingBase hitBBBUp;
					if (Class.CastTo(hitBBBUp, hitEnt))
					{
						hitThisBBB = (hitBBBUp == bbb);
						break;
					}
					hitEnt = hitEnt.GetHierarchyParent();
				}
			}

			if (!hitThisBBB)
				continue;

			if (outEntities.Find(bbb) == INDEX_NOT_FOUND)
				outEntities.Insert(bbb);
		}

		return outEntities;
	}

	static float CalculateMaxSplashDistance(vector center, float radius, Object ignore = null, int iterations = 24, int layer = ObjIntersectFire)
	{
		float maxDist = 0.0;

		array<ref BBDSPosParams> positions = {};

	#ifdef DIAG_DEVELOPER
		Debug.DestroyAllShapes();
	#endif

		float increment = Math.PI * (3.0 - Math.Sqrt(5.0));
		float offset    = 2.0 * radius / ((float)iterations - 1.0);

		for (int i = 0; i < iterations; i++)
		{
			// generating points on a sphere at radius
			float z     = -radius + i * offset;
			float r     = Math.Sqrt(radius * radius - z * z);
			float angle = i * increment;
			float x     = r * Math.Cos(angle);
			float y     = r * Math.Sin(angle);

			vector e_pos = center + Vector(x, z, y);

			vector contact_pos, contact_dir;
			float  hitFraction;
			Object hitObject;

			int layers = 0;
			layers |= PhxInteractionLayers.TERRAIN;
			layers |= PhxInteractionLayers.ROADWAY;
			layers |= PhxInteractionLayers.ITEM_LARGE;
			layers |= PhxInteractionLayers.BUILDING;
			layers |= PhxInteractionLayers.VEHICLE;
			layers |= PhxInteractionLayers.RAGDOLL;

			bool hit = DayZPhysics.RayCastBullet(center, e_pos, layers, ignore, hitObject, contact_pos, contact_dir, hitFraction);
			if (hit)
			{
				positions.Insert(new Param2<vector, vector>(center, contact_pos));
			}

	#ifdef DIAG_DEVELOPER
			if (hit)
				Debug.DrawArrow(center, contact_pos, 0.05, COLOR_WHITE);
			else
				Debug.DrawArrow(center, e_pos, 0.05, COLOR_YELLOW);
	#endif
		}

		//compute max distance from collected positions
		for (int p = 0; p < positions.Count(); p++)
		{
			BBDSPosParams pp = positions[p];
			float d = vector.Distance(pp.param1, pp.param2);
			if (d > maxDist)
				maxDist = d;
		}

		//nothing was collected for some reason, fall back to radius
		if (positions.Count() == 0)
			return radius;

		return maxDist;
	}

	/*
	* Returns config.cpp based damage distance parameters on given ammoType
	*/
	static float GetSplashDamageDistance(string ammoType, bool useMultiplier = true)
	{
		float indirectRange = g_Game.ConfigGetFloat("CfgAmmo " + ammoType + " indirectHitRange");
		if (useMultiplier)
		{
			float indirectRangeMulti = g_Game.ConfigGetFloat("CfgAmmo " + ammoType + " indirectHitRangeMultiplier");
			return indirectRange * indirectRangeMulti;
		}
		return indirectRange;
	}

	/*
	* Returns config.cpp based damage parameters on given ammoType
	*/
	static void GetHitAngle(string ammoType, out float angV, out float angH)
	{
		string cfgPathV = "CfgAmmo " + ammoType + " indirectHitAngle1";
		string cfgPathH = "CfgAmmo " + ammoType + " indirectHitAngle2";

		if (!g_Game.ConfigIsExisting(cfgPathV) || !g_Game.ConfigIsExisting(cfgPathH))
		{
			BBDS_Print("BaseBuildingDamageSystem::GetHitAngle ! ERROR ! " + ammoType + " doesn't have angles defined!");
			return;
		}

		angV = g_Game.ConfigGetFloat(cfgPathV);
		angH = g_Game.ConfigGetFloat(cfgPathH);
	}
}

void BBDS_Print(string d)
{
	#ifdef BBDS_ENABLE_DEBUG_LOGGING
		Print(string.Format("[BBDS| %1 ]", d));
	#endif
}

#endif