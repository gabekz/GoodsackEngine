#ifndef HPP_ECS_ENTITY
#define HPP_ECS_ENTITY

// TODO: Temporary class for testing

namespace entity {

class ECSEntity {

   public:
    ECSEntity(int id) { m_id = id; };
    int getId() { return m_id; };

   private:
    int m_id;
};

} // namespace entity

#endif // HPP_ECS_ENTITY
