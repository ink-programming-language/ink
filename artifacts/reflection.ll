; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%ink.class.4 = type { i32, i32, i1, [2 x i1], i32 }

@diagnostic = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.1 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.2 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.3 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.4 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.5 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.6 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.7 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.8 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.9 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.10 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.11 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.12 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.13 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.14 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.15 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.16 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.17 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@reflection.entries = private constant [1 x i32] [i32 2]
@reflection.desc = private constant { i32, i64, ptr } { i32 1, i64 1, ptr @reflection.entries }
@reflection.name = private constant [1 x i8] zeroinitializer
@reflection.desc.18 = private constant { i32, i32 } { i32 32, i32 1 }
@reflection.name.19 = private constant [4 x i8] c"i32\00"
@reflection.desc.20 = private constant { i32, i32 } { i32 3, i32 1 }
@reflection.name.21 = private constant [1 x i8] zeroinitializer
@reflection.name.22 = private constant [6 x i8] c"Value\00"
@reflection.name.24 = private constant [5 x i8] c"Step\00"
@reflection.name.26 = private constant [5 x i8] c"Flag\00"
@reflection.name.28 = private constant [6 x i8] c"Flags\00"
@reflection.name.30 = private constant [7 x i8] c"Secret\00"
@reflection.name.32 = private constant [8 x i8] c"advance\00"
@reflection.name.34 = private constant [5 x i8] c"copy\00"
@reflection.entries.35 = private constant [5 x { ptr, i32, i64, i32, ptr }] [{ ptr, i32, i64, i32, ptr } { ptr @reflection.name.22, i32 1, i64 0, i32 0, ptr @__ink_reflection_call }, { ptr, i32, i64, i32, ptr } { ptr @reflection.name.24, i32 1, i64 4, i32 0, ptr @__ink_reflection_call.23 }, { ptr, i32, i64, i32, ptr } { ptr @reflection.name.26, i32 4, i64 8, i32 0, ptr @__ink_reflection_call.25 }, { ptr, i32, i64, i32, ptr } { ptr @reflection.name.28, i32 5, i64 9, i32 0, ptr @__ink_reflection_call.27 }, { ptr, i32, i64, i32, ptr } { ptr @reflection.name.30, i32 1, i64 12, i32 1, ptr @__ink_reflection_call.29 }]
@reflection.entries.36 = private constant [2 x { ptr, i32, i32, i32, ptr }] [{ ptr, i32, i32, i32, ptr } { ptr @reflection.name.32, i32 0, i32 0, i32 1, ptr @__ink_reflection_call.31 }, { ptr, i32, i32, i32, ptr } { ptr @reflection.name.34, i32 9, i32 0, i32 1, ptr @__ink_reflection_call.33 }]
@reflection.desc.37 = private constant { i64, ptr, i64, ptr } { i64 5, ptr @reflection.entries.35, i64 2, ptr @reflection.entries.36 }
@reflection.name.38 = private constant [13 x i8] c"main.Counter\00"
@reflection.name.39 = private constant [5 x i8] c"bool\00"
@reflection.desc.40 = private constant { i32, i64 } { i32 4, i64 2 }
@reflection.name.41 = private constant [1 x i8] zeroinitializer
@reflection.entries.42 = private constant [0 x i32] zeroinitializer
@reflection.desc.43 = private constant { i32, i64, ptr } { i32 1, i64 0, ptr @reflection.entries.42 }
@reflection.name.44 = private constant [1 x i8] zeroinitializer
@reflection.entries.45 = private constant [0 x i32] zeroinitializer
@reflection.desc.46 = private constant { i32, i64, ptr } { i32 4, i64 0, ptr @reflection.entries.45 }
@reflection.name.47 = private constant [1 x i8] zeroinitializer
@reflection.entries.48 = private constant [0 x i32] zeroinitializer
@reflection.desc.49 = private constant { i32, i64, ptr } { i32 5, i64 0, ptr @reflection.entries.48 }
@reflection.name.50 = private constant [1 x i8] zeroinitializer
@reflection.entries.51 = private constant [1 x i32] [i32 2]
@reflection.desc.52 = private constant { i32, i64, ptr } { i32 3, i64 1, ptr @reflection.entries.51 }
@reflection.name.53 = private constant [1 x i8] zeroinitializer
@reflection.name.54 = private constant [5 x i8] c"main\00"
@reflection.entries.55 = private constant [10 x { i32, ptr, i64, i64, ptr }] [{ i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name, i64 8, i64 8, ptr @reflection.desc }, { i32, ptr, i64, i64, ptr } { i32 3, ptr @reflection.name.19, i64 4, i64 4, ptr @reflection.desc.18 }, { i32, ptr, i64, i64, ptr } { i32 6, ptr @reflection.name.21, i64 8, i64 8, ptr @reflection.desc.20 }, { i32, ptr, i64, i64, ptr } { i32 9, ptr @reflection.name.38, i64 16, i64 4, ptr @reflection.desc.37 }, { i32, ptr, i64, i64, ptr } { i32 2, ptr @reflection.name.39, i64 1, i64 1, ptr null }, { i32, ptr, i64, i64, ptr } { i32 8, ptr @reflection.name.41, i64 2, i64 1, ptr @reflection.desc.40 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.44, i64 8, i64 8, ptr @reflection.desc.43 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.47, i64 8, i64 8, ptr @reflection.desc.46 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.50, i64 8, i64 8, ptr @reflection.desc.49 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.53, i64 8, i64 8, ptr @reflection.desc.52 }]
@reflection.desc.56 = private constant { i32, ptr, i64, ptr } { i32 1, ptr @reflection.name.54, i64 10, ptr @reflection.entries.55 }

