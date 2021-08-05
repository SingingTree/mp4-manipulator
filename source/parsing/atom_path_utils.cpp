#include "parsing/atom_path_utils.h"

Ap4CompatiblePath GetAp4Path(AP4_Atom* atom, AP4_List<AP4_Atom>& top_level) {
  size_t atom_depth = 0;
  for (AP4_Atom* parent = AP4_DYNAMIC_CAST(AP4_Atom, atom->GetParent());
       parent != nullptr;
       parent = AP4_DYNAMIC_CAST(AP4_Atom, parent->GetParent())) {
    ++atom_depth;
  }

  std::vector<Ap4CompatiblePathItem> path{atom_depth + 1};
  AP4_Atom* current_atom = atom;
  size_t current_path_item_index = atom_depth;

  // Loop through parents until we reach the root, figure out the index at each
  // level.
  while (current_atom != nullptr) {
    AP4_Ordinal preceding_atoms_with_same_type = 0;
    AP4_AtomParent* parent = current_atom->GetParent();
    AP4_List<AP4_Atom>& atom_and_siblings =
        parent != nullptr ? parent->GetChildren() : top_level;
    bool is_atom_within_siblings = false;

    // Loop through the current atom + its siblings to figure out the path
    // index.
    for (AP4_List<AP4_Atom>::Item* item = atom_and_siblings.FirstItem();
         item != nullptr; item = item->GetNext()) {
      AP4_Atom* item_atom = item->GetData();
      assert(item_atom);
      if (item_atom == current_atom) {
        is_atom_within_siblings = true;
        break;
      }

      if (item_atom->GetType() == current_atom->GetType()) {
        ++preceding_atoms_with_same_type;
      }
    }
    // If all atoms were iterated and we didn't find current_atom within its
    // siblings, then something is busted.
    assert(is_atom_within_siblings);

    Ap4CompatiblePathItem& path_item = path.at(current_path_item_index);
    path_item.type = current_atom->GetType();
    path_item.index = preceding_atoms_with_same_type;

    --current_path_item_index;
    current_atom = AP4_DYNAMIC_CAST(AP4_Atom, atom->GetParent());
  }

  return Ap4CompatiblePath{path};
}

Ap4CompatiblePath GetAp4Path(AP4_Atom* atom, AP4_AtomParent& top_level) {
  return GetAp4Path(atom, top_level.GetChildren());
}

std::optional<AP4_Atom*> GetAp4AtomFromPath(Ap4CompatiblePath& path,
                                            AP4_List<AP4_Atom>& top_level) {
  AP4_List<AP4_Atom>& atoms_at_current_depth = top_level;
  AP4_Atom* current_atom = nullptr;
  for (size_t i = 0; i < path.path.size(); ++i) {
    Ap4CompatiblePathItem path_item = path.path.at(i);
    AP4_Ordinal preceding_atoms_with_same_type_left = path_item.index;
    for (AP4_List<AP4_Atom>::Item* item = atoms_at_current_depth.FirstItem();
         item != nullptr; item = item->GetNext()) {
      AP4_Atom* item_atom = item->GetData();
      assert(item_atom);
      if (item_atom->GetType() == path_item.type) {
        if (preceding_atoms_with_same_type_left == 0) {
          // We've found the atom for this step in the path, go to the next
          // step.
          current_atom = item_atom;
          break;
        } else {
          --preceding_atoms_with_same_type_left;
        }
      }
    }
  }

  if (current_atom == nullptr) {
    assert(false);
    return std::nullopt;
  }
  return current_atom;
}
std::optional<AP4_Atom*> GetAp4AtomFromPath(Ap4CompatiblePath& path,
                                            AP4_AtomParent& top_level) {
  return GetAp4AtomFromPath(path, top_level.GetChildren());
}