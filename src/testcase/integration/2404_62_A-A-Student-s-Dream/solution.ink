// Translated from solution.cpp.

func main() -> dynamic
{
  var ml: dynamic = cpp_uninitialized();
  var mr: dynamic = cpp_uninitialized();
  var hl: dynamic = cpp_uninitialized();
  var hr: dynamic = cpp_uninitialized();
  read(ml, mr, hl, hr);
  var ok: dynamic = 0;
  if ((((hr + 1) >= ml) && (hr <= (2 * ((ml + 1))))))
  {
    ok = 1;
  }
  if ((((hl + 1) >= mr) && (hl <= (2 * ((mr + 1))))))
  {
    ok = 1;
  }
  printf( (ok) ? "YES\n" : "NO\n");
  return 0;
}
