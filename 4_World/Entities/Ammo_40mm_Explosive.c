#ifdef SERVER

/*
* Defines to toggle certain behaviors
* Uncomment to enable
*/
#define CBD_DISABLE_40MM_GRND_EXPLOSION     //uncomment to disable 40mm from being shot to explode



#ifdef CBD_DISABLE_40MM_GRND_EXPLOSION
modded class Ammo_40mm_Explosive
{
	//override without super, stops exploding behavior when shot at
	override void EEKilled(Object killer)
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DeleteSafe, 1000, false);
	}
}
#else
//! Special case for this retarded shit (only when the explosive round is shot on ground), it doesn't inherent from an ExplosiveBase
modded class Ammo_40mm_Explosive
{
	protected ref Timer	m_DmgTimer;
	protected ref array<BaseBuildingBase> m_dmgTargets; //weak ptr

	void Ammo_40mm_Explosive()
	{
		m_dmgTargets = {};
	}

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		InventoryLocationType locType = newLoc.GetType();
		if (locType == InventoryLocationType.GROUND)
		{
			BBDS_Print(string.Format("%1 inventory location: %2", GetType(), typename.EnumToString(InventoryLocationType, locType)));

			//Fetch around explosive
			m_dmgTargets = BaseBuildingDamageSystem.FetchBaseBuildingTargets(GetPosition(), BaseBuildingDamageSystem.GetSplashDamageDistance("Explosion_40mm_Ammo"));
		}
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		m_DmgTimer = new Timer();
		m_DmgTimer.Run(0.25, this, "VerifyDamage");
	}

	void VerifyDamage()
	{
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
				target.ApplyEstimateDamage(GetPosition(), "Explosion_40mm_Ammo", BBDS_FALLOFF_POWER_40MM);
			}
		}
	}
}
#endif

#endif