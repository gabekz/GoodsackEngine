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

    template<typename T> int GetVariable(const char *var, T *destination);
    void SetVariable(const char *var, void *value);

    const char* getName() { return m_Name; };

private:
    std::map<std::string, Accessor> m_Variables;
    const char *m_Name;
    void *m_DataArray;

    struct {
        void *mem;
        int size;
        int index;
    } m_Data;
};

void ParseComponents(const char *path);

}


#endif // H_COMPONENT
