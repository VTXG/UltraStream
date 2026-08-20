#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

enum class StreamEndian {
    Big,
    Little,
};

class UltraStream {
private:
    std::uint8_t* mBuffer;
    std::size_t mPosition;
    std::size_t mSize;
    std::size_t mCapacity;
    StreamEndian mEndian;
    bool mIsBufferInternal;

    void ReadEndian(void* buffer, std::size_t size, StreamEndian endian);
    void WriteEndian(void* buffer, std::size_t size, StreamEndian endian);

public:
    constexpr auto GetPosition() const { return mPosition; }
    constexpr auto GetSize() const { return mSize; }
    constexpr auto GetCapacity() const { return mCapacity; }
    constexpr auto GetEndian() const { return mEndian; }
    constexpr void SetEndian(StreamEndian v) { mEndian = v; }

    UltraStream(std::size_t size, StreamEndian endian);
    UltraStream(void* buffer, std::size_t size, StreamEndian endian);
    UltraStream(const std::filesystem::path& path, StreamEndian endian);

    UltraStream(const UltraStream&) = delete;
    UltraStream(UltraStream&& other)
        : mBuffer(other.mBuffer), mPosition(other.mPosition), mSize(other.mSize), mCapacity(other.mCapacity), mEndian(other.mEndian),
          mIsBufferInternal(other.mIsBufferInternal) {
        other.mBuffer = nullptr;
        other.mPosition = 0;
        other.mSize = 0;
        other.mCapacity = 0;
        other.mEndian = StreamEndian::Little;
        other.mIsBufferInternal = false;
    }

    UltraStream& operator=(const UltraStream&) = delete;
    UltraStream& operator=(UltraStream&& other) {
        if (this != &other) {
            mBuffer = other.mBuffer;
            mPosition = other.mPosition;
            mSize = other.mSize;
            mCapacity = other.mCapacity;
            mEndian = other.mEndian;
            mIsBufferInternal = other.mIsBufferInternal;
            other.mBuffer = nullptr;
            other.mPosition = 0;
            other.mSize = 0;
            other.mCapacity = 0;
            other.mEndian = StreamEndian::Little;
            other.mIsBufferInternal = false;
        }

        return *this;
    }

    ~UltraStream() {
        if (mIsBufferInternal && mBuffer != nullptr) {
            delete[] mBuffer;
        }
    }

    void Read(void* buffer, std::size_t size);
    void Write(const void* data, std::size_t size);
    void Seek(std::size_t offset) { mPosition = offset; }
    void Skip(std::int64_t offset);
    bool EnsureCapacity(std::size_t capacity);

    template <typename T>
    void ReadValue(T& out, StreamEndian endian) {
        ReadEndian(static_cast<void*>(&out), sizeof(T), endian);
    }

    template <typename T>
    void ReadValue(T& out) {
        ReadValue(out, mEndian);
    }

    template <typename T>
    T ReadValue() {
        T out;
        ReadValue(out, mEndian);
        return out;
    }

    template <typename T>
    void PeekValue(T& out, StreamEndian endian) {
        ReadEndian(static_cast<void*>(&out), sizeof(T), endian);
        Skip(-sizeof(T));
    }

    template <typename T>
    void PeekValue(T& out) {
        PeekValue(out, mEndian);
    }

    template <typename T>
    std::basic_string<T> ReadString(T nullChar = '\0') {
        std::basic_string<T> s{};
        T c;

        while (true) {
            ReadValue(c);

            if (c == nullChar) {
                break;
            }

            s.push_back(c);
        }

        return s;
    }

    template <typename T>
    std::basic_string<T> ReadString(std::size_t maxLength, T nullChar = '\0') {
        auto startPostion = mPosition;

        std::basic_string<T> s{};
        T c;

        while (s.length() < maxLength) {
            ReadValue(c);

            if (c == nullChar) {
                break;
            }

            s.push_back(c);
        }

        Seek(startPostion + (maxLength * sizeof(T)));
        return s;
    }

    template <typename T>
    void WriteValue(const T& data, StreamEndian endian) {
        T buffer = data;
        WriteEndian(static_cast<void*>(&buffer), sizeof(T), endian);
    }

    template <typename T>
    void WriteValue(const T& data) {
        WriteValue(data, mEndian);
    }

    template <typename T>
    void WriteString(const std::basic_string<T>& s, T nullChar = '\0') {
        std::size_t maxSize = s.length() * sizeof(T);
        auto data = s.c_str();

        EnsureCapacity(mPosition + maxSize);

        for (std::size_t i = 0; i < s.length(); i++) {
            WriteValue(data[i]);
        }

        WriteValue(nullChar);
    }

    template <typename T>
    void WriteString(const std::basic_string<T>& s, std::size_t maxLength, T nullChar = '\0') {
        std::size_t maxSize = maxLength * sizeof(T);
        std::size_t charLenth = std::min(s.length(), maxLength);
        std::size_t padLength = maxLength - charLenth;
        auto data = s.c_str();

        EnsureCapacity(mPosition + maxSize);

        for (std::size_t i = 0; i < charLenth; i++) {
            WriteValue(data[i]);
        }

        for (std::size_t i = 0; i < padLength; i++) {
            WriteValue(nullChar);
        }
    }
};

class StreamSeek {
private:
    UltraStream& mStream;
    std::size_t mPosition;

public:
    StreamSeek(UltraStream& us, std::size_t position) : mStream(us) {
        mPosition = us.GetPosition();
        us.Seek(position);
    }

    StreamSeek(const StreamSeek&) = delete;
    StreamSeek(const StreamSeek&&) = delete;

    StreamSeek& operator=(const StreamSeek&) = delete;
    StreamSeek& operator=(const StreamSeek&&) = delete;

    ~StreamSeek() { mStream.Seek(mPosition); }
};