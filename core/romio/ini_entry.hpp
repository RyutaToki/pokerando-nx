// Port de IniEntry / IniEntryReader / RomEntry de FVX.
// Lee los archivos genN_offsets.ini con las mismas reglas que el original.
#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pr {

class IniEntry {
public:
    explicit IniEntry(std::string name) : name_(std::move(name)) {}
    virtual ~IniEntry() = default;

    const std::string& name() const { return name_; }

    // Igual que en Java: pedir una clave inexistente devuelve 0 / "" / [] (y la crea).
    int getInt(const std::string& key);
    int getInt(const std::string& key) const;
    void putInt(const std::string& key, int value) { ints_[key] = value; }
    const std::string& getString(const std::string& key);
    const std::string& getString(const std::string& key) const;  // "" si no existe
    void putString(const std::string& key, std::string value) { strings_[key] = std::move(value); }
    const std::vector<int>& getArray(const std::string& key);
    const std::vector<int>& getArray(const std::string& key) const;  // [] si no existe
    void putArray(const std::string& key, std::vector<int> value) { arrays_[key] = std::move(value); }
    bool hasInt(const std::string& key) const { return ints_.count(key) > 0; }

    virtual void copyFrom(const IniEntry& other);

protected:
    std::string name_;
    std::unordered_map<std::string, int> ints_;
    std::unordered_map<std::string, std::string> strings_;
    std::unordered_map<std::string, std::vector<int>> arrays_;
};

class RomEntry : public IniEntry {
public:
    using IniEntry::IniEntry;

    const std::string& romCode() const { return romCode_; }
    int version() const { return version_; }
    int romType() const { return romType_; }
    void setRomCode(std::string c) { romCode_ = std::move(c); }
    void setVersion(int v) { version_ = v; }
    void setRomType(int t) { romType_ = t; }
    virtual void setRomTypeFromString(const std::string& s) = 0;

    void putTweakFile(const std::string& key, std::string value) { tweakFiles_[key] = std::move(value); }
    bool hasTweakFile(const std::string& key) const { return tweakFiles_.count(key) > 0; }
    std::string getTweakFile(const std::string& key) const;

protected:
    void copyRomEntryFields(const RomEntry& o);

    std::string romCode_;
    int version_ = 0;
    int romType_ = 0;
    std::unordered_map<std::string, std::string> tweakFiles_;
};

namespace ini {
int parseInt(std::string_view s);
int64_t parseLong(std::string_view s);
bool parseBoolean(std::string_view s);
std::vector<int> parseArray(std::string_view s);
}  // namespace ini

// Lector genérico. T debe derivar de IniEntry.
template <class T>
class IniEntryReader {
public:
    enum class DefaultReadMode { INT, STRING };
    enum class CopyFromMode { NAME, ROMCODE };
    using ValuePairFn = std::function<void(T&, const std::string& key, const std::string& value)>;
    using SpecialFn = std::function<void(T&, const std::string& value)>;
    using Factory = std::function<std::unique_ptr<T>(const std::string& name)>;

    IniEntryReader(DefaultReadMode mode, CopyFromMode copyMode, Factory factory)
        : mode_(mode), copyMode_(copyMode), factory_(std::move(factory)) {}

    void putSpecialKeyMethod(const std::string& key, SpecialFn fn) { special_[key] = std::move(fn); }
    void putKeyPrefixMethod(const std::string& key, ValuePairFn fn) { prefix_.emplace_back(key, std::move(fn)); }
    void putKeySuffixMethod(const std::string& key, ValuePairFn fn) { suffix_.emplace_back(key, std::move(fn)); }

    std::vector<std::unique_ptr<T>> read(std::string_view text, const std::string& fileName);

private:
    void parseLine(T& current, const std::string& line);
    bool matchesCopyFrom(const T& other, const std::string& value) const;

    DefaultReadMode mode_;
    CopyFromMode copyMode_;
    Factory factory_;
    std::map<std::string, SpecialFn> special_;
    std::vector<std::pair<std::string, ValuePairFn>> prefix_;
    std::vector<std::pair<std::string, ValuePairFn>> suffix_;
    std::vector<std::unique_ptr<T>>* entries_ = nullptr;
    std::string fileName_;
};

}  // namespace pr

#include "romio/ini_entry_reader.inl"
