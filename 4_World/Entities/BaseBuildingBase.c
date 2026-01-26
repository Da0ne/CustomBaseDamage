#ifdef SERVER

#define BBDS_ENABLE_SMART_ZONE //!comment out to stop smart zone damage protection!
#define BBDS_OVERRIDE_VANILLA  //!comment out to ENABLE vanilla damage system to assist ontop of our custom!

modded class BaseBuildingBase
{
    protected ref map<EntityAI, int> m_dmgSources; //weak ptr
    protected ref array<string> m_dmg_zones;

    void BaseBuildingBase()
    {
        m_dmgSources = new map<EntityAI, int>;
    }

    override void EOnInit(IEntity other, int extra)
    {
        BBDS_Print("BaseBuildingBase::EOnInit!");

        DamageZoneMap zonesMap = new DamageZoneMap();
        DamageSystem.GetDamageZoneMap(this, zonesMap);
        m_dmg_zones = zonesMap.GetKeyArray();
    }

    /*
    * NOTE: Explicit forced NO SUPER call in this method to avoid other mods breaking THIS logic. 
    */
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
        /*
        ItemBase m_itm;
        if(Class.CastTo(m_itm, source))
        {
            if(!m_itm.IsRaidWpn())
            {
                return false;
            }
        }
        */

        if (source && damageType == DamageType.EXPLOSION)
        {
            //record damage source
            if (!m_dmgSources)
                m_dmgSources = new map<EntityAI, int>;

            m_dmgSources.Insert(source, 1);
            BBDS_Print(string.Format("%1::InsertDamageSource -> %2 COUNT: %3", GetType(), source, m_dmgSources.Count()));

#ifdef BBDS_ENABLE_SMART_ZONE
            BBDS_Print("<< IGNORE VANILLA DAMAGE >>");

            float falloffPower = BBDS_FALLOFF_POWER_OTHER;
            vector dmgPos;
            M79_Base launcher;
            ExplosivesBase explosive;
            if (Class.CastTo(explosive, source))
            {
                falloffPower = explosive.GetFallOffPower();
                dmgPos       = source.GetPosition();
            }
            else if (Class.CastTo(launcher, source))
            {
                falloffPower = BBDS_FALLOFF_POWER_40MM;
                dmgPos       = launcher.GetLastHitProjectilePos();
            }
            
            ApplyEstimateDamage(dmgPos, ammo, falloffPower);
            return false;
#else
            return true; //allow vanilla to do it's own damage calculations ontop of our system.
#endif
        }
        return false; //reject all other types of damage
	}

    bool FindUnprocessedDamageSrc(EntityAI src)
    {
        bool found = (m_dmgSources[src] == 1);
        m_dmgSources.Remove(src); //Remove after find, we don't need this ptr anymore
        return found;
    }

    /*
    * Returns total of damage estimate for a given damage zone
    * based on distance between the sourcePos of damage type and this entity
    * distance is taken into account based on the source ammoType config definitions
    * This contains a slight tiny inaccuracy of total damage due to component positioning
    * in this estimate we use world position rather than pixel perfect component position of the damage zone.
    *
    * !Changing falloffPower to higher value will cause a significant damage drop off based on distance!
    */
    void ApplyEstimateDamage(vector sourcePos, string ammoType, float falloffPower)
    {
        float indirectRange      = g_Game.ConfigGetFloat("CfgAmmo " + ammoType + " indirectHitRange");
        float indirectRangeMulti = g_Game.ConfigGetFloat("CfgAmmo " + ammoType + " indirectHitRangeMultiplier");
        float baseDamageApplied  = g_Game.ConfigGetFloat("CfgAmmo " + ammoType + " DamageApplied Health damage");

        float inner = indirectRange;
        float outer = indirectRange * indirectRangeMulti;
        if (outer <= 0.0)
            return;

        float d = vector.Distance(sourcePos, GetPosition());
        if (d >= outer)
            return;

        // Normalize across whole range 0..outer
        float u = d / outer;
        u = Math.Clamp(u, 0.0, 1.0);

        // Attenuation: 1 at 0m, 0 at outer
        float atten = Math.Pow(1.0 - u, falloffPower);

        BBDS_Print("========");
        foreach(string zone : m_dmg_zones)
        {
            zone.ToLower();

            if (!GetConstruction().IsPartConstructed(zone))
                continue; //ignore hitting parts that aren't constructed.

            #ifdef BBDS_ENABLE_SMART_ZONE
            if (!BaseBuildingDamageSystem.CanDamageZone(this, zone)){
                BBDS_Print("!CANNOT HIT " + zone + " due to rules!");
                continue;
            }
            #endif

            string cfgPath = string.Format("CfgVehicles %1 DamageSystem DamageZones %2 ArmorType FragGrenade Health damage", GetType(), zone);
            float ammoCoefDamage = g_Game.ConfigGetFloat(cfgPath);

            float raw = baseDamageApplied * ammoCoefDamage;
            float damageToApply = raw * atten;

            DecreaseHealth(zone, "Health", damageToApply);
            BBDS_Print(string.Format("zone=%1: [dmg=%2] (raw=%3 atten=%4 d=%5 u=%6 inner=%7 outer=%8)", zone, damageToApply, raw, atten, d, u, inner, outer));
        }
        BBDS_Print("========");
    }
};
#endif