#include "SlimList.h"
#include "SlimListDeserializer.h"
#include "SlimListSerializer.h"
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>

struct Node {
    Node* next = nullptr;
    bool isNull = false;   // true when this slot holds SLIM null (distinct from empty string)
    std::string string;
    SlimList* list = nullptr;
};

struct SlimList {
    int length = 0;
    Node* head = nullptr;
    Node* tail = nullptr;
};

static void insertNode(SlimList* self, Node* node);
static void SlimList_DestroyNode(Node* node);

SlimList* SlimList_Create()
{
    return new SlimList();
}

void SlimList_Destroy(SlimList* self)
{
    Node* node = self->head;
    while (node) {
        Node* next = node->next;
        SlimList_DestroyNode(node);
        node = next;
    }
    delete self;
}

static void SlimList_DestroyNode(Node* node)
{
    if (node->list)
        SlimList_Destroy(node->list);
    delete node;
}

SlimListIterator* SlimList_CreateIterator(SlimList* list)
{
    return list->head;
}

int SlimList_Iterator_HasItem(SlimListIterator* iterator)
{
    return iterator != nullptr;
}

void SlimList_Iterator_Advance(SlimListIterator** iterator)
{
    if (*iterator != nullptr)
        *iterator = (*iterator)->next;
}

void SlimList_AddBuffer(SlimList* self, char const* buffer, int length)
{
    Node* node = new Node();
    if (buffer)
        node->string.assign(buffer, static_cast<std::size_t>(length));
    else
        node->isNull = true;
    insertNode(self, node);
}

void SlimList_AddString(SlimList* self, char const* string)
{
    SlimList_AddBuffer(self, string, string ? static_cast<int>(strlen(string)) : 0);
}

void SlimList_AddList(SlimList* self, SlimList* element)
{
    char* embedded = SlimList_Serialize(element);
    SlimList_AddString(self, embedded);
    SlimList_Release(embedded);
}

void SlimList_PopHead(SlimList* self)
{
    assert(self->head != nullptr);
    Node* previousHead = self->head;
    self->head = previousHead->next;
    if (self->tail == previousHead)
        self->tail = nullptr;
    self->length--;
    SlimList_DestroyNode(previousHead);
}

int SlimList_GetLength(SlimList* self)
{
    return self->length;
}

int SlimList_Equals(SlimList* self, SlimList* other)
{
    if (self->length != other->length)
        return 0;
    for (Node *p = self->head, *q = other->head; p; p = p->next, q = q->next) {
        if (p->isNull != q->isNull || p->string != q->string)
            return 0;
    }
    return 1;
}

static Node* SlimList_GetNodeAt(SlimList* self, int index)
{
    if (index >= self->length)
        return nullptr;
    Node* node = self->head;
    for (int i = 0; i < index; i++)
        node = node->next;
    return node;
}

SlimList* SlimList_GetListAt(SlimList* self, int index)
{
    Node* node = SlimList_GetNodeAt(self, index);
    return SlimList_Iterator_GetList(node);
}

SlimList* SlimList_Iterator_GetList(SlimListIterator* iterator)
{
    assert(iterator != nullptr);
    if (!iterator->list)
        iterator->list = SlimList_Deserialize(iterator->string.c_str());
    return iterator->list;
}

const char* SlimList_GetStringAt(SlimList* self, int index)
{
    Node* node = SlimList_GetNodeAt(self, index);
    if (!node)
        return nullptr;
    return SlimList_Iterator_GetString(node);
}

const char* SlimList_Iterator_GetString(SlimListIterator* iterator)
{
    assert(iterator != nullptr);
    return iterator->isNull ? nullptr : iterator->string.c_str();
}

double SlimList_GetDoubleAt(SlimList* self, int index)
{
    const char* s = SlimList_GetStringAt(self, index);
    return s ? atof(s) : 0.0;
}

static std::string parseHashCell(const char** cellStart)
{
    const char* cellValue = *cellStart + 4;  // skip "<td>"
    const char* cellStop = strstr(cellValue, "</td>");
    if (!cellStop) return "";
    std::string result(cellValue, cellStop - cellValue);
    *cellStart = strstr(cellStop + 4, "<td>");
    return result;
}

static SlimList* parseHashEntry(const char* row)
{
    SlimList* element = SlimList_Create();
    const char* cellStart = strstr(row, "<td>");
    if (cellStart) {
        std::string hashKey = parseHashCell(&cellStart);
        SlimList_AddString(element, hashKey.c_str());
        if (cellStart) {
            std::string hashValue = parseHashCell(&cellStart);
            SlimList_AddString(element, hashValue.c_str());
        }
    }
    return element;
}

static SlimList* SlimList_deserializeHash(const char* serializedHash)
{
    SlimList* hash = SlimList_Create();
    const char* row = strstr(serializedHash, "<tr>");
    while (row != nullptr) {
        SlimList* element = parseHashEntry(row);
        SlimList_AddList(hash, element);
        SlimList_Destroy(element);
        row = strstr(row + 4, "<tr>");
    }
    return hash;
}

SlimList* SlimList_GetHashAt(SlimList* self, int index)
{
    return SlimList_deserializeHash(SlimList_GetStringAt(self, 0));
    (void)index;
}

void SlimList_ReplaceAt(SlimList* self, int index, char const* replacementString)
{
    Node* node = SlimList_GetNodeAt(self, index);
    SlimList_Iterator_Replace(node, replacementString);
}

void SlimList_Iterator_Replace(SlimListIterator* iterator, const char* replacementString)
{
    if (iterator->list) {
        SlimList_Destroy(iterator->list);
        iterator->list = nullptr;
    }
    iterator->isNull = (replacementString == nullptr);
    iterator->string = replacementString ? replacementString : "";
}

static void insertNode(SlimList* self, Node* node)
{
    if (self->length == 0)
        self->head = node;
    else
        self->tail->next = node;
    self->tail = node;
    self->length++;
}

void SlimList_Iterator_AdvanceBy(SlimListIterator** iterator, int amount)
{
    for (int i = 0; i < amount; i++)
        SlimList_Iterator_Advance(iterator);
}

SlimList* SlimList_GetTailAt(SlimList* self, int index)
{
    SlimList* tail = SlimList_Create();
    SlimListIterator* it = SlimList_CreateIterator(self);
    SlimList_Iterator_AdvanceBy(&it, index);
    while (SlimList_Iterator_HasItem(it)) {
        SlimList_AddString(tail, SlimList_Iterator_GetString(it));
        SlimList_Iterator_Advance(&it);
    }
    return tail;
}

const char* SlimList_ToString(SlimList* self)
{
    std::string result = "[";
    SlimListIterator* it = SlimList_CreateIterator(self);
    while (SlimList_Iterator_HasItem(it)) {
        SlimList* sublist = SlimList_Iterator_GetList(it);
        if (sublist) {
            const char* sub = SlimList_ToString(sublist);
            result += sub;
            std::free(const_cast<char*>(sub));
        } else {
            result += '"';
            result += SlimList_Iterator_GetString(it);
            result += '"';
        }
        SlimList_Iterator_Advance(&it);
        if (SlimList_Iterator_HasItem(it))
            result += ", ";
    }
    result += "]";
    char* ret = static_cast<char*>(std::malloc(result.size() + 1));
    std::memcpy(ret, result.c_str(), result.size() + 1);
    return ret;
}
