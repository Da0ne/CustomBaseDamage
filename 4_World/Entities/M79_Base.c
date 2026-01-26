modded class M79_Base
{
	protected vector m_PosLastHitProjectile;

	void SetLastHitProjectilePos(vector pos)
	{
		m_PosLastHitProjectile = pos;
	}

	vector GetLastHitProjectilePos()
	{
		if (m_PosLastHitProjectile == vector.Zero)
			return GetPosition();

		return m_PosLastHitProjectile;
	}
}