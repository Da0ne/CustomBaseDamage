class DamageGuardRegistry
{
	/*
	* Here you can define logic for which zone parts can be hit based on what is constructed
	* Hierarchy based logic that you can define which certain parts can take damage or not if certain parts are built
	*/
	protected void RegisterDefaults()
	{
		//Vanilla DayZ Example:
		{
			DamageGuardProfile _Fence = new DamageGuardProfile("Fence");

			/*
			 - Down/Up part of Base cannot be hit if wood/metal down wall exists
			 - Down/Up part of Base cannot be hit if gate is built
			*/
			_Fence.Guard("Wall_Base_Down", {"Wall_Wood_Down", "Wall_Metal_Down", "Wall_Gate"});
			_Fence.Guard("Wall_Base_Up", {"Wall_Wood_Up", "Wall_Metal_Up", "Wall_Gate"});

			Register(_Fence);
		}
	}

	protected ref map<string, ref DamageGuardProfile> m_Profiles;

	void DamageGuardRegistry()
	{
		m_Profiles = new map<string, ref DamageGuardProfile>();
		RegisterDefaults();
	}

	DamageGuardProfile GetProfile(string typeName)
	{
		return m_Profiles[typeName];
	}

	void Register(DamageGuardProfile profile)
	{
		m_Profiles.Set(profile.TypeName, profile);
	}
}