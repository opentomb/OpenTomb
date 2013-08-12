"g++.exe" -c "bullet/LinearMath/*.cpp" "bullet/BulletSoftBody/*.cpp" "bullet/BulletDynamics/ConstraintSolver/*.cpp" "bullet/BulletDynamics/Dynamics/*.cpp" "bullet/BulletDynamics/Vehicle/*.cpp" "bullet/BulletCollision/BroadphaseCollision/*.cpp" "bullet/BulletCollision/CollisionDispatch/*.cpp" "bullet/BulletCollision/CollisionShapes/*.cpp" "bullet/BulletCollision/Gimpact/*.cpp" "bullet/BulletCollision/NarrowPhaseCollision/*.cpp" -I"bullet" -s -O3 -march=prescott
ar rcs libbullete.a *.o
rm -rf *.o
