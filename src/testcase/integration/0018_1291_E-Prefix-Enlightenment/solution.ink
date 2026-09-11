// Translated from solution.cpp.

var maxn: dynamic = (3e5 + 10);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(maxn);

var pre: dynamic = cpp_array((maxn << 1));

var sz: dynamic = cpp_array((maxn << 1));

var op: dynamic = cpp_array(maxn);

func find(x: dynamic) -> dynamic
{
  return  ((x == pre[x])) ? x : cpp_assign(pre[x], "=", find(pre[x]));
}

func merge(x: dynamic, y: dynamic) -> dynamic
{
  var fx: dynamic = find(x);
  var fy: dynamic = find(y);
  if ((fy == 0))
  {
    swap(fx, fy);
  }
  if ((fx != fy))
  {
    pre[fy] = fx;
    sz[fx] += sz[fy];
  }
}

func cal(x: dynamic) -> dynamic
{
  var y: dynamic =  (((x <= k))) ? (x + k) : (x - k);
  var fx: dynamic = find(x);
  var fy: dynamic = find(y);
  if (((fx == 0) || (fy == 0)))
  {
    return sz[(fx + fy)];
  }
  return min(sz[fx], sz[fy]);
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  scanf("%s", (s + 1));
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      pre[i] = i;
      pre[(i + k)] = (i + k);
      sz[(i + k)] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var c: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    while ((i <= k))
    {
      scanf("%d", (&c));
      while (cpp_update(c, "--"))
      {
        scanf("%d", (&x));
        op[x].push_back(i);
      }
      i += 1;
    }
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((op[i].size() == 1))
      {
        var x: dynamic = op[i][0];
        res -= cal(x);
        if ((s[i] == cpp_char("1")))
        {
          pre[find((x + k))] = 0;
        } else
        {
          pre[find(x)] = 0;
        }
        res += cal(x);
      } else if ((op[i].size() == 2))
      {
        var x: dynamic = op[i][0];
        var y: dynamic = op[i][1];
        if ((s[i] == cpp_char("1")))
        {
          if ((find(x) != find(y)))
          {
            res -= (cal(x) + cal(y));
            merge(x, y);
            merge((x + k), (y + k));
            res += cal(x);
          }
        } else
        {
          if ((find((x + k)) != find(y)))
          {
            res -= (cal(x) + cal(y));
            merge((x + k), y);
            merge(x, (y + k));
            res += cal(x);
          }
        }
      }
      printf("%d\n", res);
      i += 1;
    }
  }
  return 0;
}
