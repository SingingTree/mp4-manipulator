#include "parsing/position_aware_atom_factory.h"

namespace mp4_manipulator {
AP4_Result PositionAwareAtomFactory::CreateAtomFromStream(
    AP4_ByteStream& stream, AP4_UI32 type, AP4_UI32 size_32, AP4_UI64 size_64,
    AP4_Atom*& atom) {
  // TODO(bryce): we could build time assert AP4_Position is of expected width.

  // At the time this is called into, `stream` is positioned after the atom
  // header. I.e. we need to move backwards from the stream position to get
  // the position where our atom header starts.

  // Get the position before the atom header.
  AP4_Position initial_stream_position;
  stream.Tell(initial_stream_position);

  bool const atom_is_large = (size_32 == 1);
  unsigned int payload_offset = 8;
  if (atom_is_large) {
    payload_offset += 8;
  }

  assert(initial_stream_position >= payload_offset);
  // Figure out the start of our atom before the header.
  AP4_Position atom_start_position = initial_stream_position - payload_offset;

  AP4_Result result = AP4_AtomFactory::CreateAtomFromStream(
      stream, type, size_32, size_64, atom);
  if (AP4_FAILED(result)) {
    return result;
  }

  if (atom == nullptr) {
    // Normally the method we return to would handle this case and construct an
    // AP4_UnknownAtom. However, this means we can't track the positions of
    // such atoms. So we duplicate the atom creation logic from AP4_AtomFactory
    // here so our position aware factory can track the atoms.
    stream.Seek(initial_stream_position);
    AP4_UI64 const size = atom_is_large ? size_64 : size_32;
    atom = new AP4_UnknownAtom(type, size, stream);
  }

  // Insertions should always create a new element. I.e. we should not have
  // collisions as each atom is unique.
  bool const inserted = atom_to_position_map_.insert({atom, atom_start_position}).second;
  assert(inserted);

  return AP4_SUCCESS;
}

std::unordered_map<AP4_Atom*, uint64_t>
PositionAwareAtomFactory::TakeAtomToPositionMap() {
  return std::move(atom_to_position_map_);
}

}  // namespace mp4_manipulator
