; ModuleID = 'const_method'
source_filename = "const_method"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@reflection.entries = private constant [1 x i32] [i32 2]
@reflection.desc = private constant { i32, i64, ptr } { i32 1, i64 1, ptr @reflection.entries }
@reflection.name = private constant [1 x i8] zeroinitializer
@reflection.desc.17 = private constant { i32, i32 } { i32 32, i32 1 }
@reflection.name.18 = private constant [4 x i8] c"i32\00"
@reflection.desc.19 = private constant { i32, i32 } { i32 3, i32 1 }
@reflection.name.20 = private constant [1 x i8] zeroinitializer
@reflection.name.21 = private constant [2 x i8] c"X\00"
@reflection.name.22 = private constant [5 x i8] c"read\00"
@reflection.name.23 = private constant [9 x i8] c"__init__\00"
@reflection.name.24 = private constant [8 x i8] c"__del__\00"
@reflection.entries.25 = private constant [1 x { ptr, i32, i64, i32, ptr }] [{ ptr, i32, i64, i32, ptr } { ptr @reflection.name.21, i32 1, i64 0, i32 0, ptr @_INK2H179_N10_reflectionI120_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_N1_XS23_B1_iC1_cA1_nL0_Q5_i2_32S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_ }]
@reflection.entries.26 = private constant [3 x { ptr, i32, i32, i32, ptr }] [{ ptr, i32, i32, i32, ptr } { ptr @reflection.name.22, i32 0, i32 0, i32 1, ptr @_INK2H450_N10_reflectionF391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_ }, { ptr, i32, i32, i32, ptr } { ptr @reflection.name.23, i32 5, i32 0, i32 1, ptr @_INK2H461_N10_reflectionF402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_ }, { ptr, i32, i32, i32, ptr } { ptr @reflection.name.24, i32 5, i32 0, i32 1, ptr @_INK2H460_N10_reflectionF401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_ }]
@reflection.desc.27 = private constant { i64, ptr, i64, ptr } { i64 1, ptr @reflection.entries.25, i64 3, ptr @reflection.entries.26 }
@reflection.name.28 = private constant [99 x i8] c"_INK2T89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_\00"
@reflection.entries.29 = private constant [0 x i32] zeroinitializer
@reflection.desc.30 = private constant { i32, i64, ptr } { i32 1, i64 0, ptr @reflection.entries.29 }
@reflection.name.31 = private constant [1 x i8] zeroinitializer
@reflection.entries.32 = private constant [1 x i32] [i32 2]
@reflection.desc.33 = private constant { i32, i64, ptr } { i32 6, i64 1, ptr @reflection.entries.32 }
@reflection.name.34 = private constant [1 x i8] zeroinitializer
@reflection.name.35 = private constant [5 x i8] c"void\00"
@reflection.name.36 = private constant [13 x i8] c"const_method\00"
@reflection.entries.37 = private constant [7 x { i32, ptr, i64, i64, ptr }] [{ i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name, i64 8, i64 8, ptr @reflection.desc }, { i32, ptr, i64, i64, ptr } { i32 3, ptr @reflection.name.18, i64 4, i64 4, ptr @reflection.desc.17 }, { i32, ptr, i64, i64, ptr } { i32 6, ptr @reflection.name.20, i64 8, i64 8, ptr @reflection.desc.19 }, { i32, ptr, i64, i64, ptr } { i32 9, ptr @reflection.name.28, i64 4, i64 4, ptr @reflection.desc.27 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.31, i64 8, i64 8, ptr @reflection.desc.30 }, { i32, ptr, i64, i64, ptr } { i32 7, ptr @reflection.name.34, i64 8, i64 8, ptr @reflection.desc.33 }, { i32, ptr, i64, i64, ptr } { i32 1, ptr @reflection.name.35, i64 0, i64 1, ptr null }]
@reflection.desc.38 = private constant { i32, ptr, i64, ptr } { i32 1, ptr @reflection.name.36, i64 7, ptr @reflection.entries.37 }
@ink.abi = private constant [3126 x i8] c"INKABI1L3113_T22_x86_64-pc-windows-msvcD79_e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128L2998_E142_N128__INK2F118_R84_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_fN4_mainG0_H7_A1_nL0_X0_S23_B1_iC1_cA1_nL0_Q5_i2_32D6_i32 ()E418_N401__INK2F391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32D9_i32 (ptr)E430_N411__INK2F401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_D10_void (ptr)E431_N412__INK2F402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_D10_void (ptr)E144_N130__INK2I120_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_N1_XS23_B1_iC1_cA1_nL0_Q5_i2_32D6_i32 ()E1402_N98__INK2T89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_D1294_L1288_D1_4D1_4L25_E21_N1_Xi2_32D1_0D1_0D1_1L1245_F403__INK2F391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32:0F413__INK2F401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_:0F414__INK2F402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_:0", section ".inkabi"
@llvm.compiler.used = appending global [1 x ptr] [ptr @ink.abi], section "llvm.metadata"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read)
define i32 @_INK2F391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32(ptr readonly captures(none) %0) local_unnamed_addr #0 {
prologue:
  %1 = load i32, ptr %0, align 1
  ret i32 %1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write)
