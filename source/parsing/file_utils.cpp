#include "parsing/file_utils.h"

#include <iostream>

#include "Ap4.h"
#include "parsing/atom_inspector.h"
#include "parsing/position_aware_atom_factory.h"

namespace mp4_manipulator::utility {
namespace {
void RecursiveSetAtomPositions(
    AtomOrDescriptorBase* atom,
    std::unordered_map<AP4_Atom*, uint64_t> const& atom_to_position_map) {
  AP4_Atom* ap4_atom = atom->GetAp4Atom();
  assert(ap4_atom != nullptr);
  if (ap4_atom == nullptr) {
    return;
  }

  size_t count = atom_to_position_map.count(ap4_atom);
  assert(count == 1);
  if (count != 1) {
    return;
  }

  uint64_t position = atom_to_position_map.at(ap4_atom);
  atom->SetPositionInStream(position);

  for (auto& child_atom : atom->GetChildAtoms()) {
    RecursiveSetAtomPositions(child_atom.get(), atom_to_position_map);
  }
}

// Walks the parsed mp4_manipulator atom tree and sets offsets based on their
// associated AP4 atoms. This should be called after `MatchAtoms` is used to
// link the mp4_manipulator atoms to their AP4 counterparts.
void SetAtomPositions(
    AtomHolder& atom_holder,
    std::unordered_map<AP4_Atom*, uint64_t> const& atom_to_position_map) {
  for (auto& child_atom : atom_holder.GetTopLevelAtoms()) {
    RecursiveSetAtomPositions(child_atom.get(), atom_to_position_map);
  }
}
}  // namespace

std::optional<std::unique_ptr<AtomHolder>> ReadAtoms(AP4_ByteStream* input) {
  std::unique_ptr<AtomInspector> inspector = std::make_unique<AtomInspector>();
  // Grab top level atoms, store and inspect them.
  AP4_Atom* atom;
  PositionAwareAtomFactory atom_factory;
  // We want virtual functions to be used, so grab a ptr.
  AP4_AtomFactory* atom_factory_ptr =
      static_cast<AP4_AtomFactory*>(&atom_factory);
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms;
  while (atom_factory_ptr->CreateAtomFromStream(*input, atom) == AP4_SUCCESS) {
    // This AP4_Position code if from the mp4 dump source. There it's suggested
    // that inspect could change the stream position so that this is needed.
    // It's not clear that it is... The code is kept in case uncommenting it
    // ever proves useful for fixing bad parses.
    // AP4_Position position;
    // input->Tell(position);

    // inspect the atom
    atom->Inspect(*inspector);

    // restore the previous stream position
    // input->Seek(position);

    top_level_ap4_atoms.emplace_back(std::unique_ptr<AP4_Atom>(atom));
  }

  std::unique_ptr<AtomHolder> holder = std::make_unique<AtomHolder>(
      std::move(inspector->TakeAtoms()), std::move(top_level_ap4_atoms));

  std::unordered_map<AP4_Atom*, uint64_t> const atom_to_position_map =
      atom_factory.TakeAtomToPositionMap();

  SetAtomPositions(*holder, atom_to_position_map);

  return holder;
}

std::optional<std::unique_ptr<AtomHolder>> ReadAtoms(char const* file_name) {
  // We don't bother using a file approach, because the atoms are not in the
  // same order as if the boxes are streamed. An example of how to use AP4's
  // file API is shown below, but again, we don't want to do this.
  // AP4_File* file = new AP4_File(*input, false);
  // file->Inspect(*inspector);

  AP4_ByteStream* input = NULL;
  AP4_Result result = AP4_FileByteStream::Create(
      file_name, AP4_FileByteStream::STREAM_MODE_READ, input);
  if (AP4_FAILED(result)) {
    // TODO(bryce): Tie this into some global error handler.
    fprintf(stderr, "ERROR: cannot open input file %s (%d)\n", file_name,
            result);
    return std::nullopt;
  }

  return ReadAtoms(input);
}

void DumpAtom(char const* output_file_name, AP4_Atom& atom) {
  AP4_ByteStream* output = nullptr;
  AP4_Result result = AP4_FileByteStream::Create(
      output_file_name, AP4_FileByteStream::STREAM_MODE_WRITE, output);
  if (AP4_FAILED(result)) {
    // fprintf(stderr, "ERROR: cannot open output file (%s)\n",
    // output_filename);
    // TODO(bryce): error handling
    return;
  }

  atom.Write(*output);

  // cleanup
  output->Release();
}
}  // namespace mp4_manipulator::utility
