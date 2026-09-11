// Translated from solution.cpp.

var ans: dynamic = cpp_uninitialized();

var pa: dynamic = cpp_array(55);

var r: dynamic = cpp_array(55);

func findset(x: dynamic) -> dynamic
{
  return  ((pa[x] != x)) ? cpp_assign(pa[x], "=", findset(pa[x])) : x;
}

var f: dynamic = [0];

var mm: dynamic = cpp_array(55);

func get_2() -> dynamic
{
  mm[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < 55))
    {
      mm[i] = (mm[(i - 1)] * 2);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  get_2();
  ans = 1;
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  scanf("%d", (&m));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (n + 1)))
    {
      pa[i] = i;
      r[i] = 0;
      i += 1;
    }
  }
  var k: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&x));
      scanf("%d", (&y));
      x = findset(x);
      y = findset(y);
      if ((x != y))
      {
        if ((r[x] < r[y]))
        {
          pa[x] = y;
        } else
        {
          pa[y] = x;
          if ((r[x] == r[y]))
          {
            r[x] += 1;
          }
        }
        k += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      f[findset(i)] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      if (f[i])
      {
        ans = (ans * mm[(f[i] - 1)]);
      }
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
