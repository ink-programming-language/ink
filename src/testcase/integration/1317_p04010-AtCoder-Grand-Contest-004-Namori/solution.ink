// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var N: dynamic = 200010;

var size: dynamic = cpp_array(N);

var Flag: dynamic = cpp_uninitialized();

var cnt2: dynamic = cpp_array(N);

var pp: dynamic = cpp_array(N);

var cc: dynamic = cpp_uninitialized();

var sd: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(N);

var col: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ne: dynamic = cpp_array(N);

var tot: dynamic = cpp_uninitialized();

var fi: dynamic = cpp_array(N);

var zz: dynamic = cpp_array(N);

var flag: dynamic = cpp_array(N);

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

func jb(x: dynamic, y: dynamic) -> dynamic
{
  ne[cpp_update(tot, "++")] = fi[x];
  fi[x] = tot;
  zz[tot] = y;
}

func dfs2(x: dynamic) -> dynamic
{
  cnt2[x] = col[x];
  flag[x] = 1;
  {
    var i: dynamic = fi[x];
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

func solve() -> dynamic
{
  memset(flag, 0, cpp_sizeof(flag));
  dfs2(1);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += abs(((size[i] - cnt2[i]) - cnt[i]));
      i += 1;
    }
  }
  return ans;
}

func check(x: dynamic) -> dynamic
{
  col[X] -= x;
  col[Y] += x;
  var k: dynamic = solve();
  col[X] += x;
  col[Y] -= x;
  return (k + abs(x));
}

func dfs(x: dynamic, y: dynamic) -> dynamic
{
  size[x] = cpp_assign(flag[x], "=", 1);
  cnt[x] = col[x];
  {
    var i: dynamic = fi[x];
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

func main() -> dynamic
{
  scanf("%lld%lld", (&n), (&m));
  tot = 1;
  {
    var i: dynamic = 1;
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
  var S: dynamic = cnt[1];
  var T: dynamic = (n - cnt[1]);
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
    var l: dynamic = (-n);
    var r: dynamic = n;
    while (((l + 5) < r))
    {
      var mid1: dynamic = (l + (((r - l)) / 3));
      var mid2: dynamic = (r - (((r - l)) / 3));
      if ((check(mid1) > check(mid2)))
      {
        l = mid1;
      } else
      {
        r = mid2;
      }
    }
    var ans: dynamic = 1e18;
    {
      var i: dynamic = l;
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
