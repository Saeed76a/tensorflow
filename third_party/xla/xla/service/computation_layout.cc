/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "xla/service/computation_layout.h"

#include <string>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "xla/printer.h"
#include "xla/types.h"

namespace xla {

ComputationLayout::ComputationLayout(const ProgramShape& program_shape,
                                     bool ignore_layouts)
    : result_layout_(program_shape.result()) {
  for (auto& shape : program_shape.parameters()) {
    parameter_layouts_.emplace_back(shape);
  }
  if (ignore_layouts) {
    SetToDefaultLayout();
  }
}

void ComputationLayout::SetToDefaultLayout() {
  for (auto& parameter_layout : parameter_layouts_) {
    parameter_layout.SetToDefaultLayout();
  }
  result_layout_.SetToDefaultLayout();
}
bool ComputationLayout::LayoutIsSet() const {
  return absl::c_all_of(parameter_layouts_,
                        [](const ShapeLayout& s) { return s.LayoutIsSet(); }) &&
         result_layout_.LayoutIsSet();
}

bool ComputationLayout::AnyLayoutSet() const {
  return absl::c_any_of(parameter_layouts_,
                        [](const ShapeLayout& s) { return s.LayoutIsSet(); }) ||
         result_layout_.LayoutIsSet();
}

void ComputationLayout::Print(Printer* printer) const {
  printer->Append("(");
  if (!parameter_layouts_.empty()) {
    parameter_layouts_[0].Print(printer);
    for (int i = 1; i < parameter_layouts_.size(); ++i) {
      if (i % 5 == 0) {
        printer->Append(absl::StrFormat(", /*index=%lld*/", i));
      } else {
        printer->Append(", ");
      }
      parameter_layouts_[i].Print(printer);
    }
  }
  printer->Append(")->");
  result_layout_.Print(printer);
}

std::string ComputationLayout::ToString() const {
  StringPrinter printer;
  Print(&printer);
  return std::move(printer).ToString();
}

ProgramShape ComputationLayout::ComputeProgramShape() const {
  ProgramShape program_shape;
  for (int64_t i = 0; i < parameter_layouts_.size(); ++i) {
    *program_shape.add_parameters() = parameter_layouts_[i].shape();
    *program_shape.add_parameter_names() = absl::StrCat("p", i);
  }
  *program_shape.mutable_result() = result_layout_.shape();
  return program_shape;
}

bool ComputationLayout::operator==(const ComputationLayout& other) const {
  return result_layout() == other.result_layout() &&
         parameter_layouts() == other.parameter_layouts();
}

bool ComputationLayout::operator!=(const ComputationLayout& other) const {
  return result_layout() != other.result_layout() ||
         parameter_layouts() != other.parameter_layouts();
}

}  // namespace xla
