#ifndef H_COMPONENT
#define H_COMPONENT

#include <string>
#include <map>

namespace ecs {

typedef struct _datatype { int size, stride; } DataType;
typedef struct _accessor { int position, size, stride ; } Accessor;

class ComponentLayout {
public:
    ComponentLayout(const char *name);
    ~ComponentLayout();

    void SetData(std::map<std::string, Accessor> data);

    // getters
    Accessor getAccessor(std::string var) { return m_Variables[var]; };
    ulong getSizeReq() { return m_SizeReq; };
    const char* getName() { return m_Name; };

private:
    std::map<std::string, Accessor> m_Variables;
    ulong m_SizeReq;
    const char *m_Name;
};

class Component {
public:
    Component(ComponentLayout &layout);

    template<typename T> int GetVariable(const char *var, T *destination);
    void SetVariable(const char *var, void *value);

private:
    ComponentLayout &m_ComponentLayout;
    struct { void *mem; int size, index; } m_Data;
};

void ParseComponents(const char *path);

}


#endif // H_COMPONENT
