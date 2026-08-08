// Translated from solution.cpp.

func main()
{
  while (1)
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    if (((!n) && (!m)))
    {
      break;
    }
    var ans: dynamic;
    var c = 1;
    var d = 1;
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
      var t = d;
      var a: dynamic;
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
      var i = (n % c);
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
