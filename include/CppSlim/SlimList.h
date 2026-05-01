#pragma once
#include <string>
#include <memory>

class SlimList;

class SlimListIterator {
    friend class SlimList;
public:
    ~SlimListIterator();
    SlimListIterator* advance()        const { return next_.get(); }
    SlimListIterator* advanceBy(int n) const;
    const char* getString() const { return isNull_ ? nullptr : string_.c_str(); }
    SlimList*   getList();
    void        replace(const char* s);
private:
    std::unique_ptr<SlimListIterator> next_;
    bool        isNull_ = false;
    std::string string_;
    std::unique_ptr<SlimList> list_;
};

class SlimList {
public:
    SlimList();
    ~SlimList() = default;
    SlimList(SlimList&& other) noexcept;
    SlimList& operator=(SlimList&& other) noexcept;
    SlimList(const SlimList&) = delete;
    SlimList& operator=(const SlimList&) = delete;

    void addString(const char* s);
    void addList(SlimList*);
    void addBuffer(const char*, int);
    void popHead();
    void replaceAt(int, const char*);

    int  getLength() const { return length_; }
    bool equals(const SlimList* other) const;

    const char* getStringAt(int) const;
    double      getDoubleAt(int) const;
    SlimList*   getListAt(int) const;
    SlimList    getHashAt(int) const;
    SlimList    getTailAt(int) const;
    std::string toString() const;

    SlimListIterator* createIterator() const { return head_.get(); }

    std::string serialize() const;
    int         serializedLength() const;

    static std::unique_ptr<SlimList> deserialize(const char*);

private:
    int               length_ = 0;
    std::unique_ptr<SlimListIterator> head_;
    SlimListIterator* tail_   = nullptr;

    SlimListIterator* getNodeAt(int) const;
    void              insertNode(std::unique_ptr<SlimListIterator>);
};
