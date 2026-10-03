// Native linkage remains available even though the declaration is private to Ink source lookup.
private export "C" func inkNativeBoundaryValue(): i32
{
  return 21;
}
