// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a ^= b;
    b ^= a;
    a ^= b;
  }
  return  (((a > b))) ? gcd((a - b), b) : a;
}

func abs(x: dynamic) -> dynamic
{
  return  ((x > 0)) ? x : (-x);
}

func solve() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var all: dynamic = cpp_uninitialized();
  var fin: dynamic = false;
  read(a, b, c);
  if ((b >= a))
  {
    write(0);
    return;
  }
  t = 0;
  all = (((((c * a) + b) - 1)) / b);
  while (cpp_update(t, "++"))
  {
    if (((all * b) >= (((all - t)) * a)))
    {
      break;
    }
  }
  write(t);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
