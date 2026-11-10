// Gives each group or case a handle to the active spec for reporting results.
funcdef void FeatureSpecBody(FeatureSpec@ Spec);

// Runs named cases and reports their assertion failures to the asrun console.
class FeatureSpec
{
    // Counts the cases that ran.
    private int CaseCount = 0;

    // Counts the cases with at least one failure.
    private int FailureCount = 0;

    // Collects assertion and exception messages for the active case.
    private string FailureDetails;

    // Groups related cases under a heading.
    // The name is read-only and the function handle runs synchronously.
    void Describe(const string &in Name, FeatureSpecBody@ Body)
    {
        print("\n" + Name + "\n");
        Body(this);
    }

    // Runs one named case and reports its result before continuing.
    // The name is read-only and the function handle runs synchronously.
    void It(const string &in Name, FeatureSpecBody@ Body)
    {
        ++CaseCount;
        FailureDetails = "";
        try
        {
            Body(this);
        }
        catch
        {
            FailureDetails += "    " + getExceptionInfo() + "\n";
        }

        if (FailureDetails == "")
        {
            print("  PASS " + Name + "\n");
            return;
        }

        ++FailureCount;
        print("  FAIL " + Name + "\n" + FailureDetails);
    }

    // Checks an exact integer or enum value.
    void ExpectEqual(int Actual, int Expected)
    {
        if (Actual != Expected)
        {
            FailureDetails += "    expected " + Expected + ", got " + Actual + "\n";
        }
    }

    // Checks the boolean result of a rule.
    void ExpectEqual(bool Actual, bool Expected)
    {
        if (Actual != Expected)
        {
            FailureDetails += "    expected " + Expected + ", got " + Actual + "\n";
        }
    }

    // Checks a floating-point result within the default tolerance of 0.001.
    void ExpectNear(float Actual, float Expected, float Tolerance = 0.001f)
    {
        if (!(Actual >= Expected - Tolerance && Actual <= Expected + Tolerance))
        {
            FailureDetails += "    expected " + Expected + " (within " + Tolerance + "), got " + Actual + "\n";
        }
    }

    // Prints the total and returns the process exit code.
    int Finish()
    {
        print("\n" + (CaseCount - FailureCount) + " passed, " + FailureCount + " failed\n");
        return FailureCount == 0 ? 0 : 1;
    }
}
