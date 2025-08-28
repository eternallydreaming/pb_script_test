#include "registry.h"
#include "lexer.h"
#include "type_def.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool match(Lexer *lexer, TokenType what) {
  Token token = lexer_peek(lexer);
  if (token.type != what)
    return false;
  lexer_advance(lexer);
  return true;
}

static void assert_not_defined(const Registry *registry, const char *name) {
  for (size_t i = 0; i < registry->native_fns_num; i++) {
    if (strcmp(registry->native_fns[i].name, name) == 0)
      assert(0 && "a native function with that name is already defined");
  }
}

Registry new_registry() {
  return (Registry){
      .native_fns_num = 0,
      .native_fns = NULL,
  };
}

void delete_registry(Registry *registry) {
  for (size_t i = 0; i < registry->native_fns_num; i++) {
    NativeFn *native_fn = &registry->native_fns[i];
    free(native_fn->arg_types);
    free(native_fn->name);
  }
  free(registry->native_fns);
}

void register_native_fn(Registry *registry, const char *sig, NativeFnPtr ptr) {
  Lexer lexer = new_lexer(sig);
  TypeDef return_type = parse_type_def(&lexer);
  if (return_type.value == ValueType_Error)
    assert(0 && "bad return type");

  Token name_token = lexer_peek(&lexer);
  if (name_token.type != TokenType_Identifier)
    assert(0 && "expected identifier after return type");
  lexer_advance(&lexer);

  char *name = strndup(name_token.text.start, name_token.text.len);
  assert(name != NULL);
  assert_not_defined(registry, name);

  NativeFn native_fn = {
      .return_type = return_type,
      .name = name,

      .args_num = 0,
      .arg_types = NULL,
      .variadic = false,

      .ptr = ptr,
  };

  // arguments
  if (!match(&lexer, TokenType_LParen))
    assert(0 && "expected '(' after identifier");
  while (lexer_peek(&lexer).type != TokenType_Eof &&
         lexer_peek(&lexer).type != TokenType_RParen) {
    if (native_fn.variadic)
      assert(0 && "variadic arguments must be at the end of the argument list");

    if (native_fn.args_num != 0 && !match(&lexer, TokenType_Comma))
      assert(0 && "expected ',' after type");

    if (match(&lexer, TokenType_TripleDot)) {
      native_fn.variadic = true;
      continue;
    }

    TypeDef arg_type = parse_type_def(&lexer);
    if (arg_type.value == ValueType_Error)
      assert(0 && "bad argument type");
    if (arg_type.value == ValueType_Void)
      assert(0 && "can't use void as an argument type");

    // add to list
    size_t new_size = (native_fn.args_num + 1) * sizeof(*native_fn.arg_types);
    native_fn.arg_types = realloc(native_fn.arg_types, new_size);
    assert(native_fn.arg_types != NULL);

    native_fn.arg_types[native_fn.args_num++] = arg_type;
  }
  if (!match(&lexer, TokenType_RParen))
    assert(0 && "expected ')' to close '('");

  // let's not forget to add the function itself
  size_t new_size =
      (registry->native_fns_num + 1) * sizeof(*registry->native_fns);
  registry->native_fns = realloc(registry->native_fns, new_size);
  assert(registry->native_fns != NULL);

  registry->native_fns[registry->native_fns_num++] = native_fn;
}