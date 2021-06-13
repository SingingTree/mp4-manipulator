#ifndef MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_
#define MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_

#include <cstdint>
#include <unordered_map>

#include "Ap4.h"

namespace mp4_manipulator {
// An atom factory that also stores the position in the file of the atoms.
// This is useful in situations such as showing the offset of atoms in the UI.
class PositionAwareAtomFactory : public AP4_AtomFactory {
 public:
  PositionAwareAtomFactory() = default;

  AP4_Result CreateAtomFromStream(AP4_ByteStream& stream, AP4_UI32 type,
                                  AP4_UI32 size_32, AP4_UI64 size_64,
                                  AP4_Atom*& atom) override;

  // Moves atom_to_position_map_ out of the factory to the caller. This map
  // maps AP4_Atoms to a byte offset where they were read in the input stream.
  std::unordered_map<AP4_Atom*, uint64_t> TakeAtomToPositionMap();

 private:
  // A map from AP4_Atoms to a byte offset where they were read in the input
  // stream.
  std::unordered_map<AP4_Atom*, uint64_t> atom_to_position_map_;
};
}  // namespace mp4_manipulator

#endif  // MP4_MANIPULATOR_POSITION_AWARE_ATOM_FACTORY_
