// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var d: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(d, m);
    var res: dynamic = 1;
    var msb: dynamic = 1;
    while ((msb <= d))
    {
      res *= (min(msb, ((d - msb) + 1)) + 1);
      res %= m;
      msb <<= 1;
    }
    write(((((res - 1) + m)) % m), "\n");
  }
}
