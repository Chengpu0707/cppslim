#include "SlimList.h"
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>

// ---------------------------------------------------------------------------
// SlimListIterator
// ---------------------------------------------------------------------------

SlimListIterator* SlimListIterator::advanceBy(int n) const
{
    const SlimListIterator* it = this;
    for (int i = 0; i < n && it; ++i)
        it = it->next_;
    return const_cast<SlimListIterator*>(it);
}

SlimList* SlimListIterator::getList()
{
    if (!list_)
        list_ = SlimList::deserialize(string_.c_str());
    return list_;
}

void SlimListIterator::replace(const char* s)
{
    if (list_) {
        delete list_;
        list_ = nullptr;
    }
    isNull_ = (s == nullptr);
    string_ = s ? s : "";
}

// ---------------------------------------------------------------------------
// SlimList
// ---------------------------------------------------------------------------

SlimList::SlimList() = default;

SlimList::~SlimList()
{
    SlimListIterator* node = head_;
    while (node) {
        SlimListIterator* next = node->advance();
        delete node->list_;
        delete node;
        node = next;
    }
}

void SlimList::insertNode(SlimListIterator* node)
{
    if (length_ == 0)
        head_ = node;
    else
        tail_->next_ = node;
    tail_ = node;
    ++length_;
}

void SlimList::addBuffer(const char* buf, int length)
{
    SlimListIterator* node = new SlimListIterator();
    if (buf)
        node->string_.assign(buf, static_cast<std::size_t>(length));
    else
        node->isNull_ = true;
    insertNode(node);
}

void SlimList::addString(const char* s)
{
    addBuffer(s, s ? static_cast<int>(std::strlen(s)) : 0);
}

void SlimList::addList(SlimList* element)
{
    char* embedded = element->serialize();
    addString(embedded);
    SlimList::release(embedded);
}

void SlimList::popHead()
{
    assert(head_ != nullptr);
    SlimListIterator* prev = head_;
    head_ = prev->next_;
    if (tail_ == prev)
        tail_ = nullptr;
    --length_;
    delete prev->list_;
    delete prev;
}

bool SlimList::equals(const SlimList* other) const
{
    if (length_ != other->length_)
        return false;
    for (SlimListIterator *p = head_, *q = other->head_; p; p = p->advance(), q = q->advance())
        if (p->isNull_ != q->isNull_ || p->string_ != q->string_)
            return false;
    return true;
}

SlimListIterator* SlimList::getNodeAt(int index) const
{
    if (index >= length_)
        return nullptr;
    SlimListIterator* node = head_;
    for (int i = 0; i < index; ++i)
        node = node->advance();
    return node;
}

const char* SlimList::getStringAt(int index) const
{
    SlimListIterator* node = getNodeAt(index);
    return node ? node->getString() : nullptr;
}

double SlimList::getDoubleAt(int index) const
{
    const char* s = getStringAt(index);
    return s ? std::atof(s) : 0.0;
}

SlimList* SlimList::getListAt(int index) const
{
    SlimListIterator* node = getNodeAt(index);
    return node ? node->getList() : nullptr;
}

void SlimList::replaceAt(int index, const char* s)
{
    SlimListIterator* node = getNodeAt(index);
    if (node)
        node->replace(s);
}

SlimList* SlimList::getTailAt(int index) const
{
    SlimList* tail = new SlimList();
    for (auto* it = head_->advanceBy(index); it != nullptr; it = it->advance())
        tail->addString(it->getString());
    return tail;
}

static std::string parseHashCell(const char** cellStart)
{
    const char* cellValue = *cellStart + 4;
    const char* cellStop  = std::strstr(cellValue, "</td>");
    if (!cellStop) return "";
    std::string result(cellValue, cellStop - cellValue);
    *cellStart = std::strstr(cellStop + 4, "<td>");
    return result;
}

static SlimList* parseHashEntry(const char* row)
{
    SlimList* element  = new SlimList();
    const char* start = std::strstr(row, "<td>");
    if (start) {
        std::string key = parseHashCell(&start);
        element->addString(key.c_str());
        if (start) {
            std::string val = parseHashCell(&start);
            element->addString(val.c_str());
        }
    }
    return element;
}

SlimList* SlimList::getHashAt(int index) const
{
    const char* s = getStringAt(index);
    SlimList* hash = new SlimList();
    const char* row = std::strstr(s ? s : "", "<tr>");
    while (row) {
        SlimList* entry = parseHashEntry(row);
        hash->addList(entry);
        delete entry;
        row = std::strstr(row + 4, "<tr>");
    }
    return hash;
}

const char* SlimList::toString() const
{
    std::string result = "[";
    for (auto* it = head_; it != nullptr; it = it->advance()) {
        SlimList* sub = it->getList();
        if (sub) {
            const char* s = sub->toString();
            result += s;
            SlimList::release(const_cast<char*>(s));
        } else {
            result += '"';
            result += it->getString();
            result += '"';
        }
        if (it->advance())
            result += ", ";
    }
    result += "]";
    char* ret = static_cast<char*>(std::malloc(result.size() + 1));
    std::memcpy(ret, result.c_str(), result.size() + 1);
    return ret;
}
