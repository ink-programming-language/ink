// Translated from solution.cpp.

var INF: dynamic = 1e9;

var N: dynamic = 1e5;

var mod: dynamic = (1e9 + 7);

var eps: dynamic = 1E-7;

var n: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var mn: dynamic = 1e9;

var cnt: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  var T: dynamic = cpp_uninitialized();
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
}
