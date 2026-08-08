// Translated from solution.cpp.

var maxn = (1e6 + 5);

var N = (4e5 + 5);

var n: dynamic;

var k: dynamic;

var a = cpp_array(N);

var M: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var L = 1;
  var R = 0;
  var ok = false;
  var ans = 0;
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
