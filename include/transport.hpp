#pragma once
#include <span>
#include <cstddef>
#include <cstdint>


class Transport {
    public:
    Transport() {};
    template<std::size_t S>
    std::size_t send(std::span<uint8_t, S> buf);

    template<std::size_t S>
    std::size_t recv(std::span<uint8_t, S> buf);
};