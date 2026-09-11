// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 5);

var N: dynamic = (4e5 + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var M: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var L: dynamic = 1;
  var R: dynamic = 0;
  var ok: dynamic = false;
  var ans: dynamic = 0;
  while ((R <= n))
  {
    if (ok)
    {
      ans += ((n - R) + 1);
      if ((cpp_update(M[a[cpp_update(L, "++")]], "++") == (k - 1)))
      {
        ok = false;
      }
    } else
    {
      if ((cpp_update(M[a[cpp_update(R, "++")]], "++") == k))
      {
        ok = true;
      }
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
