#include "atom.h"

namespace mp4_manipulator {

AtomOrDescriptorBase::AtomOrDescriptorBase(char const* name,
                                           uint32_t header_size, uint64_t size)
    : name_{name}, header_size_{header_size}, size_{size} {}

AtomOrDescriptorBase::~AtomOrDescriptorBase(){};

QString const& AtomOrDescriptorBase::GetName() const { return name_; }

uint32_t AtomOrDescriptorBase::GetHeaderSize() const { return header_size_; }

uint64_t AtomOrDescriptorBase::GetSize() const { return size_; }

std::optional<uint64_t> AtomOrDescriptorBase::GetPositionInFile() const { return position_in_file_; }

void AtomOrDescriptorBase::SetPositionInFile(uint64_t position_in_file) {
  position_in_file_ = position_in_file;
}


std::vector<Field> const& AtomOrDescriptorBase::GetFields() const {
  return fields_;
}

void AtomOrDescriptorBase::AddField(QString&& name, QString&& data) {
  Field& field = fields_.emplace_back();
  field.name = std::move(name);
  field.data = std::move(data);
}

AtomOrDescriptorBase* AtomOrDescriptorBase::GetParent() const {
  return parent_;
}

void AtomOrDescriptorBase::SetParent(AtomOrDescriptorBase* parent) {
  parent_ = parent;
}

std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
AtomOrDescriptorBase::GetChildAtoms() const {
  return child_atoms_;
}

void AtomOrDescriptorBase::AddChildAtom(
    std::unique_ptr<AtomOrDescriptorBase>&& child) {
  assert(GetType() == Type::kAtom);  // Only atoms can have atom children.
  child_atoms_.push_back(std::move(child));
}

std::vector<std::unique_ptr<AtomOrDescriptorBase>> const&
AtomOrDescriptorBase::GetChildDescriptors() const {
  return child_descriptors_;
}

void AtomOrDescriptorBase::AddChildDescriptor(
    std::unique_ptr<AtomOrDescriptorBase>&& child) {
  child_descriptors_.push_back(std::move(child));
}

void AtomOrDescriptorBase::SetAp4Atom(AP4_Atom* ap4_atom) {
  ap4_atom_ = ap4_atom;
}

AP4_Atom* AtomOrDescriptorBase::GetAp4Atom() const { return ap4_atom_; }

Atom::Atom(char const* name, uint32_t header_size, uint64_t size)
    : AtomOrDescriptorBase(name, header_size, size) {}

AtomOrDescriptorBase::Type Atom::GetType() const {
  return AtomOrDescriptorBase::Type::kAtom;
}

Descriptor::Descriptor(char const* name, uint32_t header_size, uint64_t size)
    : AtomOrDescriptorBase(name, header_size, size) {}

AtomOrDescriptorBase::Type Descriptor::GetType() const {
  return AtomOrDescriptorBase::Type::kDescriptor;
}

}  // namespace mp4_manipulator