define internal i32 @__ink_fn_0(ptr %0) {
prologue:
  %value = alloca ptr, align 8
  %initialized = alloca i1, align 1
  store i1 false, ptr %initialized, align 1
  %value1 = alloca ptr, align 8
  %initialized2 = alloca i1, align 1
  store i1 false, ptr %initialized2, align 1
  %value3 = alloca i32, align 4
  %initialized4 = alloca i1, align 1
  store i1 false, ptr %initialized4, align 1
  %value5 = alloca ptr, align 8
  %initialized6 = alloca i1, align 1
  store i1 false, ptr %initialized6, align 1
  %value7 = alloca i32, align 4
  %initialized8 = alloca i1, align 1
  store i1 false, ptr %initialized8, align 1
  %value9 = alloca i32, align 4
  %initialized10 = alloca i1, align 1
  store i1 false, ptr %initialized10, align 1
  %value11 = alloca ptr, align 8
  %initialized12 = alloca i1, align 1
  store i1 false, ptr %initialized12, align 1
  %value13 = alloca i32, align 4
  %initialized14 = alloca i1, align 1
  store i1 false, ptr %initialized14, align 1
  br label %block

block:                                            ; preds = %prologue
  %1 = getelementptr i8, ptr %0, i64 0
  store ptr %1, ptr %value, align 8
  store i1 true, ptr %initialized, align 1
  %2 = getelementptr i8, ptr %0, i64 0
  store ptr %2, ptr %value1, align 8
  store i1 true, ptr %initialized2, align 1
  %3 = load i1, ptr %initialized2, align 1
  br i1 %3, label %valid, label %invalid

valid:                                            ; preds = %block
  %4 = load ptr, ptr %value1, align 8
  %5 = load i32, ptr %4, align 1
  store i32 %5, ptr %value3, align 4
  store i1 true, ptr %initialized4, align 1
  %6 = getelementptr i8, ptr %0, i64 4
  store ptr %6, ptr %value5, align 8
  store i1 true, ptr %initialized6, align 1
  %7 = load i1, ptr %initialized6, align 1
  br i1 %7, label %valid15, label %invalid16

invalid:                                          ; preds = %block
  %8 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic, i64 75)
  unreachable

valid15:                                          ; preds = %valid
  %9 = load ptr, ptr %value5, align 8
  %10 = load i32, ptr %9, align 1
  store i32 %10, ptr %value7, align 4
  store i1 true, ptr %initialized8, align 1
  %11 = load i1, ptr %initialized4, align 1
  br i1 %11, label %valid17, label %invalid18

invalid16:                                        ; preds = %valid
  %12 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.1, i64 75)
  unreachable

valid17:                                          ; preds = %valid15
  %13 = load i32, ptr %value3, align 4
  %14 = load i1, ptr %initialized8, align 1
  br i1 %14, label %valid19, label %invalid20

invalid18:                                        ; preds = %valid15
  %15 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.2, i64 75)
  unreachable

