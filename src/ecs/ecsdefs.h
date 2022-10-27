#ifndef H_ECSDEFS
#define H_ECSDEFS

#define ECSEVENT_FIRST ECS_INIT
#define ECSEVENT_LAST ECS_UPDATE
enum ECSEvent {
    ECS_INIT = 0, ECS_DESTROY, ECS_RENDER, ECS_UPDATE
};

#ifdef __cplusplus
namespace ecs {
#endif

// Event enum as a [Lua] function name
inline const char* EventToString(int event) {
    switch(event) {
        case ECS_INIT:      return "start";
        case ECS_DESTROY:   return "destroy";
        case ECS_RENDER:    return "render";
        case ECS_UPDATE:    return "update";
        default:            return "";
    }
}

#ifdef __cplusplus
} // namespace
#endif

#define ECS_TAG_SIZE 1 // Tag size as bytes
#define ECS_TAG_UNUSED  0b00000000
#define ECS_TAG_USED    0b00110000

#endif // H_ECSDEFS
