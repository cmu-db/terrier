#include "storage/index/hash_key.h"

namespace terrier::storage::index {

template class HashKey<8>;
template class HashKey<16>;
template class HashKey<32>;
template class HashKey<64>;
template class HashKey<128>;
template class HashKey<256>;

}  // namespace terrier::storage::index
