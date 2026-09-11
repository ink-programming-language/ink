// Translated from solution.cpp.

var N: dynamic = 100010;

var w: dynamic = cpp_array(N);

var fa: dynamic = cpp_array(N);

var pa: dynamic = cpp_array(N);

var q: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

func gi() -> dynamic
{
  var x: dynamic = 0;
  var o: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
     ((ch == cpp_char("-"))) ? cpp_assign(o, "=", -1) : 0;
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
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  func operator_less(A: dynamic) -> dynamic
  {
      return (z < A.z);
    }
}

var g: dynamic = cpp_array(N);

func find(x: dynamic) -> dynamic
{
  return  ((fa[x] == x)) ? x : cpp_assign(fa[x], "=", find(fa[x]));
}

func get(x: dynamic) -> dynamic
{
  return  ((pa[x] == x)) ? x : cpp_assign(pa[x], "=", get(pa[x]));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  read(n, m);
  ans = m;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = cpp_assign(b[i], "=", gi());
      fa[i] = cpp_assign(pa[i], "=", i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = find(g[i].x);
      var y: dynamic = find(g[i].y);
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
        for (var j: dynamic in q[y])
        {
          q[x].push_back(j);
        }
        if ((a[x] >= g[i].z))
        {
          for (var j: dynamic in q[x])
          {
            var X: dynamic = get(g[j].x);
            var Y: dynamic = get(g[j].y);
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
    var i: dynamic = 1;
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
