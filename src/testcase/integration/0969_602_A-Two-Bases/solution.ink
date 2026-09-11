// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var bn: dynamic = cpp_uninitialized();
  read(n, bn);
  var sum: dynamic = 0;
  while (cpp_update(n, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    read(a);
    sum = ((cpp_cast(bn) * sum) + a);
  }
  var sum1: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var bm: dynamic = cpp_uninitialized();
  read(m, bm);
  while (cpp_update(m, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    read(a);
    sum1 = ((cpp_cast(bm) * sum1) + a);
  }
  if ((sum > sum1))
  {
    write(cpp_char(">"));
  } else if ((sum < sum1))
  {
    write(cpp_char("<"));
  } else
  {
    write(cpp_char("="));
  }
}
