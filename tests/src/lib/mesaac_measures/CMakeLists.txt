add_mesaac_test(
  TEST_NAME
  test_shape_measures_factory
  SOURCES
  test_shape_measures_factory.cpp
  LIBS
  mesaac_common
  mesaac_measures)

set(MEASURES bub cosine euclidean hamann tanimoto tversky)

foreach(MEASURE IN LISTS MEASURES)
  add_mesaac_test(
    TEST_NAME
    test_${MEASURE}
    SOURCES
    test_${MEASURE}.cpp
    LIBS
    mesaac_common
    mesaac_measures)
endforeach()