valid19:                                          ; preds = %valid17
  %16 = load i32, ptr %value7, align 4
  %17 = add i32 %13, %16
  store i32 %17, ptr %value9, align 4
  store i1 true, ptr %initialized10, align 1
  %18 = load i1, ptr %initialized, align 1
  br i1 %18, label %valid21, label %invalid22

invalid20:                                        ; preds = %valid17
  %19 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.3, i64 75)
  unreachable

valid21:                                          ; preds = %valid19
  %20 = load ptr, ptr %value, align 8
  %21 = load i1, ptr %initialized10, align 1
  br i1 %21, label %valid23, label %invalid24

invalid22:                                        ; preds = %valid19
  %22 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.4, i64 75)
  unreachable

valid23:                                          ; preds = %valid21
  %23 = load i32, ptr %value9, align 4
  store i32 %23, ptr %20, align 1
  %24 = getelementptr i8, ptr %0, i64 0
  store ptr %24, ptr %value11, align 8
  store i1 true, ptr %initialized12, align 1
  %25 = load i1, ptr %initialized12, align 1
  br i1 %25, label %valid25, label %invalid26

invalid24:                                        ; preds = %valid21
  %26 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.5, i64 75)
  unreachable

valid25:                                          ; preds = %valid23
  %27 = load ptr, ptr %value11, align 8
  %28 = load i32, ptr %27, align 1
  store i32 %28, ptr %value13, align 4
  store i1 true, ptr %initialized14, align 1
  %29 = load i1, ptr %initialized14, align 1
  br i1 %29, label %valid27, label %invalid28

invalid26:                                        ; preds = %valid23
  %30 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.6, i64 75)
  unreachable

valid27:                                          ; preds = %valid25
  %31 = load i32, ptr %value13, align 4
  ret i32 %31

invalid28:                                        ; preds = %valid25
  %32 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.7, i64 75)
  unreachable
}

define internal %ink.class.4 @__ink_fn_1(ptr %0) {
prologue:
  %value = alloca ptr, align 8
  %initialized = alloca i1, align 1
  store i1 false, ptr %initialized, align 1
  %value1 = alloca i32, align 4
  %initialized2 = alloca i1, align 1
  store i1 false, ptr %initialized2, align 1
  %value3 = alloca ptr, align 8
  %initialized4 = alloca i1, align 1
  store i1 false, ptr %initialized4, align 1
  %value5 = alloca i32, align 4
  %initialized6 = alloca i1, align 1
  store i1 false, ptr %initialized6, align 1
  %value7 = alloca i1, align 1
  %initialized8 = alloca i1, align 1
  store i1 false, ptr %initialized8, align 1
  %value9 = alloca [2 x i1], align 1
  %initialized10 = alloca i1, align 1
  store i1 false, ptr %initialized10, align 1
  %value11 = alloca i32, align 4
  %initialized12 = alloca i1, align 1
  store i1 false, ptr %initialized12, align 1
  %value13 = alloca %ink.class.4, align 8
  %initialized14 = alloca i1, align 1
  store i1 false, ptr %initialized14, align 1
  br label %block

block:                                            ; preds = %prologue
  %1 = getelementptr i8, ptr %0, i64 0
  store ptr %1, ptr %value, align 8
  store i1 true, ptr %initialized, align 1
  %2 = load i1, ptr %initialized, align 1
  br i1 %2, label %valid, label %invalid

valid:                                            ; preds = %block
  %3 = load ptr, ptr %value, align 8
  %4 = load i32, ptr %3, align 1
  store i32 %4, ptr %value1, align 4
  store i1 true, ptr %initialized2, align 1
  %5 = getelementptr i8, ptr %0, i64 4
  store ptr %5, ptr %value3, align 8
  store i1 true, ptr %initialized4, align 1
  %6 = load i1, ptr %initialized4, align 1
  br i1 %6, label %valid15, label %invalid16

invalid:                                          ; preds = %block
  %7 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.8, i64 75)
  unreachable

