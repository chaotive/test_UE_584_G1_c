#include "../FeatureBallRules.as"

// Stands in for the host-registered action enum. Use different flag bits from C++.
enum BallContactAction
{
    None = 0,
    CountBounce = 8,
    Retire = 32
}

// Describes contact classification, retirement, and replacement.
// The spec handle registers cases with the shared suite.
void FeatureBallRulesTests(FeatureSpec@ Spec)
{
    Spec.It("retires a ball on player contact without counting a bounce", function(Test)
    {
        Test.ExpectEqual(EvaluateBallContact(true, false, 4, 5), BallContactAction::Retire);
        Test.ExpectEqual(EvaluateBallContact(true, true, 4, 5), BallContactAction::Retire);
    });

    Spec.It("ignores non-player contact without solver impulse", function(Test)
    {
        Test.ExpectEqual(EvaluateBallContact(false, false, 4, 5), BallContactAction::None);
    });

    Spec.It("counts a bounce below the rebound limit and keeps the ball active", function(Test)
    {
        Test.ExpectEqual(EvaluateBallContact(false, true, 3, 5), BallContactAction::CountBounce);
    });

    Spec.It("counts the bounce that reaches the rebound limit and retires the ball", function(Test)
    {
        Test.ExpectEqual(EvaluateBallContact(false, true, 4, 5),
            BallContactAction::CountBounce | BallContactAction::Retire);
    });

    Spec.It("allows replacement during an active session", function(Test)
    {
        Test.ExpectEqual(ShouldReplaceBall(true), true);
    });

    Spec.It("disallows replacement outside an active session", function(Test)
    {
        Test.ExpectEqual(ShouldReplaceBall(false), false);
    });
}
