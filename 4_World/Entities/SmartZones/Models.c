class DamageGuardRule
{
	string TargetZone;
	ref array<string> BlockedIfBuiltZones;

	void DamageGuardRule(string targetZone, array<string> blockedIfBuiltZones)
	{
		targetZone.ToLower();
		TargetZone = targetZone;
		BlockedIfBuiltZones = blockedIfBuiltZones;
	}
}

class DamageGuardProfile
{
	string TypeName;
	ref map<string, ref array<string>> GuardsByTarget; // target -> protectors

	void DamageGuardProfile(string typeName)
	{
		typeName.ToLower();
		TypeName = typeName;
		GuardsByTarget = new map<string, ref array<string>>();
	}

	void Guard(string targetZone, array<string> blockedIfBuiltZones)
	{
		targetZone.ToLower();
		for (int i = 0; i < blockedIfBuiltZones.Count(); ++i)
		{
			string lowered = blockedIfBuiltZones[i];
			lowered.ToLower();

			blockedIfBuiltZones[i] = lowered;
		}
		GuardsByTarget.Set(targetZone, blockedIfBuiltZones);
	}
}
