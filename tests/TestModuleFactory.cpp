#include "TestModuleFactory.h"
#include "LLVMCompatibility.h"
#include "mull/Logger.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sys/param.h>
#include <unistd.h>

namespace mull {

std::unique_ptr<MullModule> loadModuleFromIR(const char *path,
                                             LLVMContext &context) {
  ///  I cannot recall why it is implemented the way it is implemented
  ///  Sorry about that
  std::ifstream file(path);
  std::string str;
  std::string file_contents;
  while (std::getline(file, str)) {
    file_contents += str;
    file_contents.push_back('\n');
  }

  SMDiagnostic error;
  auto module = parseAssemblyString(file_contents, error, context);

  /// FIXME: is there another way to check for errors?
  if (!error.getMessage().empty()) {
    error.print("test", dbgs());
  }

  assert(module && "Expected module to be parsed correctly");

  assert(!llvm::verifyModule(*module, &dbgs()));

  return make_unique<MullModule>(
      std::move(module), std::unique_ptr<llvm::MemoryBuffer>(), "fake_path");
}

} // namespace mull