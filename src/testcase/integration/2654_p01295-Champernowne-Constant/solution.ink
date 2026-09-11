// Translated from solution.cpp.

func main() -> dynamic
{
  while (1)
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    if (((!n) && (!m)))
    {
      break;
    }
    var ans: dynamic = cpp_uninitialized();
    var c: dynamic = 1;
    var d: dynamic = 1;
    n -= 1;
    while (((n - ((9 * c) * d)) >= 0))
    {
      n -= ((9 * c) * d);
      c += 1;
      d *= 10;
    }
    d = ((n / c) + d);
    while ((cpp_cast(ans.size()) < ((m + (n % c)) + 10)))
    {
      var t: dynamic = d;
      var a: dynamic = cpp_uninitialized();
      while (t)
      {
        a += (((t % 10) + cpp_char("0")));
        t /= 10;
      }
      reverse(a.begin(), a.end());
      ans += a;
      d += 1;
    }
    {
      var i: dynamic = (n % c);
      while ((i < (m + (n % c))))
      {
        write(ans[i]);
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
