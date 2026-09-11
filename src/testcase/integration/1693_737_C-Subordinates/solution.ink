// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var second: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var k: dynamic = 1;
  var i: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  read(n, second);
  var mp: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= n))
    {
      read(a);
      if (((i == second) && a))
      {
        ans += 1;
      } else
      {
        mp[a] += 1;
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      if ((k >= n))
      {
        break;
      }
      if ((mp[i] == 0))
      {
        k += 1;
        ans += 1;
      } else
      {
        k += mp[i];
      }
      i += 1;
    }
  }
  write(ans);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
