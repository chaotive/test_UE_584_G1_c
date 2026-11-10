#include "../FeaturePlayerRules.as"

// Describes the player contact bonus and walking speed rules.
// The spec handle registers cases with the shared suite.
void FeaturePlayerRulesTests(FeatureSpec@ Spec)
{
    Spec.It("clears existing bonuses when the boost is disabled", function(Test)
    {
        Test.ExpectEqual(PlayerSpeedBonusCount(2, true, 0.0f), 0);
    });

    Spec.It("starts and refreshes one bonus when stacking is disabled", function(Test)
    {
        const int FirstBonus = PlayerSpeedBonusCount(0, false, 5.0f);
        Test.ExpectEqual(FirstBonus, 1);
        Test.ExpectEqual(PlayerSpeedBonusCount(FirstBonus, false, 5.0f), 1);
    });

    Spec.It("adds one bonus on contact when stacking is enabled", function(Test)
    {
        Test.ExpectEqual(PlayerSpeedBonusCount(2, true, 5.0f), 3);
    });

    Spec.It("uses the configured baseline speed without bonuses", function(Test)
    {
        Test.ExpectNear(PlayerWalkSpeed(500.0f, 0.6f, 1.5f, 0), 300.0f);
    });

    Spec.It("increases walking speed with two bonuses", function(Test)
    {
        Test.ExpectNear(PlayerWalkSpeed(500.0f, 0.6f, 1.5f, 2), 600.0f);
    });

    Spec.It("floors walking speed at zero for stacked slowdowns", function(Test)
    {
        Test.ExpectNear(PlayerWalkSpeed(500.0f, 0.6f, 0.25f, 2), 0.0f);
    });
}
