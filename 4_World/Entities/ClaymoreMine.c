/*
* Defines to toggle certain behaviors
* Uncomment to enable
*/
#define CBD_DISABLE_CLAYMORE_GRND_EXPLOSION //uncomment to disable Claymores from being shot to explode

#ifdef SERVER
modded class ClaymoreMine
{
	#ifdef CBD_DISABLE_CLAYMORE_GRND_EXPLOSION
	override void EEKilled(Object killer)
	{
		if (!GetArmed())
			return;

		super.EEKilled(killer);
	}
	#endif
}
#endif