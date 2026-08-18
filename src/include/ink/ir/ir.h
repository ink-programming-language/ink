#ifndef INK_IR_IR_H
#define INK_IR_IR_H

#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/compilation/compilation_session.h"
#include "ink/ir/compilation/module_name.h"
#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/basic_block.h"
#include "ink/ir/model/attribute.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/constant_pool.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/function.h"
#include "ink/ir/model/global_variable.h"
#include "ink/ir/model/id.h"
#include "ink/ir/model/import_info.h"
#include "ink/ir/model/module.h"
#include "ink/ir/model/name.h"
#include "ink/ir/model/operand.h"
#include "ink/ir/model/parameter.h"
#include "ink/ir/model/struct_type.h"
#include "ink/ir/model/type.h"
#include "ink/ir/model/value.h"
#include "ink/ir/model/value_handle.h"

#endif
