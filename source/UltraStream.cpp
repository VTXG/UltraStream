#include "UltraStream.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace {
    static void ReverseBuffer(void* buffer, std::size_t size) {
        if (size > 1) {
            std::uint8_t* start = static_cast<std::uint8_t*>(buffer);
            std::uint8_t* end = start + size;
            std::reverse(start, end);
        }
    }
}  // namespace

void UltraStream::ReadEndian(void* buffer, std::size_t size, StreamEndian endian) {
    Read(buffer, size);

    if (endian == StreamEndian::Big) {
        ::ReverseBuffer(buffer, size);
    }
}

void UltraStream::WriteEndian(void* buffer, std::size_t size, StreamEndian endian) {
    if (endian == StreamEndian::Big) {
        ::ReverseBuffer(buffer, size);
    }

    Write(buffer, size);
}

UltraStream::UltraStream(std::size_t size, StreamEndian endian) {
    mBuffer = new std::uint8_t[size];
    mPosition = 0;
    mSize = size;
    mCapacity = size;
    mEndian = endian;
    mIsBufferInternal = true;
}

UltraStream::UltraStream(void* buffer, std::size_t size, StreamEndian endian) {
    mBuffer = static_cast<std::uint8_t*>(buffer);
    mPosition = 0;
    mSize = size;
    mCapacity = size;
    mEndian = endian;
    mIsBufferInternal = false;
}

UltraStream::UltraStream(const std::filesystem::path& path, StreamEndian endian) {
    mPosition = 0;
    mEndian = endian;
    mIsBufferInternal = true;

    std::ifstream fs{path, std::ios::binary};

    fs.seekg(0, fs.end);
    std::size_t size = fs.tellg();

    mBuffer = new std::uint8_t[size];
    mSize = size;
    mCapacity = size;

    fs.seekg(0, fs.beg);
    fs.read(reinterpret_cast<char*>(mBuffer), size);
}

void UltraStream::Read(void* buffer, std::size_t size) {
    if (size == 0) {
        return;
    }

    std::size_t newPosition = mPosition + size;

    if (newPosition > mSize) {
        throw std::range_error("Attempted to read data outside of the stream buffer.");
    }

    std::memcpy(buffer, mBuffer + mPosition, size);
    Seek(newPosition);
}

void UltraStream::Write(const void* data, std::size_t size) {
    if (size == 0) {
        return;
    }

    std::size_t newPosition = mPosition + size;

    if (!EnsureCapacity(newPosition)) {
        throw std::range_error("Cannot expand stream buffer.");
    }

    std::memcpy(mBuffer + mPosition, data, size);
    Seek(newPosition);
}

void UltraStream::Skip(std::int64_t offset) {
    std::int64_t newPosition = mPosition + offset;

    if (newPosition < 0) {
        mPosition = 0;
    } else {
        mPosition = newPosition;
    }
}

bool UltraStream::EnsureCapacity(std::size_t capacity) {
    if (!mIsBufferInternal) {
        return false;
    }

    if (mCapacity >= capacity) {
        return true;
    }

    if (mCapacity * 2 > capacity) {
        mCapacity *= 2;
    } else {
        mCapacity += capacity;
    }

    std::uint8_t* newBuffer = new std::uint8_t[mCapacity];
    std::memcpy(newBuffer, mBuffer, mSize);
    delete[] mBuffer;

    mBuffer = newBuffer;
    mSize = capacity;
    return true;
}