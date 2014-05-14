/*
 *
 * Copyright 2013 Samsung Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define _ARMV7_A_NEON
#define TGT_LIR ArmLIR

#include "Dalvik.h"
#include "compiler/CompilerInternals.h"
#include "ArmLIR.h"
#include "Codegen.h"
#include "compiler/codegen/Ralloc.h"
#include "compiler/codegen/arm/armv7-a-neon/ArchVariant.h"

#include "compiler/codegen/arm/CodegenCommon-static.cpp"
#include "compiler/codegen/arm/Thumb2/Factory.cpp"
#include "compiler/codegen/CodegenFactory.cpp"
#include "compiler/codegen/arm/ArchFactory.cpp"

#ifndef SWE_DVM_OPT
#error "Strange build configuration!!!"
#endif

#include "CodegenDriverOptimizations.cpp"
#include "compiler/FrontEndOptimizations.cpp"
#include "BackEndOptimizations.cpp"
