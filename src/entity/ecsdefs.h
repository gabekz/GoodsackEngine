#ifndef H_ECSDEFS
#define H_ECSDEFS

// #ifdef __cplusplus
// enum ECSEvent { ECS_INIT = 0, ECS_DESTROY, ECS_RENDER, ECS_UPDATE };
// #else
enum ECSEvent { ECS_INIT = 0, ECS_DESTROY, ECS_RENDER, ECS_UPDATE };
// #endif // __cplusplus

#define ECSEVENT_FIRST ECS_INIT
#define ECSEVENT_LAST  ECS_UPDATE

#define ECSEVENT_STRING(x) (_ecs_EventToString(x))

#define ECS_TAG_SIZE   1 // Tag size as bytes
#define ECS_TAG_UNUSED 0b00000000
#define ECS_TAG_USED   0b00110000

#define USING_GENERATED_COMPONENTS 0

#ifdef __cplusplus
extern "C" {
#endif // extern "C"

// Event enum as a [Lua] function name
inline const char *
_ecs_EventToString(int event)
{
    switch (event) {
    case ECS_INIT: return "start";
    case ECS_DESTROY: return "destroy";
    case ECS_RENDER: return "render";
    case ECS_UPDATE: return "update";
    default: return "";
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // H_ECSDEFS
