// Translated from solution.cpp.

var maxn = (3e5 + 10);

var n: dynamic;

var k: dynamic;

var s = cpp_array(maxn);

var pre = cpp_array((maxn << 1));

var sz = cpp_array((maxn << 1));

var op = cpp_array(maxn);

func find(x: dynamic)
{
  return if ((x == pre[x])) x else cpp_assign(pre[x], "=", find(pre[x]));
}

func merge(x: dynamic, y: dynamic)
{
  var fx = find(x);
  var fy = find(y);
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

func cal(x: dynamic)
{
  var y = if (((x <= k))) (x + k) else (x - k);
  var fx = find(x);
  var fy = find(y);
  if (((fx == 0) || (fy == 0)))
  {
    return sz[(fx + fy)];
  }
  return min(sz[fx], sz[fy]);
}

func main()
{
  scanf("%d%d", (&n), (&k));
  scanf("%s", (s + 1));
  {
    var i = 1;
    while ((i <= k))
    {
      pre[i] = i;
      pre[(i + k)] = (i + k);
      sz[(i + k)] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    var c: dynamic;
    var x: dynamic;
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
  var res = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((op[i].size() == 1))
      {
        var x = op[i][0];
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
        var x = op[i][0];
        var y = op[i][1];
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
