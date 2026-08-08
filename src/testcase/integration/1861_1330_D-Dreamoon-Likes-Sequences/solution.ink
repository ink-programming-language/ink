// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var d: dynamic;
    var m: dynamic;
    read(d, m);
    var res = 1;
    var msb = 1;
    while ((msb <= d))
    {
      res *= (min(msb, ((d - msb) + 1)) + 1);
      res %= m;
      msb <<= 1;
    }
    write(((((res - 1) + m)) % m), "\n");
  }
}
