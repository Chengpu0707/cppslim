#pragma once
#include <string>

class SlimList;

// SlimListIterator is both a linked-list node and the iterator handle.
// Calling advance() on a non-null iterator returns the next node (or nullptr).
// Always check for nullptr before calling any method: it != nullptr.
class SlimListIterator {
    friend class SlimList;
public:
    SlimListIterator* advance()       const { return next_; }
    SlimListIterator* advanceBy(int n) const;

    const char* getString() const { return isNull_ ? nullptr : string_.c_str(); }
    SlimList*   getList();
    void        replace(const char* s);

private:
    SlimListIterator* next_   = nullptr;
    bool              isNull_ = false;
    std::string       string_;
    SlimList*         list_   = nullptr;
};

class SlimList {
public:
    SlimList();
    ~SlimList();

    // Mutation
    void addString(const char* s);
    void addList(SlimList* element);
    void addBuffer(const char* buf, int length);
    void popHead();
    void replaceAt(int index, const char* s);

    // Query
    int         getLength()               const { return length_; }
    bool        equals(const SlimList* other) const;
    const char* getStringAt(int index)    const;
    double      getDoubleAt(int index)    const;
    SlimList*   getListAt(int index)      const;
    SlimList*   getHashAt(int index)      const;
    SlimList*   getTailAt(int index)      const;
    const char* toString()                const; // caller must call SlimList::release()

    // Iteration
    SlimListIterator* createIterator() const { return head_; }

    // Serialization  (implemented in SlimListSerializer.cpp)
    char* serialize()          const;
    int   serializedLength()   const;
    static void release(char* serialized);

    // Deserialization (implemented in SlimListDeserializer.cpp)
    static SlimList* deserialize(const char* serialized);

private:
    int               length_ = 0;
    SlimListIterator* head_   = nullptr;
    SlimListIterator* tail_   = nullptr;

    SlimListIterator* getNodeAt(int index) const;
    void              insertNode(SlimListIterator* node);
};
