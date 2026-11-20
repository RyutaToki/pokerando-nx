// Réplica del comportamiento de java.util.HashMap / HashSet (JDK 25) en lo que afecta al
// ORDEN DE ITERACIÓN: tabla de cubetas, crecimiento, división al redimensionar y orden de
// inserción dentro de cada cubeta. FVX recorre conjuntos de Pokémon (SpeciesSet extiende
// HashSet) mientras usa el generador aleatorio, así que el orden tiene que ser idéntico.
//
// Limitación conocida: los "tree bins" de Java (8+ claves en una cubeta con tabla >= 64) se
// tratan como listas; si ocurre se avisa por stderr para poder revisarlo.
#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <utility>
#include <vector>

namespace pr {

// hashCode() de Java para cada tipo de clave. Especializar según haga falta.
template <class K>
struct JavaHashCode;

template <>
struct JavaHashCode<int> {
    int operator()(int v) const { return v; }  // Integer.hashCode()
};

namespace detail {
inline int tableSizeFor(int cap) {
    if (cap <= 1) return 1;
    unsigned n = static_cast<unsigned>(cap - 1);
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
    return static_cast<int>(n) + 1 >= (1 << 30) ? (1 << 30) : static_cast<int>(n) + 1;
}
inline int spread(int h) { return h ^ static_cast<int>(static_cast<uint32_t>(h) >> 16); }
}  // namespace detail

template <class K, class V, class H = JavaHashCode<K>, class Eq = std::equal_to<K>>
class JavaHashMap {
    struct Node {
        int hash;
        K key;
        V value;
        Node* next;
    };

public:
    JavaHashMap() = default;
    // new HashMap<>(initialCapacity)
    explicit JavaHashMap(int initialCapacity) { threshold_ = detail::tableSizeFor(initialCapacity); }
    // HashMap.newHashMap(numMappings)
    static JavaHashMap newHashMap(int numMappings) {
        return JavaHashMap(static_cast<int>(std::ceil(numMappings / 0.75)));
    }
    // clone(): misma estructura interna
    JavaHashMap(const JavaHashMap& o) { cloneFrom(o); }
    JavaHashMap& operator=(const JavaHashMap& o) {
        if (this != &o) { destroy(); cloneFrom(o); }
        return *this;
    }
    JavaHashMap(JavaHashMap&& o) noexcept { swap(o); }
    JavaHashMap& operator=(JavaHashMap&& o) noexcept {
        if (this != &o) { destroy(); swap(o); }
        return *this;
    }
    ~JavaHashMap() { destroy(); }

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    int capacity() const { return static_cast<int>(table_.size()); }

    V* get(const K& key) {
        Node* n = find(key);
        return n ? &n->value : nullptr;
    }
    const V* get(const K& key) const {
        Node* n = const_cast<JavaHashMap*>(this)->find(key);
        return n ? &n->value : nullptr;
    }
    bool containsKey(const K& key) const { return get(key) != nullptr; }

    // put(): devuelve true si la clave era nueva
    bool put(const K& key, V value) { return putVal(key, std::move(value), false); }
    bool putIfAbsent(const K& key, V value) { return putVal(key, std::move(value), true); }
    // computeIfAbsent simplificado: inserta V() si falta y devuelve referencia
    V& getOrInsert(const K& key) {
        if (V* v = get(key)) return *v;
        putVal(key, V{}, false);
        return *get(key);
    }
    // putAll(m): putMapEntries
    void putAll(const JavaHashMap& m) {
        int s = m.size();
        if (s > 0) {
            if (table_.empty()) {
                double dt = std::ceil(s / 0.75);
                int t = static_cast<int>(dt);
                if (t > threshold_) threshold_ = detail::tableSizeFor(t);
            } else {
                while (s > threshold_ && capacity() < (1 << 30)) resize();
            }
            m.forEach([&](const K& k, const V& v) { putVal(k, v, false); });
        }
    }

