// Translated from solution.cpp.

var N = (2e5 + 5);

var M = ((2 * N) + 5);

var n: dynamic;

var s = cpp_array(N);

func solve(l: dynamic, r: dynamic, lev: dynamic, state: dynamic)
{
  if ((l > r))
  {
    return 0;
  }
  if (((l == r) || (state == false)))
  {
    var ret = 0;
    {
      var j = l;
      while ((j <= r))
      {
        ret += (((s[j] - cpp_char("a")) != lev));
        j += 1;
      }
    }
    return ret;
  }
  var ret = (n + 5);
  var mid = (((l + r)) / 2);
  ret = min(ret, (solve(l, mid, lev, false) + solve((mid + 1), r, (lev + 1), true)));
  ret = min(ret, (solve(l, mid, (lev + 1), true) + solve((mid + 1), r, lev, false)));
  return ret;
}

func main()
{
  var t: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%d %s", (&n), s);
    printf("%d\n", solve(0, (n - 1), 0, true));
  }
}
