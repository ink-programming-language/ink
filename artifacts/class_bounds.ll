; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@diagnostic = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.1 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.2 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.3 = private unnamed_addr constant [73 x i8] c"inkc: error[INK-E0025]: AOT execution failed: array index out of bounds\0A\00", align 1
@diagnostic.4 = private unnamed_addr constant [73 x i8] c"inkc: error[INK-E0025]: AOT execution failed: array index out of bounds\0A\00", align 1
@diagnostic.5 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.6 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@diagnostic.7 = private unnamed_addr constant [76 x i8] c"inkc: error[INK-E0008]: AOT execution failed: runtime value is unavailable\0A\00", align 1
@reflection.entries = private constant [0 x i32] zeroinitializer
@reflection.desc = private constant { i32, i64, ptr } { i32 1, i64 0, ptr @reflection.entries }
@reflection.name = private constant [1 x i8] zeroinitializer
@reflection.desc.8 = private constant { i32, i32 } { i32 32, i32 1 }
@reflection.name.9 = private constant [4 x i8] c"i32\00"
@reflection.name.10 = private constant [6 x i8] c"Value\00"
@reflection.entries.11 = private constant [1 x { ptr, i32, i64, i32, ptr }] [{ ptr, i32, i64, i32, ptr } { ptr @reflection.name.10, i32 1, i64 0, i32 0, ptr null }]
@reflection.entries.12 = private constant [0 x { ptr, i32, i32, i32, ptr }] zeroinitializer
@reflection.desc.13 = private constant { i64, ptr, i64, ptr } { i64 1, ptr @reflection.entries.11, i64 0, ptr @reflection.entries.12 }
@reflection.name.14 = private constant [9 x i8] c"main.Box\00"
@reflection.name.15 = private constant [5 x i8] c"main\00"
@reflection.entries.16 = private constant [3 x { i32, ptr, i64, i64, ptr }] [{ i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name, i64 8, i64 8, ptr @reflection.desc }, { i32, ptr, i64, i64, ptr } { i32 3, ptr @reflection.name.9, i64 4, i64 4, ptr @reflection.desc.8 }, { i32, ptr, i64, i64, ptr } { i32 9, ptr @reflection.name.14, i64 4, i64 4, ptr @reflection.desc.13 }]
@reflection.desc.17 = private constant { i32, ptr, i64, ptr } { i32 1, ptr @reflection.name.15, i64 3, ptr @reflection.entries.16 }

define internal i32 @__ink_fn_0() {
prologue:
  br label %block

block:                                            ; preds = %prologue
  ret i32 2
}

define internal i32 @__ink_fn_1() {
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
  %value5 = alloca ptr, align 8
  %initialized6 = alloca i1, align 1
  store i1 false, ptr %initialized6, align 1
  %value7 = alloca i32, align 4
  %initialized8 = alloca i1, align 1
  store i1 false, ptr %initialized8, align 1
  br label %block

block:                                            ; preds = %prologue
  %object = alloca i8, i64 4, align 4
  store ptr %object, ptr %value, align 8
  store i1 true, ptr %initialized, align 1
  %0 = load i1, ptr %initialized, align 1
  br i1 %0, label %valid, label %invalid

valid:                                            ; preds = %block
  %1 = load ptr, ptr %value, align 8
  %2 = getelementptr i8, ptr %1, i64 0
  %3 = getelementptr i8, ptr %2, i64 0
  store i32 42, ptr %3, align 1
  %4 = call i32 @__ink_fn_0()
  store i32 %4, ptr %value1, align 4
  store i1 true, ptr %initialized2, align 1
  %5 = load i1, ptr %initialized, align 1
  br i1 %5, label %valid9, label %invalid10

invalid:                                          ; preds = %block
  %6 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic, i64 75)
  unreachable

valid9:                                           ; preds = %valid
  %7 = load ptr, ptr %value, align 8
  %8 = load i1, ptr %initialized2, align 1
  br i1 %8, label %valid11, label %invalid12

invalid10:                                        ; preds = %valid
  %9 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.1, i64 75)
  unreachable

valid11:                                          ; preds = %valid9
  %10 = load i32, ptr %value1, align 4
  %11 = icmp sge i32 %10, 0
  br i1 %11, label %valid13, label %invalid14

invalid12:                                        ; preds = %valid9
  %12 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.2, i64 75)
  unreachable

valid13:                                          ; preds = %valid11
  %13 = zext i32 %10 to i64
  %14 = icmp ult i64 %13, 1
  br i1 %14, label %valid15, label %invalid16

invalid14:                                        ; preds = %valid11
  %15 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.3, i64 72)
  unreachable

valid15:                                          ; preds = %valid13
  %16 = zext i32 %10 to i64
  %17 = mul i64 %16, 4
  %18 = getelementptr i8, ptr %7, i64 %17
  store ptr %18, ptr %value3, align 8
  store i1 true, ptr %initialized4, align 1
  %19 = load i1, ptr %initialized4, align 1
  br i1 %19, label %valid17, label %invalid18

invalid16:                                        ; preds = %valid13
  %20 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.4, i64 72)
  unreachable

valid17:                                          ; preds = %valid15
  %21 = load ptr, ptr %value3, align 8
  %22 = getelementptr i8, ptr %21, i64 0
  store ptr %22, ptr %value5, align 8
  store i1 true, ptr %initialized6, align 1
  %23 = load i1, ptr %initialized6, align 1
  br i1 %23, label %valid19, label %invalid20

invalid18:                                        ; preds = %valid15
  %24 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.5, i64 75)
  unreachable

valid19:                                          ; preds = %valid17
  %25 = load ptr, ptr %value5, align 8
  %26 = load i32, ptr %25, align 1
  store i32 %26, ptr %value7, align 4
  store i1 true, ptr %initialized8, align 1
  %27 = load i1, ptr %initialized8, align 1
  br i1 %27, label %valid21, label %invalid22

invalid20:                                        ; preds = %valid17
  %28 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.6, i64 75)
  unreachable

valid21:                                          ; preds = %valid19
  %29 = load i32, ptr %value7, align 4
  ret i32 %29

invalid22:                                        ; preds = %valid19
  %30 = call i32 @fflush(ptr null)
  call void @ink_aot_panic(ptr @diagnostic.7, i64 75)
  unreachable
}

; Function Attrs: cold noreturn
declare void @ink_aot_panic(ptr, i64) #0

declare i32 @fflush(ptr)

define ptr @ink_reflection_6d61696e() {
entry:
  ret ptr @reflection.desc.17
}

define i32 @main() {
entry:
  %0 = call i32 @__ink_fn_1()
  ret i32 %0
}

attributes #0 = { cold noreturn }
