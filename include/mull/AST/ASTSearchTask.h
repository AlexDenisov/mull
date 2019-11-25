#pragma once

#include "mull/AST/ASTTypes.h"
#include "mull/Bitcode.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/Filters/FilePathFilter.h"

#include <unordered_map>
#include <vector>

namespace mull {

class progress_counter;

class ASTSearchTask {
public:
  using In = const std::vector<std::unique_ptr<mull::Bitcode>>;
  using Out = std::vector<std::pair<SourceFilePath, SingleASTUnitMutations>>;
  using iterator = In::const_iterator;

  ASTSearchTask(mull::ASTStorage &astStorage, mull::FilePathFilter &pathFilter,
                mull::TraverseMask traverseMask);

  void operator()(iterator begin, iterator end, Out &storage,
                  mull::progress_counter &counter);

private:
  mull::ASTStorage &astStorage;
  mull::FilePathFilter &pathFilter;
  mull::TraverseMask traverseMask;
};

} // namespace mull
