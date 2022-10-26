#ifndef HPP_COMPONENT_LAYOUT
#define HPP_COMPONENT_LAYOUT

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
    std::map<std::string, Accessor> GetData() { return m_Variables; };
    Accessor getAccessor(std::string var) { return m_Variables[var]; };
    ulong getSizeReq() { return m_SizeReq; };
    const char* getName() { return m_Name; };

private:
    std::map<std::string, Accessor> m_Variables;
    ulong m_SizeReq;
    const char *m_Name;
};

};

#endif // H
