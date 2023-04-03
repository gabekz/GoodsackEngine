#ifndef HPP_ECS_COMPONENT_LAYOUT
#define HPP_ECS_COMPONENT_LAYOUT

#include <map>
#include <string>
#include <util/sysdefs.h>

namespace entity {

typedef struct _datatype
{
    int size, stride;
} DataType;
typedef struct _accessor
{
    int position, size, stride;
} Accessor;

class ECSComponentLayout {
   public:
    ECSComponentLayout(const char *name);
    ~ECSComponentLayout();

    void SetData(std::map<std::string, Accessor> data);

    std::map<std::string, Accessor> getData() { return m_Variables; };
    Accessor getAccessor(std::string var) { return m_Variables[var]; };
    ulong getSizeReq() { return m_SizeReq; };
    const char *getName() { return m_Name; };

   private:
    std::map<std::string, Accessor> m_Variables;
    ulong m_SizeReq;
    const char *m_Name;
};

} // namespace entity

#endif // HPP_ECS_COMPONENT_LAYOUT
