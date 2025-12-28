class CfgPatches
{
	class 0_CustomBaseDamage
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			
		};
	};
};
class CfgMods
{
	class CustomBaseDamage
	{
		dir="CustomBaseDamage";
		hideName=0;
		hidePicture=0;
		name="Custom Base Building Damage System";
		credits="DaOne";
		author="DaOne";
		authorID="420420";
		version="Version 1.0";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"CustomBaseDamage/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"CustomBaseDamage/4_World"
				};
			};
		};
	};
};