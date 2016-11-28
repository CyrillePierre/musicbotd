#pragma once

#include "config.hpp"

class WebMusic {
    std::string _id;
    std::string _title;

public:
    WebMusic(std::string const & id, std::string const & title)
        : _id{id}, _title{title} {}

    std::string const & id() const { return _id; }
    std::string const & title() const { return _title; }
    std::string url() const { return cfg::ytUrlPrefix + _id; }
    bool operator == (WebMusic const & wm) { return _id == wm._id; }
};
