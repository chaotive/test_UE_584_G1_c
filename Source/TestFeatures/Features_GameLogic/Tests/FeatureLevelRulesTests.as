#include "../FeatureLevelRules.as"

// Stands in for the host-registered result enum. Use a different order from C++.
enum SessionResult
{
    Defeat,
    InProgress,
    Victory
}

// Describes level thresholds, session outcomes, and progression.
// The spec handle registers cases with the shared suite.
void FeatureLevelRulesTests(FeatureSpec@ Spec)
{
    Spec.It("uses the base contact target on level one", function(Test)
    {
        Test.ExpectEqual(PlayerContactsToWin(1, 8, 2), 8);
    });

    Spec.It("raises the contact target by the configured amount per level", function(Test)
    {
        Test.ExpectEqual(PlayerContactsToWin(4, 8, 2), 14);
    });

    Spec.It("uses the base bounce limit on level one", function(Test)
    {
        Test.ExpectEqual(SessionBouncesToLose(1, 20, 3, 8), 20);
    });

    Spec.It("reduces the bounce limit by the configured amount per level", function(Test)
    {
        Test.ExpectEqual(SessionBouncesToLose(3, 20, 3, 8), 14);
    });

    Spec.It("floors the bounce limit at the configured minimum", function(Test)
    {
        Test.ExpectEqual(SessionBouncesToLose(8, 20, 3, 8), 8);
    });

    Spec.It("keeps the session in progress below both limits", function(Test)
    {
        Test.ExpectEqual(SessionOutcome(7, 19, 8, 20), SessionResult::InProgress);
    });

    Spec.It("awards victory at the contact limit", function(Test)
    {
        Test.ExpectEqual(SessionOutcome(8, 19, 8, 20), SessionResult::Victory);
    });

    Spec.It("ends in defeat at the bounce limit", function(Test)
    {
        Test.ExpectEqual(SessionOutcome(7, 20, 8, 20), SessionResult::Defeat);
    });

    Spec.It("gives victory precedence when both limits are reached", function(Test)
    {
        Test.ExpectEqual(SessionOutcome(8, 20, 8, 20), SessionResult::Victory);
    });

    Spec.It("advances one level after victory", function(Test)
    {
        Test.ExpectEqual(NextLevel(4, SessionResult::Victory), 5);
    });

    Spec.It("retains the level after defeat or during an unfinished session", function(Test)
    {
        Test.ExpectEqual(NextLevel(4, SessionResult::Defeat), 4);
        Test.ExpectEqual(NextLevel(4, SessionResult::InProgress), 4);
    });
}
