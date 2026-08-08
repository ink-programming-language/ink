// Translated from solution.cpp.

var N = 100010;

var w = cpp_array(N);

var fa = cpp_array(N);

var pa = cpp_array(N);

var q = cpp_array(N);

var a = cpp_array(N);

var b = cpp_array(N);

func gi()
{
  var x = 0;
  var o = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-"))) cpp_assign(o, "=", -1) else 0;
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * o);
}

class Dat
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  func operator_less(A: dynamic)
  {
      return (z < A.z);
    }
}

var g = cpp_array(N);

func find(x: dynamic)
{
  return if ((fa[x] == x)) x else cpp_assign(fa[x], "=", find(fa[x]));
}

func get(x: dynamic)
{
  return if ((pa[x] == x)) x else cpp_assign(pa[x], "=", get(pa[x]));
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var ans: dynamic;
  read(n, m);
  ans = m;
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = cpp_assign(b[i], "=", gi());
      fa[i] = cpp_assign(pa[i], "=", i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      g[i].x = gi();
      g[i].y = gi();
      g[i].z = gi();
      i += 1;
    }
  }
  sort((g + 1), ((g + 1) + m));
  {
    var i = 1;
    while ((i <= m))
    {
      var x = find(g[i].x);
      var y = find(g[i].y);
      if ((x != y))
      {
        if ((q[x].size() < q[y].size()))
        {
          swap(x, y);
        }
        w[i] += 1;
        q[x].push_back(i);
        fa[y] = x;
        a[x] += a[y];
        for (var j in q[y])
        {
          q[x].push_back(j);
        }
        if ((a[x] >= g[i].z))
        {
          for (var j in q[x])
          {
            var X = get(g[j].x);
            var Y = get(g[j].y);
            pa[Y] = X;
            b[X] += b[Y];
            ans -= 1;
          }
          q[x].clear();
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      if (((!w[i]) && (b[get(g[i].x)] >= g[i].z)))
      {
        ans -= 1;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
