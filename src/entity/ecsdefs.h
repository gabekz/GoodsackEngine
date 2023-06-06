#ifndef H_ECSDEFS
#define H_ECSDEFS

// #ifdef __cplusplus
// enum ECSEvent { ECS_INIT = 0, ECS_DESTROY, ECS_RENDER, ECS_UPDATE };
// #else
enum ECSEvent {
    ECS_INIT = 0,
    ECS_DESTROY,
    ECS_RENDER,
    ECS_UPDATE,
    ECS_LATE_UPDATE
};
// #endif // __cplusplus

#define ECSEVENT_FIRST ECS_INIT
#define ECSEVENT_LAST  ECS_LATE_UPDATE

#define ECSEVENT_STRING(x) (_ecs_EventToString(x))

#define ECS_TAG_SIZE   1 // Tag size as bytes
#define ECS_TAG_UNUSED 0b00000000
#define ECS_TAG_USED   0b00110000

// Should always be True
#define USING_GENERATED_COMPONENTS    1

// Component struct Packing/Padding
#define ECS_COMPONENTS_PACKED         0
#if defined(SYS_ENV_WIN)
#define ECS_COMPONENTS_ALIGN_BYTES    16
#else
#define ECS_COMPONENTS_ALIGN_BYTES    12 
#endif // defined(SYS_ENV_WIN)

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
    case ECS_LATE_UPDATE: return "late_update";
    default: return "";
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef ECS_SYSTEM
// #define ECS_SYSTEM_DECLARE(_name) static
#endif

#endif // H_ECSDEFS
