// Implementación de la plantilla IniEntryReader (incluida desde ini_entry.hpp).
#include <cstdio>

#include "util/java_compat.hpp"

namespace pr {

template <class T>
std::vector<std::unique_ptr<T>> IniEntryReader<T>::read(std::string_view text, const std::string& fileName) {
    std::vector<std::unique_ptr<T>> entries;
    entries_ = &entries;
    fileName_ = fileName;
    T* current = nullptr;

    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        std::string line(text.substr(pos, nl == std::string_view::npos ? std::string_view::npos : nl - pos));
        pos = (nl == std::string_view::npos) ? text.size() : nl + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // removeComments(): solo recorta si hay comentario, como en Java
        size_t c = line.find("//");
        if (c != std::string::npos) line = jc::trim(std::string_view(line).substr(0, c));
        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']') {
            entries.push_back(factory_(line.substr(1, line.size() - 2)));
            current = entries.back().get();
        } else if (current != nullptr) {
            parseLine(*current, line);
        }
    }
    entries_ = nullptr;
    return entries;
}

template <class T>
void IniEntryReader<T>::parseLine(T& current, const std::string& line) {
    auto pair = jc::split(line, "=", 2);
    if (pair.size() == 1) {
        std::fprintf(stderr, "invalid line in %s/%s: %s\n", fileName_.c_str(), current.name().c_str(), line.c_str());
        return;
    }
    std::string key = jc::trim(pair[0]);
    std::string value = jc::trim(pair[1]);

    if (key == "CopyFrom") {
        // Java recorre todas las entradas leídas hasta ahora; copiarse a sí misma no cambia nada
        for (auto& other : *entries_) {
            if (other.get() != &current && matchesCopyFrom(*other, value)) current.copyFrom(*other);
        }
        return;
    }
    if (auto it = special_.find(key); it != special_.end()) {
        it->second(current, value);
        return;
    }
    for (auto& [p, fn] : prefix_) {
        if (jc::startsWith(key, p)) { fn(current, key, value); return; }
    }
    for (auto& [s, fn] : suffix_) {
        if (jc::endsWith(key, s)) { fn(current, key, value); return; }
    }
    if (jc::startsWith(value, "[") && jc::endsWith(value, "]")) {
        current.putArray(key, ini::parseArray(value));
    } else if (mode_ == DefaultReadMode::INT) {
        current.putInt(key, ini::parseInt(value));
    } else {
        current.putString(key, value);
    }
}

template <class T>
bool IniEntryReader<T>::matchesCopyFrom(const T& other, const std::string& value) const {
    if (copyMode_ == CopyFromMode::ROMCODE) return value == other.romCode();
    return jc::equalsIgnoreCase(value, other.name());
}

}  // namespace pr
