using UnrealBuildTool;

public class Soullike : ModuleRules
{
    public Soullike(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        
        PublicIncludePaths.Add("Soullike");

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "EnhancedInput",       // Enhanced Input
            "GameplayTags",        // SharedGameplayTags 
            "AIModule",            // AI 
            "NavigationSystem",    // pathfind
            "DeveloperSettings",   
            "UMG",                 // HUD widget
            "MotionWarping",       
            "PhysicsCore",         // CollisionShape for melee sweeps
            "Niagara",          
        });

        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
    }
}
