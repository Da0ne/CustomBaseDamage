class DamageGuardRegistry
{
	/*
	* Here you can define logic for which zone parts can be hit based on what is constructed
	* Hierarchy based logic that you can define which certain parts can take damage or not if certain parts are built
	*/
	protected void RegisterDefaults()
	{
		//tR_MediumDoor
		{
			DamageGuardProfile _tR_MediumDoor = new DamageGuardProfile("tR_MediumDoor");

			_tR_MediumDoor.Guard("tr_door_base_left", {"tr_door_wood_left", "tr_door_wood_cut_left", "tr_door_metal_Left", "tr_door_metal_cut_left"});
			_tR_MediumDoor.Guard("tr_door_base_right", {"tr_door_wood_right", "tr_door_wood_cut_right", "tr_door_metal_right", "tr_door_metal_cut_right"});


			Register(_tR_MediumDoor);
		}

		//tR_Narrowdoor
		{
			DamageGuardProfile _tR_NarrowDoor = new DamageGuardProfile("tR_NarrowDoor");

			_tR_NarrowDoor.Guard("tr_door_base", {"tr_door_wood", "tr_door_wood_cut", "tr_door_metal", "tr_door_metal_cut"}); //Base can't be damaged if wood is built & or metal

			Register(_tR_NarrowDoor);
		}

		//tR_Fortification
		{
			DamageGuardProfile _tR_Fortification = new DamageGuardProfile("tR_Fortification");

			_tR_Fortification.Guard("tr_level_1_wall_1_door_base", {"tr_level_1_wall_1_door_wood", "tr_level_1_wall_1_door_metal"});
			_tR_Fortification.Guard("tr_level_1_wall_1_gateway_base", {"tr_level_1_wall_1_gateway_wood", "tr_level_1_wall_1_gateway_metal"});

			//Level 1
			_tR_Fortification.Guard("tr_level_1_wall_1_doorway_base", {"tr_level_1_wall_1_doorway_wood", "tr_level_1_wall_1_doorway_metal"});
			_tR_Fortification.Guard("tr_level_1_wall_1_gateleft_base", {"tr_level_1_wall_1_gateleft_wood", "tr_level_1_wall_1_gateleft_metal"});
			_tR_Fortification.Guard("tr_level_1_wall_1_gateright_base", {"tr_level_1_wall_1_gateright_wood", "tr_level_1_wall_1_gateright_metal"});
			
			//Level 2
			_tR_Fortification.Guard("tr_level_2_wall_2_doorway_base", {"tr_level_2_wall_2_doorway_wood", "tr_level_2_wall_2_doorway_metal"});
			//Level 3
			_tR_Fortification.Guard("tr_level_3_wall_3_doorway_base", {"tr_level_3_wall_3_doorway_wood", "tr_level_3_wall_3_doorway_metal"});
			
			Register(_tR_Fortification);
		}


		//Vanilla DayZ Example:
		{
			DamageGuardProfile _Fence = new DamageGuardProfile("Fence");

			/*
			 - Down/Up part of Base cannot be hit if wood(up or down)/metal(up or down)/gate is built exists
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