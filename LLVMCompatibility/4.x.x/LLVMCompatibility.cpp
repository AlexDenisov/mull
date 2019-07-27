#include "LLVMCompatibility.h"

#include <llvm/Bitcode/BitcodeReader.h>

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) { return symbol.getAddress(); }

JITSymbolFlags
JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return JITSymbolFlags::fromObjectSymbol(symbol);
}

object::OwningBinary<object::ObjectFile>
compileModule(orc::SimpleCompiler &compiler, llvm::Module &module) {
  auto objectFile = compiler(module);
  return std::move(objectFile);
}

std::unique_ptr<Module> parseBitcode(MemoryBufferRef bufferRef,
                                     LLVMContext &context) {
  auto module = parseBitcodeFile(bufferRef, context);
  if (!module) {
    logAllUnhandledErrors(module.takeError(), errs(),
                          "\nparseBitcodeFile failed: ");
    return std::unique_ptr<Module>();
  }

  return std::move(module.get());
}

void setVersionPrinter(void (*oldPrinter)(),
                       void (*newPrinter)(raw_ostream &)) {
  llvm::cl::SetVersionPrinter(oldPrinter);
}

Function *GetOrInsertFunction(Module &module, const std::string &name,
                              FunctionType *functionType) {
  return cast<Function>(module.getOrInsertFunction(name, functionType));
}

} // namespace llvm_compat
