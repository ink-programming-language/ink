// Translated from solution.cpp.

var maxn: dynamic = (cpp_cast(1e5) + 100);

var tr: dynamic = cpp_construct((4 * maxn));

func upd(idx: dynamic) -> dynamic
{
  var l: dynamic = (idx * 2);
  var r: dynamic = ((idx * 2) + 1);
  if (((tr[l] == -1) || (tr[r] == -1)))
  {
    tr[idx] = max(tr[l], tr[r]);
  } else
  {
    (cpp_assign( ((a[tr[l]] >= a[tr[r]])) ? cpp_assign(tr[idx], "=", tr[l]) : tr[idx], "=", tr[r]));
  }
}

func build(idx: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    tr[idx] = l;
    return;
  }
  var m: dynamic = (((l + r)) >> 1);
  build((idx * 2), l, m);
  build(((idx * 2) + 1), (m + 1), r);
  upd(idx);
}

func get(idx: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic) -> dynamic
{
  if (((l >= ll) && (r <= rr)))
  {
    return tr[idx];
  }
  if (((ll > r) || (rr < l)))
  {
    return -1;
  }
  var m: dynamic = (((l + r)) >> 1);
  var fi: dynamic = get((idx * 2), l, m, ll, rr);
  var se: dynamic = get(((idx * 2) + 1), (m + 1), r, ll, rr);
  if ((fi == -1))
  {
    return se;
  }
  if ((se == -1))
  {
    return fi;
  }
  return ( ((a[fi] >= a[se])) ? fi : se);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  a[n] = 0;
  build(1, 1, n);
  var dp: dynamic = cpp_construct((n + 10), 0);
  var ans: dynamic = 0;
  {
    var i: dynamic = (n - 1);
    while ((i > 0))
    {
      var idx: dynamic = get(1, 1, n, (i + 1), a[i]);
      dp[i] = (((dp[idx] + n) - i) - ((a[i] - idx)));
      ans += dp[i];
      i -= 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
