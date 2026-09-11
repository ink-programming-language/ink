// Translated from solution.cpp.

var N: dynamic = 100010;

var INF: dynamic = 1e9;

var MOD: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(N);

var d: dynamic = cpp_array(N);

var dcnt: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

func readData() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
}

func initDis() -> dynamic
{
  var cnt: dynamic = 1;
  p[1] = [0, 0];
  {
    var i: dynamic = 1;
    var j: dynamic = cpp_uninitialized();
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

func Diz() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      d[cpp_update(dcnt, "++")] = p[i].y;
      i += 1;
    }
  }
  sort((d + 1), ((d + 1) + dcnt));
  dcnt = ((unique((d + 1), ((d + 1) + dcnt)) - d) - 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i].y = (lower_bound((d + 1), ((d + 1) + dcnt), p[i].y) - d);
      i += 1;
    }
  }
}

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func setup(n: dynamic) -> dynamic
{
  n = n;
}

func add(u: dynamic, x: dynamic) -> dynamic
{
  {
    while ((u && (u <= n)))
    {
      (cpp_assign(a[u], "+=", x)) %= MOD;
      u += (u & (-u));
    }
  }
}

func que(u: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    while (u)
    {
      (cpp_assign(res, "+=", a[u])) %= MOD;
      u -= (u & (-u));
    }
  }
  return res;
}

func cmpByX(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.x != b.x))
  {
    return (a.x < b.x);
  }
  return (a.y < b.y);
}

func solve() -> dynamic
{
  sort((p + 1), ((p + 1) + n), cmpByX);
  n = ((unique((p + 1), ((p + 1) + n)) - p) - 1);
  BIT.setup(dcnt);
  BIT.add(p[1].y, 1);
  {
    var i: dynamic = 2;
    var j: dynamic = cpp_uninitialized();
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
        var k: dynamic = i;
        while ((k < j))
        {
          f[k] = BIT.que((p[k].y - 1));
          k += 1;
        }
      }
      {
        var k: dynamic = i;
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

func main() -> dynamic
{
  readData();
  initDis();
  Diz();
  solve();
  return 0;
}
