#include "FeatureSpec.as"
#include "FeatureBallRulesTests.as"
#include "FeatureLevelRulesTests.as"
#include "FeaturePlayerRulesTests.as"

// Runs the standalone game logic suite and returns its process exit code.
int main()
{
    FeatureSpec Spec;
    Spec.Describe("Ball rules", @FeatureBallRulesTests);
    Spec.Describe("Level rules", @FeatureLevelRulesTests);
    Spec.Describe("Player rules", @FeaturePlayerRulesTests);
    return Spec.Finish();
}