valid15:                                          ; preds = %valid
  %8 = load ptr, ptr %value3, align 8
  %9 = load i32, ptr %8, align 1
  store i32 %9, ptr %value5, align 4
  store i1 true, ptr %initialized6, align 1
  %10 = call i1 @__ink_fn_7()
  store i1 %10, ptr %value7, align 1
  store i1 true, ptr %initialized8, align 1
  %11 = call [2 x i1] @__ink_fn_8()
  store [2 x i1] %11, ptr %value9, align 1
  store i1 true, ptr %initialized10, align 1
  %12 = call i32 @__ink_fn_9()
  store i32 %12, ptr %value11, align 4
  store i1 true, ptr %initialized12, align 1
  %13 = load i1, ptr %initialized2, align 1
  br i1 %13, label %valid17, label %invalid18

invalid16:                                        ; preds = %valid
  %14 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.9, i64 75)
  unreachable

valid17:                                          ; preds = %valid15
  %15 = load i32, ptr %value1, align 4
  %16 = insertvalue %ink.class.4 zeroinitializer, i32 %15, 0
  %17 = load i1, ptr %initialized6, align 1
  br i1 %17, label %valid19, label %invalid20

invalid18:                                        ; preds = %valid15
  %18 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.10, i64 75)
  unreachable

valid19:                                          ; preds = %valid17
  %19 = load i32, ptr %value5, align 4
  %20 = insertvalue %ink.class.4 %16, i32 %19, 1
  %21 = load i1, ptr %initialized8, align 1
  br i1 %21, label %valid21, label %invalid22

invalid20:                                        ; preds = %valid17
  %22 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.11, i64 75)
  unreachable

valid21:                                          ; preds = %valid19
  %23 = load i1, ptr %value7, align 1
  %24 = insertvalue %ink.class.4 %20, i1 %23, 2
  %25 = load i1, ptr %initialized10, align 1
  br i1 %25, label %valid23, label %invalid24

invalid22:                                        ; preds = %valid19
  %26 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.12, i64 75)
  unreachable

valid23:                                          ; preds = %valid21
  %27 = load [2 x i1], ptr %value9, align 1
  %28 = insertvalue %ink.class.4 %24, [2 x i1] %27, 3
  %29 = load i1, ptr %initialized12, align 1
  br i1 %29, label %valid25, label %invalid26

invalid24:                                        ; preds = %valid21
  %30 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.13, i64 75)
  unreachable

valid25:                                          ; preds = %valid23
  %31 = load i32, ptr %value11, align 4
  %32 = insertvalue %ink.class.4 %28, i32 %31, 4
  store %ink.class.4 %32, ptr %value13, align 4
  store i1 true, ptr %initialized14, align 1
  %33 = load i1, ptr %initialized14, align 1
  br i1 %33, label %valid27, label %invalid28

invalid26:                                        ; preds = %valid23
  %34 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.14, i64 75)
  unreachable

valid27:                                          ; preds = %valid25
  %35 = load %ink.class.4, ptr %value13, align 4
  ret %ink.class.4 %35

invalid28:                                        ; preds = %valid25
  %36 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.15, i64 75)
  unreachable
}

declare i32 @inkReflectionNext()

declare i32 @inkReflectionProbe()

define internal i32 @__ink_fn_4() {
prologue:
  %value = alloca i32, align 4
  %initialized = alloca i1, align 1
  store i1 false, ptr %initialized, align 1
  br label %block

block:                                            ; preds = %prologue
  %0 = call i32 @inkReflectionProbe()
  store i32 %0, ptr %value, align 4
  store i1 true, ptr %initialized, align 1
  %1 = load i1, ptr %initialized, align 1
  br i1 %1, label %valid, label %invalid

valid:                                            ; preds = %block
  %2 = load i32, ptr %value, align 4
  ret i32 %2

invalid:                                          ; preds = %block
  %3 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.16, i64 75)
  unreachable
}

define internal i32 @__ink_fn_5() {
prologue:
  %value = alloca i32, align 4
  %initialized = alloca i1, align 1
  store i1 false, ptr %initialized, align 1
  br label %block

block:                                            ; preds = %prologue
  %0 = call i32 @inkReflectionNext()
  store i32 %0, ptr %value, align 4
  store i1 true, ptr %initialized, align 1
  %1 = load i1, ptr %initialized, align 1
  br i1 %1, label %valid, label %invalid

valid:                                            ; preds = %block
  %2 = load i32, ptr %value, align 4
  ret i32 %2

invalid:                                          ; preds = %block
  %3 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.17, i64 75)
  unreachable
}

