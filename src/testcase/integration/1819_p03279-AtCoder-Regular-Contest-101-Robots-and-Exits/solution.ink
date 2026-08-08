// Translated from solution.cpp.

var N = 100010;

var INF = 1e9;

var MOD = (1e9 + 7);

var n: dynamic;

var m: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

class Point
{
  var x: dynamic;
  var y: dynamic;
}

var p = cpp_array(N);

var d = cpp_array(N);

var dcnt: dynamic;

var f = cpp_array(N);

func readData()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
}

func initDis()
{
  var cnt = 1;
  p[1] = [0, 0];
  {
    var i = 1;
    var j: dynamic;
    while ((i <= n))
    {
      j = (upper_bound((b + 1), ((b + 1) + m), a[i]) - b);
      if (((1 < j) && (j <= m)))
      {
        p[cpp_update(cnt, "++")] = [(a[i] - b[(j - 1)]), (b[j] - a[i])];
      }
      i += 1;
    }
  }
  p[cpp_update(cnt, "++")] = [INF, INF];
  n = cnt;
}

func Diz()
{
  {
    var i = 1;
    while ((i <= n))
    {
      d[cpp_update(dcnt, "++")] = p[i].y;
      i += 1;
    }
  }
  sort((d + 1), ((d + 1) + dcnt));
  dcnt = ((unique((d + 1), ((d + 1) + dcnt)) - d) - 1);
  {
    var i = 1;
    while ((i <= n))
    {
      p[i].y = (lower_bound((d + 1), ((d + 1) + dcnt), p[i].y) - d);
      i += 1;
    }
  }
}

var n: dynamic;

var a = cpp_array(N);

func setup(n: dynamic)
{
  n = n;
}

func add(u: dynamic, x: dynamic)
{
  {
    while ((u && (u <= n)))
    {
      (cpp_assign(a[u], "+=", x)) %= MOD;
      u += (u & (-u));
    }
  }
}

func que(u: dynamic)
{
  var res = 0;
  {
    while (u)
    {
      (cpp_assign(res, "+=", a[u])) %= MOD;
      u -= (u & (-u));
    }
  }
  return res;
}

func cmpByX(a: dynamic, b: dynamic)
{
  if ((a.x != b.x))
  {
    return (a.x < b.x);
  }
  return (a.y < b.y);
}

func solve()
{
  sort((p + 1), ((p + 1) + n), cmpByX);
  n = ((unique((p + 1), ((p + 1) + n)) - p) - 1);
  BIT.setup(dcnt);
  BIT.add(p[1].y, 1);
  {
    var i = 2;
    var j: dynamic;
    while ((i <= n))
    {
      {
        j = i;
        while (((j <= n) && (p[j].x == p[i].x)))
        {
          j += 1;
        }
      }
      {
        var k = i;
        while ((k < j))
        {
          f[k] = BIT.que((p[k].y - 1));
          k += 1;
        }
      }
      {
        var k = i;
        while ((k < j))
        {
          BIT.add(p[k].y, f[k]);
          k += 1;
        }
      }
      i = j;
    }
  }
  printf("%d\n", f[n]);
}

func main()
{
  readData();
  initDis();
  Diz();
  solve();
  return 0;
}
