// Port de FreedSpace: lista ordenada de huecos libres de la ROM donde escribir datos nuevos.
#pragma once
#include <list>
#include <string>

namespace pr {

class FreedSpace {
public:
    void free(int start, int length);
    int findAndUnfree(int length);  // -1 si no hay hueco
    int lengthSum() const;
    std::string toString() const;

private:
    struct Chunk {
        int start, end;  // ambos incluidos
    };
    std::list<Chunk> chunks_;
};

}  // namespace pr