define internal i32 @__ink_fn_6() {
prologue:
  br label %block

block:                                            ; preds = %prologue
  ret i32 2
}

define internal i1 @__ink_fn_7() {
prologue:
  br label %block

block:                                            ; preds = %prologue
  ret i1 true
}

define internal [2 x i1] @__ink_fn_8() {
prologue:
  br label %block

block:                                            ; preds = %prologue
  ret [2 x i1] [i1 true, i1 false]
}

define internal i32 @__ink_fn_9() {
prologue:
  br label %block

block:                                            ; preds = %prologue
  ret i32 7
}

; Function Attrs: cold noreturn
declare void @ink_aot_panic(ptr, i64) #0

declare i32 @fflush(ptr)

define private void @__ink_reflection_call(ptr %0, ptr %1) {
entry:
  %2 = call i32 @__ink_fn_5()
  store i32 %2, ptr %0, align 1
  ret void
}

define private void @__ink_reflection_call.23(ptr %0, ptr %1) {
entry:
  %2 = call i32 @__ink_fn_6()
  store i32 %2, ptr %0, align 1
  ret void
}

define private void @__ink_reflection_call.25(ptr %0, ptr %1) {
entry:
  %2 = call i1 @__ink_fn_7()
  %3 = zext i1 %2 to i8
  store i8 %3, ptr %0, align 1
  ret void
}

define private void @__ink_reflection_call.27(ptr %0, ptr %1) {
entry:
  %2 = call [2 x i1] @__ink_fn_8()
  %3 = getelementptr i8, ptr %0, i64 0
  %4 = extractvalue [2 x i1] %2, 0
  %5 = zext i1 %4 to i8
  store i8 %5, ptr %3, align 1
  %6 = getelementptr i8, ptr %0, i64 1
  %7 = extractvalue [2 x i1] %2, 1
  %8 = zext i1 %7 to i8
  store i8 %8, ptr %6, align 1
  ret void
}

define private void @__ink_reflection_call.29(ptr %0, ptr %1) {
entry:
  %2 = call i32 @__ink_fn_9()
  store i32 %2, ptr %0, align 1
  ret void
}

define private void @__ink_reflection_call.31(ptr %0, ptr %1) {
entry:
  %2 = getelementptr ptr, ptr %1, i64 0
  %3 = load ptr, ptr %2, align 8
  %4 = load ptr, ptr %3, align 1
  %5 = call i32 @__ink_fn_0(ptr %4)
  store i32 %5, ptr %0, align 1
  ret void
}

define private void @__ink_reflection_call.33(ptr %0, ptr %1) {
entry:
  %2 = getelementptr ptr, ptr %1, i64 0
  %3 = load ptr, ptr %2, align 8
  %4 = load ptr, ptr %3, align 1
  %5 = call %ink.class.4 @__ink_fn_1(ptr %4)
  %6 = getelementptr i8, ptr %0, i64 0
  %7 = extractvalue %ink.class.4 %5, 0
  store i32 %7, ptr %6, align 1
  %8 = getelementptr i8, ptr %0, i64 4
  %9 = extractvalue %ink.class.4 %5, 1
  store i32 %9, ptr %8, align 1
  %10 = getelementptr i8, ptr %0, i64 8
  %11 = extractvalue %ink.class.4 %5, 2
  %12 = zext i1 %11 to i8
  store i8 %12, ptr %10, align 1
  %13 = getelementptr i8, ptr %0, i64 9
  %14 = extractvalue %ink.class.4 %5, 3
  %15 = getelementptr i8, ptr %13, i64 0
  %16 = extractvalue [2 x i1] %14, 0
  %17 = zext i1 %16 to i8
  store i8 %17, ptr %15, align 1
  %18 = getelementptr i8, ptr %13, i64 1
  %19 = extractvalue [2 x i1] %14, 1
  %20 = zext i1 %19 to i8
  store i8 %20, ptr %18, align 1
  %21 = getelementptr i8, ptr %0, i64 12
  %22 = extractvalue %ink.class.4 %5, 4
  store i32 %22, ptr %21, align 1
  ret void
}

define ptr @ink_reflection_6d61696e() {
entry:
  ret ptr @reflection.desc.56
}

define i32 @main() {
entry:
  %0 = call i32 @__ink_fn_4()
  ret i32 %0
}

attributes #0 = { cold noreturn }
