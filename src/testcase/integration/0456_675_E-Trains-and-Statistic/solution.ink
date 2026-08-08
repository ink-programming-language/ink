// Translated from solution.cpp.

var maxn = (cpp_cast(1e5) + 100);

var tr = cpp_construct((4 * maxn));

func upd(idx: dynamic)
{
  var l = (idx * 2);
  var r = ((idx * 2) + 1);
  if (((tr[l] == -1) || (tr[r] == -1)))
  {
    tr[idx] = max(tr[l], tr[r]);
  } else
  {
    (cpp_assign(if ((a[tr[l]] >= a[tr[r]])) cpp_assign(tr[idx], "=", tr[l]) else tr[idx], "=", tr[r]));
  }
}

func build(idx: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    tr[idx] = l;
    return;
  }
  var m = (((l + r)) >> 1);
  build((idx * 2), l, m);
  build(((idx * 2) + 1), (m + 1), r);
  upd(idx);
}

func get(idx: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic)
{
  if (((l >= ll) && (r <= rr)))
  {
    return tr[idx];
  }
  if (((ll > r) || (rr < l)))
  {
    return -1;
  }
  var m = (((l + r)) >> 1);
  var fi = get((idx * 2), l, m, ll, rr);
  var se = get(((idx * 2) + 1), (m + 1), r, ll, rr);
  if ((fi == -1))
  {
    return se;
  }
  if ((se == -1))
  {
    return fi;
  }
  return (if ((a[fi] >= a[se])) fi else se);
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  a[n] = 0;
  build(1, 1, n);
  var dp = cpp_construct((n + 10), 0);
  var ans = 0;
  {
    var i = (n - 1);
    while ((i > 0))
    {
      var idx = get(1, 1, n, (i + 1), a[i]);
      dp[i] = (((dp[idx] + n) - i) - ((a[i] - idx)));
      ans += dp[i];
      i -= 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
