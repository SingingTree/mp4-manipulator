#include "parsing/atom_inspector.h"

#include <QTextStream>

namespace mp4_manipulator {
AtomInspector::AtomInspector() {
  // We want maximum verbosity.
  SetVerbosity(2);
}

void AtomInspector::StartAtom(char const* name, AP4_UI08 version,
                              AP4_UI32 flags, AP4_Size header_size,
                              AP4_UI64 size) {
  std::unique_ptr<AtomOrDescriptorBase> new_atom =
      std::make_unique<Atom>(name, header_size, size);
  if (current_atom_or_descriptor_ == nullptr) {
    // We're starting a top level atom.
    current_atom_or_descriptor_ = new_atom.get();
    top_level_atoms_.push_back(std::move(new_atom));
    return;
  }
  // We're nested inside another atom.
  AtomOrDescriptorBase* parent_atom = current_atom_or_descriptor_;
  current_atom_or_descriptor_ = new_atom.get();
  new_atom->SetParent(parent_atom);
  parent_atom->AddChildAtom(std::move(new_atom));
}

void AtomInspector::EndAtom() {
  current_atom_or_descriptor_ = current_atom_or_descriptor_->GetParent();
}

void AtomInspector::StartDescriptor(const char* name, AP4_Size header_size,
                                    AP4_UI64 size) {
  std::unique_ptr<AtomOrDescriptorBase> new_descriptor =
      std::make_unique<Descriptor>(name, header_size, size);
  if (current_atom_or_descriptor_ == nullptr) {
    // The descriptor is at the top level. This shouldn't happen, but
    // gracefully handle in case we're given weird input.
    current_atom_or_descriptor_ = new_descriptor.get();
    top_level_atoms_.push_back(std::move(new_descriptor));
    return;
  }
  // We're nested inside another atom.
  AtomOrDescriptorBase* parent_atom = current_atom_or_descriptor_;
  current_atom_or_descriptor_ = new_descriptor.get();
  new_descriptor->SetParent(parent_atom);
  parent_atom->AddChildDescriptor(std::move(new_descriptor));
}

void AtomInspector::EndDescriptor() {
  current_atom_or_descriptor_ = current_atom_or_descriptor_->GetParent();
}

void AtomInspector::AddField(char const* name, AP4_UI64 value,
                             FormatHint hint /* = HINT_NONE */) {
  assert(current_atom_or_descriptor_ != nullptr);
  // Use AP4 formatting to conform to lib expectations + use hints.
  char str[32];
  AP4_FormatString(str, sizeof(str), hint == HINT_HEX ? "%llx" : "%lld", value);

  QString value_string{};
  QTextStream(&value_string) << str;
  current_atom_or_descriptor_->AddField(QString(name), std::move(value_string));
}

void AtomInspector::AddFieldF(char const* name, float value,
                              FormatHint hint /* = HINT_NONE */) {
  assert(current_atom_or_descriptor_ != nullptr);
  QString value_string{};
  QTextStream(&value_string) << value;
  current_atom_or_descriptor_->AddField(QString(name), std::move(value_string));
}

void AtomInspector::AddField(char const* name, char const* value,
                             FormatHint hint /* = HINT_NONE */) {
  assert(current_atom_or_descriptor_ != nullptr);
  current_atom_or_descriptor_->AddField(QString(name), QString(value));
}

void AtomInspector::AddField(char const* name, unsigned char const* bytes,
                             AP4_Size byte_count,
                             FormatHint hint /* = HINT_NONE */) {
  assert(current_atom_or_descriptor_ != nullptr);
  QString value_string{};
  QTextStream stream(&value_string);
  stream << "[";
  for (size_t i = 0; i < byte_count; i++) {
    stream << QString::asprintf("%02x", bytes[i]);
    if (i < byte_count - 1) {
      stream << " ";
    }
  }
  stream << "]";
  current_atom_or_descriptor_->AddField(QString(name), std::move(value_string));
}

std::vector<std::unique_ptr<AtomOrDescriptorBase>> AtomInspector::TakeAtoms() {
  return std::move(top_level_atoms_);
}

AtomOrDescriptorBase* AtomInspector::GetPreviousSibling() {
  AtomOrDescriptorBase* parent = current_atom_or_descriptor_->GetParent();
  if (parent == nullptr) {
    // We're at the top level.
  }
  return nullptr;
}

}  // namespace mp4_manipulator
