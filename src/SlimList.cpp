#include "SlimList.h"
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>

// ---------------------------------------------------------------------------
// SlimListIterator
// ---------------------------------------------------------------------------

SlimListIterator::~SlimListIterator() = default;

SlimListIterator* SlimListIterator::advanceBy(int n) const
{
    const SlimListIterator* it = this;
    for (int i = 0; i < n && it; ++i)
        it = it->next_.get();
    return const_cast<SlimListIterator*>(it);
}

SlimList* SlimListIterator::getList()
{
    if (!list_)
        list_ = SlimList::deserialize(string_.c_str());
    return list_.get();
}

void SlimListIterator::replace(const char* s)
{
    list_.reset();
    isNull_ = (s == nullptr);
    string_ = s ? s : "";
}

// ---------------------------------------------------------------------------
// SlimList
// ---------------------------------------------------------------------------

SlimList::SlimList() = default;

void SlimList::insertNode(std::unique_ptr<SlimListIterator> node)
{
    SlimListIterator* raw = node.get();
    if (length_ == 0)
        head_ = std::move(node);
    else
        tail_->next_ = std::move(node);
    tail_ = raw;
    ++length_;
}

void SlimList::addBuffer(const char* buf, int length)
{
    auto node = std::make_unique<SlimListIterator>();
    if (buf)
        node->string_.assign(buf, static_cast<std::size_t>(length));
    else
        node->isNull_ = true;
    insertNode(std::move(node));
}

void SlimList::addString(const char* s)
{
    addBuffer(s, s ? static_cast<int>(std::strlen(s)) : 0);
}

void SlimList::addList(SlimList* element)
{
    addString(element->serialize().c_str());
}

void SlimList::popHead()
{
    assert(head_ != nullptr);
    auto new_head = std::move(head_->next_);
    if (tail_ == head_.get())
        tail_ = nullptr;
    head_ = std::move(new_head);
    --length_;
}

bool SlimList::equals(const SlimList* other) const
{
    if (length_ != other->length_)
        return false;
    for (SlimListIterator *p = head_.get(), *q = other->head_.get(); p;
         p = p->advance(), q = q->advance())
        if (p->isNull_ != q->isNull_ || p->string_ != q->string_)
            return false;
    return true;
}

SlimListIterator* SlimList::getNodeAt(int index) const
{
    if (index >= length_)
        return nullptr;
    SlimListIterator* node = head_.get();
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

std::unique_ptr<SlimList> SlimList::getTailAt(int index) const
{
    auto tail = std::make_unique<SlimList>();
    for (auto* it = getNodeAt(index); it != nullptr; it = it->advance())
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

static std::unique_ptr<SlimList> parseHashEntry(const char* row)
{
    auto element  = std::make_unique<SlimList>();
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

std::unique_ptr<SlimList> SlimList::getHashAt(int index) const
{
    const char* s = getStringAt(index);
    auto hash = std::make_unique<SlimList>();
    const char* row = std::strstr(s ? s : "", "<tr>");
    while (row) {
        auto entry = parseHashEntry(row);
        hash->addList(entry.get());
        row = std::strstr(row + 4, "<tr>");
    }
    return hash;
}

std::string SlimList::toString() const
{
    std::string result = "[";
    for (auto* it = head_.get(); it != nullptr; it = it->advance()) {
        SlimList* sub = it->getList();
        if (sub) {
            result += sub->toString();
        } else {
            result += '"';
            result += it->getString();
            result += '"';
        }
        if (it->advance())
            result += ", ";
    }
    result += "]";
    return result;
}
