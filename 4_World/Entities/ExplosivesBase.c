#ifdef SERVER
/*
* Handles all cases of explosives like Claymores, Plastic explosive & Grenades and Improvised explosives
* In the case of improvised explosives, essentially they are either nades, claymores or IED which is just dropped to the ground
* then set to explode (that's vanilla) so this in theory covers that too.
*/
modded class ExplosivesBase
{
	protected ref Timer	m_DmgTimer;
	protected ref array<BaseBuildingBase> m_dmgTargets; //weak ptr

	void ExplosivesBase()
	{
		m_dmgTargets = {};
	}

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		m_dmgTargets = {}; //reset anytime this entity gets it's location changed
		InventoryLocationType locType = newLoc.GetType();
		if (locType == InventoryLocationType.GROUND)
		{
			BBDS_Print(string.Format("%1 inventory location: %2", GetType(), typename.EnumToString(InventoryLocationType, locType)));

			//Fetch around the explosive
			//Claymore specific
			ClaymoreMine entity;
			if (Class.CastTo(entity, this))
			{
				float dist = BaseBuildingDamageSystem.GetSplashDamageDistance(GetAmmoType());
				float angV, angH = 0.0;
				BaseBuildingDamageSystem.GetHitAngle(GetAmmoType(), angV, angH);

				m_dmgTargets = BaseBuildingDamageSystem.FetchBaseBuildingTargetsCone(this, GetPosition(), GetDirection(), dist, angV, angH);
			}
			else
			{
				//others
				m_dmgTargets = BaseBuildingDamageSystem.FetchBaseBuildingTargets(GetPosition(), BaseBuildingDamageSystem.GetSplashDamageDistance(GetAmmoType()));
			}
		}
	}

	string GetAmmoType()
	{
		if (!m_AmmoTypes)
			return DEFAULT_AMMO_TYPE;

		return m_AmmoTypes[0];
	}

	override protected void OnExplode()
	{
		//Not calling super here to postpone the deletion of Explosion entity
		m_DmgTimer = new Timer();
		m_DmgTimer.Run(0.25, this, "VerifyDamage");
	}

	/*
	* Returns fall off power based on configured constant values
	* checking for type of explosive triggered
	*/
	float GetFallOffPower()
	{
		switch(Type())
		{
		case ClaymoreMine:
			return BBDS_FALLOFF_POWER_CLAYMORE;
			break;

		case Plastic_Explosive:
			return BBDS_FALLOFF_POWER_PLASTIC;
			break;

		case Grenade_Base:
			return BBDS_FALLOFF_POWER_GRENADES;
			break;
		}
		return BBDS_FALLOFF_POWER_OTHER;
	}

	void VerifyDamage()
	{
		//Go ahead with the delete
		m_DeleteTimer.Run(0.30, this, "DeleteSafe");

		if (!m_dmgTargets || m_dmgTargets.Count() <= 0)
			return;

		//validate target has been hit by this explosive
		foreach(BaseBuildingBase target : m_dmgTargets)
		{
			if (!target)
				continue;

			if (!target.FindUnprocessedDamageSrc(this))
			{
				BBDS_Print(string.Format("ExplosivesBase::VerifyDamage -> target %1 was not processed by engine damage, applying custom!", target));
				target.ApplyEstimateDamage(GetPosition(), GetAmmoType(), GetFallOffPower());
			}
		}
	}
}
#endif