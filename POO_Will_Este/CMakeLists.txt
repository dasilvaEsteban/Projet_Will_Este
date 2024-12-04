# CMakeList.txt : projet CMake pour POO_Ant_Alek, incluez la source et définissez
# la logique spécifique au projet ici.
#

# Ajoutez une source à l'exécutable de ce projet.
add_executable (POO_Will_Este "main.cpp"  "traffic_light.hpp" "traffic_light.cpp" "car.hpp" "car.cpp" "bus.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET POO_Will_Este PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(POO_Will_Este PRIVATE ${SFML_LIBS})
# TODO: Ajoutez des tests et installez des cibles si nécessaire.
