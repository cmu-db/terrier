file(REMOVE_RECURSE
  "../debug/libterrier.pdb"
  "../debug/libterrier.a"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/terrier_static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
