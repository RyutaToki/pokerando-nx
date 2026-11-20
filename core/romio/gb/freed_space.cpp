#include "romio/gb/freed_space.hpp"

#include <cstdio>
#include <stdexcept>

namespace pr {

namespace {
[[noreturn]] void alreadyFreed(int s, int e, int ns, int ne) {
    char buf[200];
    std::snprintf(buf, sizeof buf, "No se puede liberar un espacio ya libre: %x-%x se solapa con %x-%x", s, e, ns, ne);
    throw std::runtime_error(buf);
}
}  // namespace

void FreedSpace::free(int start, int length) {
    if (length < 1) throw std::invalid_argument("length must be at least 1");
    Chunk toFree{start, start + length - 1};
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it) {
        Chunk& n = *it;
        if (toFree.start < n.start) {
            // addChunkBeforeOrMergeWithRightNeighbor
            if (toFree.end >= n.start) alreadyFreed(toFree.start, toFree.end, n.start, n.end);
            if (toFree.end == n.start - 1) n.start = toFree.start;
            else chunks_.insert(it, toFree);
            return;
        }
        if (n.end == toFree.start - 1) {
            // mergeChunkWithLeftOrBothNeighbors
            auto next = std::next(it);
            if (next != chunks_.end()) {
                if (toFree.end >= next->start) alreadyFreed(toFree.start, toFree.end, next->start, next->end);
                if (toFree.end == next->start - 1) {
                    n.end = next->end;
                    chunks_.erase(next);
                } else {
                    n.end = toFree.end;
                }
            } else {
                n.end = toFree.end;
            }
            return;
        }
        if (n.end >= toFree.start) alreadyFreed(toFree.start, toFree.end, n.start, n.end);
    }
    chunks_.push_back(toFree);
}

int FreedSpace::findAndUnfree(int length) {
    for (auto it = chunks_.begin(); it != chunks_.end(); ++it) {
        if (it->end - it->start + 1 >= length) {
            int offset = it->start;
            it->start += length;
            if (it->start > it->end) chunks_.erase(it);
            return offset;
        }
    }
    return -1;
}

int FreedSpace::lengthSum() const {
    int s = 0;
    for (auto& c : chunks_) s += c.end - c.start + 1;
    return s;
}

std::string FreedSpace::toString() const {
    std::string s = std::to_string(lengthSum()) + " bytes, " + std::to_string(chunks_.size()) + " chunks";
    return s;
}

}  // namespace pr
