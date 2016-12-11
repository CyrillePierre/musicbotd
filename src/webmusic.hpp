#pragma once

#include "config.hpp"

class WebMusic {
    std::string _id;
    std::string _title;

public:
    WebMusic() {}
    WebMusic(WebMusic &&) = default;
    WebMusic(WebMusic const &) = default;
    WebMusic(std::string const & id, std::string const & title)
        : _id{id}, _title{title} {}

    WebMusic & operator = (WebMusic const & wm) {
        if (this != &wm) {
            _id = wm._id;
            _title = wm._title;
        }
        return *this;
    }

    WebMusic & operator = (WebMusic && wm) {
        if (this != &wm) {
            std::swap(_id, wm._id);
            std::swap(_title, wm._title);
        }
        return *this;
    }

    std::string const & id() const { return _id; }
    std::string const & title() const { return _title; }
    std::string url() const { return cfg::ytUrlPrefix + _id; }
    bool operator == (WebMusic const & wm) { return _id == wm._id; }
};
