# CustomBaseDamage

## What is this?
Fully server side modification that expands on how the vanilla damage system works against Base Building items (Fences, gates, watchtowers etc)
Supports types that inherit from `BaseBuildingBase` vanilla class

This mod addresses the inconsistency with how vanilla raiding works, explosives have odd penetration & splash damage behavior when it comes to base building objects. This mod introduces a more "aggressive style" of damage delivery to base building object.

## How it works:
Specific to explosives, this mod *piggy-backs* onto the vanilla damage system to verify calculations of how explosive damage is applied. 
By using vanilla `config.cpp` defined damage values (Grenades, 40mm explosive, Plastic Explosive, Claymores, Improvised Explosives of all types)

## Scenario:
Base walls placed in close proximity will take appropriate amount of damage based on radius, damage, armor of object hit of used explosive, the vanilla system of damage is applied first, as a "backup / to verify" this mod will do calculations to ensure damage is consistently delivered as to what is configured.

```diff
+ Features:
```
- Supports all types of vanilla base building 
- Supports all types of custom/modded explosives (as long as it inherited `ExplosiveBase`)
- Damage & Health variables pulled from definitions of config.cpp damage system
- Script configuration to tweak behavior of how damage is applied
- By default, shooting an explosive placed on the ground is `DISABLED` (changeable with a script definition) this is to avoid "nade nukes" and slow down the process of raiding, requiring the right tools i.e; Improvised explosive, launchers etc
- Debug logging (disabled by default) to track how damage is delivered

```diff
- Compatibility:
```
The mod supports any type that extends from `BaseBuildingBase` if any base building mod on the workshop has it's types created off of the vanilla system, it is automatically supported.


**Tweaking parameters:**
This mod *avoids* using dynamic configurations (i.e JSON / txt)
Value configuration is fully supported from config.cpp definition of the vanilla damage system. 
Ontop of that, `BaseBuildingDamageSystem.c` contains parameters that you can modify to control overall damage behavior

```ini
[ Repack / Usage ]
```
>>>Server side only mod, feel free to repack, modify, redistribute and or rework. Credit author DaOne. Mod requested by @3RNO (DAYZ SALVATION)
