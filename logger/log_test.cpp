#include "mab_logger.h"

using namespace std;

int main() {
    mab::Logger::setLevel(mab::DEBUG);
    mab::Logger log;
    log.trace("Log trace");
    log.debug("Log debug");
    log.info("Log info");
    log.warn("Log warn");
    log.error("log error");
    log.crit("log crit");

    log.setLevel(mab::INFO);
    cout << "Set level to " << log.getLevel() << "\n";

    log.trace("Log trace");
    log.debug("Log debug");
    log.info("Log info");
    log.warn("Log warn");
    log.error("log error");
    log.crit("log crit");

    log.setLevel("wArniNg");
    cout << "Set level to " << log.getLevel() << "\n";

    log.trace("Log trace");
    log.debug("Log debug");
    log.info("Log info");
    log.warn("Log warn");
    log.error("log error");
    log.crit("log crit");


    cout << "DONE" << endl;
    return 0;
}