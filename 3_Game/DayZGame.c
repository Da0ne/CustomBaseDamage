#ifdef SERVER
modded class DayZGame
{
	ref ScriptInvoker m_ExplosionEffectsExInvoker = new ScriptInvoker();

	override void ExplosionEffectsEx(Object source, Object directHit, int componentIndex, float energyFactor, float explosionFactor, HitInfo hitInfo)
	{
		super.ExplosionEffectsEx(source, directHit, componentIndex, energyFactor, explosionFactor, hitInfo);
		m_ExplosionEffectsExInvoker.Invoke(source, hitInfo);
	}
}
#endif