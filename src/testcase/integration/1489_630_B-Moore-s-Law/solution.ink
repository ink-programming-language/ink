// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var lsbl: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var lsbl1: dynamic = cpp_uninitialized();

func ksm(a: dynamic, b: dynamic) -> dynamic
{
  var sumend: dynamic = 1;
  while (b)
  {
    if (((b % 2) == 1))
    {
      sumend *= a;
    }
    a *= a;
    b /= 2;
  }
  return sumend;
}

func main() -> dynamic
{
  read(n, t);
  lsbl1 = (n * ksm(1.000000011, t));
  write(fixed, setprecision(15), (n * ksm(1.000000011, t)), "\n");
  return 0;
}