    bool remove(const K& key) {
        if (table_.empty()) return false;
        int h = hashOf(key);
        size_t i = static_cast<size_t>(h) & (table_.size() - 1);
        Node* prev = nullptr;
        for (Node* n = table_[i]; n; prev = n, n = n->next) {
            if (n->hash == h && eq_(n->key, key)) {
                (prev ? prev->next : table_[i]) = n->next;
                delete n;
                size_--;
                return true;
            }
        }
        return false;
    }

    // clear(): Java conserva el tamaño de la tabla
    void clear() {
        for (auto& b : table_) {
            for (Node* n = b; n;) { Node* nx = n->next; delete n; n = nx; }
            b = nullptr;
        }
        size_ = 0;
    }

    template <class F>
    void forEach(F&& f) const {
        for (Node* b : table_)
            for (Node* n = b; n; n = n->next) f(n->key, n->value);
    }
    template <class F>
    void forEachMutable(F&& f) {
        for (Node* b : table_)
            for (Node* n = b; n; n = n->next) f(n->key, n->value);
    }
    // removeIf sobre las entradas (orden de iteración de Java)
    template <class P>
    bool removeIf(P&& pred) {
        bool removed = false;
        for (auto& b : table_) {
            Node* prev = nullptr;
            for (Node* n = b; n;) {
                Node* nx = n->next;
                if (pred(n->key, n->value)) {
                    (prev ? prev->next : b) = nx;
                    delete n;
                    size_--;
                    removed = true;
                } else {
                    prev = n;
                }
                n = nx;
            }
        }
        return removed;
    }
    std::vector<K> keys() const {
        std::vector<K> r;
        r.reserve(size_);
        forEach([&](const K& k, const V&) { r.push_back(k); });
        return r;
    }

private:
    int hashOf(const K& key) const { return detail::spread(h_(key)); }

    Node* find(const K& key) {
        if (table_.empty()) return nullptr;
        int h = hashOf(key);
        for (Node* n = table_[static_cast<size_t>(h) & (table_.size() - 1)]; n; n = n->next) {
            if (n->hash == h && eq_(n->key, key)) return n;
        }
        return nullptr;
    }

    bool putVal(const K& key, V value, bool onlyIfAbsent) {
        if (table_.empty()) resize();
        int h = hashOf(key);
        size_t n = table_.size();
        size_t i = static_cast<size_t>(h) & (n - 1);
        Node* p = table_[i];
        if (!p) {
            table_[i] = new Node{h, key, std::move(value), nullptr};
        } else {
            if (p->hash == h && eq_(p->key, key)) {
                if (!onlyIfAbsent) p->value = std::move(value);
                return false;
            }
            for (int binCount = 0;; ++binCount) {
                Node* e = p->next;
                if (!e) {
                    p->next = new Node{h, key, std::move(value), nullptr};
                    if (binCount >= 8 - 1) treeifyBin();
                    break;
                }
                if (e->hash == h && eq_(e->key, key)) {
                    if (!onlyIfAbsent) e->value = std::move(value);
                    return false;
                }
                p = e;
            }
        }
        if (++size_ > threshold_) resize();
        return true;
    }

    void treeifyBin() {
        if (table_.size() < 64) {
            resize();
        } else {
            std::fprintf(stderr, "[java_hash] AVISO: cubeta con 8+ claves (tree bin de Java no emulado)\n");
        }
    }

