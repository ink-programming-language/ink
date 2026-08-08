// Translated from solution.cpp.

var n: dynamic;

var t: dynamic;

var ans = 1;

var s: dynamic;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(s, s);
      t += (s == "soft");
      i += 1;
    }
  }
  {
    while (((((((ans * ans) + 1)) / 2) < max(t, (n - t))) || ((ans * ans) < n)))
    {
      ans += 1;
    }
  }
  write(ans);
  return 0;
}
