#ifndef MP4_MANIPULATOR_ATOM_HOLDER_H_
#define MP4_MANIPULATOR_ATOM_HOLDER_H_

#include <memory>
#include <vector>

#include "atom.h"

namespace mp4_manipulator {

class AtomHolder {
 public:
  AtomHolder(
      std::vector<std::unique_ptr<AtomOrDescriptorBase>>&& top_level_atoms,
      std::vector<std::unique_ptr<AP4_Atom>>&& top_level_ap4_atoms);
  std::vector<std::unique_ptr<AtomOrDescriptorBase>>& GetTopLevelAtoms();

 private:
  std::vector<std::unique_ptr<AtomOrDescriptorBase>> top_level_atoms_;
  std::vector<std::unique_ptr<AP4_Atom>> top_level_ap4_atoms_;

  // Walks the parsed AP4 atom tree and mp4_manipulator trees and sets pointers
  // on the mp4_manipulator atoms to their AP4 counterparts.
  void MatchAtoms();
};

}  // namespace mp4_manipulator
#endif  // MP4_MANIPULATOR_ATOM_HOLDER_H_