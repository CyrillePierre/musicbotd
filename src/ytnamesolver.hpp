#pragma once

struct YtNameSolver {
    static void init();
    static void dispose();
    static std::string getName(std::string url);
};
