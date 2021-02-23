#ifndef MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_
#define MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_

#include <unordered_map>

#include "AP4.h"

namespace mp4_manipulator {
class PositionAwareAtomFactory : public AP4_AtomFactory {
 public:
  PositionAwareAtomFactory() = default;

  AP4_Result CreateAtomFromStream(AP4_ByteStream& stream, AP4_UI32 type,
                                  AP4_UI32 size_32, AP4_UI64 size_64,
                                  AP4_Atom*& atom) override;

  // Moves atom_to_position_map_ out of the factory to the caller.
  std::unordered_map<AP4_Atom*, uint64_t> TakeAtomToPositionMap();

 private:
  std::unordered_map<AP4_Atom*, uint64_t> atom_to_position_map_;
};
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_