// Translated from solution.cpp.

var INF = 1e9;

var N = 1e5;

var mod = (1e9 + 7);

var eps = 1E-7;

var n: dynamic;

var mx: dynamic;

var mn = 1e9;

var cnt: dynamic;

var m: dynamic;

var ans: dynamic;

func solve()
{
  var s: dynamic;
  read(s);
  var s1: dynamic;
  var s2: dynamic;
  {
    var i = 0;
    while ((i < s.size()))
    {
      if (((s[i] % 2) == 0))
      {
        s1 += s[i];
      } else
      {
        s2 += s[i];
      }
      i += 1;
    }
  }
  merge(s1.begin(), s1.end(), s2.begin(), s2.end(), s.begin());
  write(s, "\n");
}

func main()
{
  ios_base.sync_with_stdio(0);
  var T: dynamic;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
}
