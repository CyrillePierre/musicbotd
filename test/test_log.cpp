#include <iostream>
#include <log/log.hpp>

void test() {
    log::Logger l;
    l.prefix("test(): ");
    l(log::trace) << "fromage";
    l(log::warn) << "champignon";
}

int main() {
    log::cfg().timeEnabled(true);
    log::cfg().timeFormat("%T");
    log::cfg().logLevel(log::dbg);
//    log::cfg().stream("test.log");

    log::Logger logger;
    logger.prefix("main(): ");
    logger(log::trace) << "niveau le plus bas";
    logger(log::dbg) << "cheval " << "mouton " << 8 << ' ' << 4.3;
    logger(log::msg) << "les brouettes c'est pratique";
    logger(log::warn) << "la maison n°" << 27 << " est bleu";
    logger(log::err) << "niveau important de message";
    logger(log::crit) << "attention ça va exploser";

    test();
    logger << "et comme ça ?";
    log::Logger{} << "sans prefixe";
    log::Logger{}(log::crit) << "sans prefixe";
}
