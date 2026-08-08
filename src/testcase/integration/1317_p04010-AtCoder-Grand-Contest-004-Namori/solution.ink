// Translated from solution.cpp.

var int_cpp = dynamic;

var N = 200010;

var size = cpp_array(N);

var Flag: dynamic;

var cnt2 = cpp_array(N);

var pp = cpp_array(N);

var cc: dynamic;

var sd = cpp_array(N);

var cnt = cpp_array(N);

var col = cpp_array(N);

var n: dynamic;

var m: dynamic;

var ne = cpp_array(N);

var tot: dynamic;

var fi = cpp_array(N);

var zz = cpp_array(N);

var flag = cpp_array(N);

var x: dynamic;

var y: dynamic;

var X: dynamic;

var Y: dynamic;

func jb(x: dynamic, y: dynamic)
{
  ne[cpp_update(tot, "++")] = fi[x];
  fi[x] = tot;
  zz[tot] = y;
}

func dfs2(x: dynamic)
{
  cnt2[x] = col[x];
  flag[x] = 1;
  {
    var i = fi[x];
    while (i)
    {
      if (((!flag[zz[i]]) && (!pp[i])))
      {
        dfs2(zz[i]);
        cnt2[x] += cnt2[zz[i]];
      }
      i = ne[i];
    }
  }
}

func solve()
{
  memset(flag, 0, cpp_sizeof(flag));
  dfs2(1);
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ans += abs(((size[i] - cnt2[i]) - cnt[i]));
      i += 1;
    }
  }
  return ans;
}

func check(x: dynamic)
{
  col[X] -= x;
  col[Y] += x;
  var k = solve();
  col[X] += x;
  col[Y] -= x;
  return (k + abs(x));
}

func dfs(x: dynamic, y: dynamic)
{
  size[x] = cpp_assign(flag[x], "=", 1);
  cnt[x] = col[x];
  {
    var i = fi[x];
    while (i)
    {
      if ((i != y))
      {
        if ((flag[zz[i]] && (sd[zz[i]] < sd[x])))
        {
          X = x;
          cc += 1;
          Y = zz[i];
          Flag = (((sd[x] - sd[zz[i]])) & 1);
          pp[i] = cpp_assign(pp[(i ^ 1)], "=", 1);
          i = ne[i];
          continue;
        }
        if (flag[zz[i]])
        {
          i = ne[i];
          continue;
        }
        sd[zz[i]] = (sd[x] + 1);
        col[zz[i]] = (col[x] ^ 1);
        dfs(zz[i], (i ^ 1));
        cnt[x] += cnt[zz[i]];
        size[x] += size[zz[i]];
      }
      i = ne[i];
    }
  }
}

func main()
{
  scanf("%lld%lld", (&n), (&m));
  tot = 1;
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%lld%lld", (&x), (&y));
      jb(x, y);
      jb(y, x);
      i += 1;
    }
  }
  col[1] = 1;
  dfs(1, 0);
  var S = cnt[1];
  var T = (n - cnt[1]);
  if ((m == (n - 1)))
  {
    if ((S != T))
    {
      puts("-1");
      return 0;
    }
    printf("%lld\n", solve());
  } else if (Flag)
  {
    if ((S != T))
    {
      puts("-1");
      return 0;
    }
    var l = (-n);
    var r = n;
    while (((l + 5) < r))
    {
      var mid1 = (l + (((r - l)) / 3));
      var mid2 = (r - (((r - l)) / 3));
      if ((check(mid1) > check(mid2)))
      {
        l = mid1;
      } else
      {
        r = mid2;
      }
    }
    var ans = 1e18;
    {
      var i = l;
      while ((i <= r))
      {
        ans = min(ans, check(i));
        i += 1;
      }
    }
    printf("%lld\n", ans);
  } else
  {
    if ((((S & 1)) != ((T & 1))))
    {
      puts("-1");
      return 0;
    }
    col[X] += (((T - S)) / 2);
    col[Y] += (((T - S)) / 2);
    printf("%lld\n", (solve() + (abs((T - S)) / 2)));
  }
  return 0;
}