define void @_INK2F402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_(ptr writeonly captures(none) initializes((0, 4)) %0) local_unnamed_addr #1 {
prologue:
  store i32 42, ptr %0, align 1
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @_INK2F401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_(ptr readnone captures(none) %0) local_unnamed_addr #2 {
prologue:
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef i32 @_INK2F118_R84_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_fN4_mainG0_H7_A1_nL0_X0_S23_B1_iC1_cA1_nL0_Q5_i2_32() local_unnamed_addr #2 {
prologue:
  ret i32 42
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef i32 @_INK2I120_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_N1_XS23_B1_iC1_cA1_nL0_Q5_i2_32() local_unnamed_addr #2 {
prologue:
  ret i32 42
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write)
define private void @_INK2H179_N10_reflectionI120_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_N1_XS23_B1_iC1_cA1_nL0_Q5_i2_32S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_(ptr writeonly captures(none) initializes((0, 4)) %0, ptr readnone captures(none) %1) #1 {
entry:
  store i32 42, ptr %0, align 1
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none)
define private void @_INK2H450_N10_reflectionF391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_(ptr writeonly captures(none) initializes((0, 4)) %0, ptr readonly captures(none) %1) #3 {
entry:
  %2 = load ptr, ptr %1, align 8
  %3 = load ptr, ptr %2, align 1
  %4 = load i32, ptr %3, align 1
  store i32 %4, ptr %0, align 1
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none)
define private void @_INK2H461_N10_reflectionF402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_(ptr readnone captures(none) %0, ptr readonly captures(none) %1) #4 {
entry:
  %2 = load ptr, ptr %1, align 8
  %3 = load ptr, ptr %2, align 1
  store i32 42, ptr %3, align 1
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define private void @_INK2H460_N10_reflectionF401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_S36_B1_cC1_cA1_nL14_p4_wv0_p4_wv0_Q3_v0_(ptr readnone captures(none) %0, ptr readonly captures(none) %1) #2 {
entry:
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef nonnull ptr @_INK2J57_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5Fmethod() local_unnamed_addr #2 {
entry:
  ret ptr @reflection.desc.38
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef i32 @main() local_unnamed_addr #2 {
entry:
  ret i32 42
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
attributes #3 = { mustprogress nofree norecurse nosync nounwind willreturn memory(read, argmem: readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none) }
attributes #4 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none) }

!llvm.module.flags = !{!0, !1}
!ink.abi.definitions = !{!2, !3, !4, !5, !6, !7}

!0 = !{i32 1, !"ink.abi.version", i32 1}
!1 = !{i32 1, !"ink.mangling.version", i32 2}
!2 = !{!"_INK2F118_R84_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_fN4_mainG0_H7_A1_nL0_X0_S23_B1_iC1_cA1_nL0_Q5_i2_32", !"i32 ()"}
!3 = !{!"_INK2F391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32", !"i32 (ptr)"}
!4 = !{!"_INK2F401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_", !"void (ptr)"}
!5 = !{!"_INK2F402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_", !"void (ptr)"}
!6 = !{!"_INK2I120_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_N1_XS23_B1_iC1_cA1_nL0_Q5_i2_32", !"i32 ()"}
!7 = !{!"_INK2T89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_", !"L1288_D1_4D1_4L25_E21_N1_Xi2_32D1_0D1_0D1_1L1245_F403__INK2F391_R265_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN4_readG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S113_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q5_i2_32:0F413__INK2F401_R277_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN15__5F_5Fdel_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_:0F414__INK2F402_R278_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO89_c85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_K1_fN16__5F_5Finit_5F_5FG0_H97_A90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_X0_S111_B1_iC1_cA90_wc85_R78_P31_N5_localL12_N9_anonymousN1_0L0_M18_N14_const_5FmethodO0_K1_cN5_PointG0_H0_X0_L0_Q3_v0_:0"}
