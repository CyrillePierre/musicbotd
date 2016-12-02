#include <iostream>
#include <log/log.hpp>

void test() {
    log::Logger l;
    l.prefix("test(): ");
    l << "fromage";
    l << "champignon";
}

int main() {
//    log::setStream(std::cout);
//    log::format() << log::item::time << log::item::msg;

    log::cfg().timeEnabled(true);
//    log::cfg().logLevel(log::msg);

    log::Logger logger;
    logger.prefix("main(): ");
    logger(log::dbg) << "cheval " << "mouton " << 8 << ' ' << 4.3;
    logger(log::msg) << "les brouettes c'est pratique";
    logger(log::warn) << "la maison n°" << 27 << " est bleu";
    test();
    logger(log::err) << "niveau important de message";
    logger(log::crit) << "attention ça va exploser";
    logger << "et comme ça ?";
}
