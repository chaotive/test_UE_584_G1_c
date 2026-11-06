#pragma once
#include "Logging/LogMacros.h"

// Identifies diagnostics from the independent training module.
// The exported category has module lifetime and no object ownership.
TESTFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogTestFeatures, Log, All);
