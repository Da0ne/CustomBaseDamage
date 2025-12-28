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
	static array<BaseBuildingBase> FetchBaseBuildingTargetsCone(vector pos, vector dir, float dist, float verticalAngleDeg, float horizontalAngleDeg)
	{
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

		foreach(Object o : hits)
		{
			BaseBuildingBase bbb;
			if (!Class.CastTo(bbb, o))
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

			if (outEntities.Find(bbb) == INDEX_NOT_FOUND)
				outEntities.Insert(bbb);
		}

		return outEntities;
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