    void resize() {
        int oldCap = static_cast<int>(table_.size());
        int oldThr = threshold_;
        int newCap, newThr = 0;
        if (oldCap > 0) {
            if (oldCap >= (1 << 30)) { threshold_ = 0x7FFFFFFF; return; }
            newCap = oldCap << 1;
            if (newCap < (1 << 30) && oldCap >= 16) newThr = oldThr << 1;
        } else if (oldThr > 0) {
            newCap = oldThr;
        } else {
            newCap = 16;
            newThr = 12;
        }
        if (newThr == 0) {
            float ft = static_cast<float>(newCap) * 0.75f;
            newThr = (newCap < (1 << 30) && ft < static_cast<float>(1 << 30)) ? static_cast<int>(ft) : 0x7FFFFFFF;
        }
        threshold_ = newThr;
        std::vector<Node*> newTab(static_cast<size_t>(newCap), nullptr);
        for (int j = 0; j < oldCap; ++j) {
            Node* e = table_[j];
            if (!e) continue;
            if (!e->next) {
                newTab[static_cast<size_t>(e->hash) & (newCap - 1)] = e;
                continue;
            }
            Node *loH = nullptr, *loT = nullptr, *hiH = nullptr, *hiT = nullptr;
            while (e) {
                Node* nx = e->next;
                if ((e->hash & oldCap) == 0) {
                    (loT ? loT->next : loH) = e;
                    loT = e;
                } else {
                    (hiT ? hiT->next : hiH) = e;
                    hiT = e;
                }
                e = nx;
            }
            if (loT) { loT->next = nullptr; newTab[j] = loH; }
            if (hiT) { hiT->next = nullptr; newTab[j + oldCap] = hiH; }
        }
        table_ = std::move(newTab);
    }

    void destroy() {
        clear();
        table_.clear();
    }
    void cloneFrom(const JavaHashMap& o) {
        threshold_ = o.threshold_;
        size_ = o.size_;
        table_.assign(o.table_.size(), nullptr);
        for (size_t i = 0; i < o.table_.size(); i++) {
            Node** tail = &table_[i];
            for (Node* n = o.table_[i]; n; n = n->next) {
                *tail = new Node{n->hash, n->key, n->value, nullptr};
                tail = &(*tail)->next;
            }
        }
    }
    void swap(JavaHashMap& o) noexcept {
        std::swap(table_, o.table_);
        std::swap(size_, o.size_);
        std::swap(threshold_, o.threshold_);
    }

    std::vector<Node*> table_;
    int size_ = 0;
    int threshold_ = 0;
    H h_;
    Eq eq_;
};

struct JavaEmpty {};

template <class K, class H = JavaHashCode<K>, class Eq = std::equal_to<K>>
class JavaHashSet {
public:
    JavaHashSet() = default;
    explicit JavaHashSet(int initialCapacity) : map_(initialCapacity) {}
    // HashMap.newHashMap(numMappings) como mapa interno
    static JavaHashSet withMappings(int numMappings) {
        JavaHashSet s;
        s.map_ = JavaHashMap<K, JavaEmpty, H, Eq>::newHashMap(numMappings);
        return s;
    }
    // new HashSet<>(collection)
    template <class Range>
    static JavaHashSet fromCollection(const Range& c) {
        int n = static_cast<int>(std::size(c));
        JavaHashSet s = withMappings(n > 12 ? n : 12);
        for (const auto& e : c) s.add(e);
        return s;
    }

    bool add(const K& k) { return map_.put(k, JavaEmpty{}); }
    bool remove(const K& k) { return map_.remove(k); }
    bool contains(const K& k) const { return map_.containsKey(k); }
    int size() const { return map_.size(); }
    bool empty() const { return map_.empty(); }
    void clear() { map_.clear(); }
    template <class Range>
    bool addAll(const Range& c) {
        bool changed = false;
        for (const auto& e : c) changed |= add(e);
        return changed;
    }
    template <class P>
    bool removeIf(P&& pred) {
        return map_.removeIf([&](const K& k, const JavaEmpty&) { return pred(k); });
    }
    template <class F>
    void forEach(F&& f) const {
        map_.forEach([&](const K& k, const JavaEmpty&) { f(k); });
    }
    // copia de los elementos en orden de iteración de Java
    std::vector<K> toVector() const { return map_.keys(); }

private:
    JavaHashMap<K, JavaEmpty, H, Eq> map_;
};

}  // namespace pr
