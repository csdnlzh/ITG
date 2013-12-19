#if defined(_MSC_VER)
# include <hash_map>
using std::hash_map;
#elif defined(__GNUC__)
# include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
# error wrong
#endif


