add_subdirectory(mesaac_mol)

message(WARNING "Not building mesaac_shape_eigen.  It depends on the obsolete Eigen2.")
# add_subdirectory(mesaac_shape_eigen)

add_subdirectory(mesaac_shape)
add_subdirectory(svd)
