#include <iostream>
#include "src/core/types.h"
#include "src/core/logic.h"
#include "src/core/property_test.h"
#include "src/shell/logger.h"
#include "src/shell/rng.h"

int main() {
    std::cout << "Starting test..." << std::endl;
    
    Logger logger;
    logger.init(false, false, "", false);
    
    Rng rng = makeRng(42);
    
    std::cout << "Running property tests..." << std::endl;
    bool passed = core::test::runPropertyTests(&logger, rng);
    
    std::cout << "Result: " << (passed ? "PASS" : "FAIL") << std::endl;
    return passed ? 0 : 1;
}
