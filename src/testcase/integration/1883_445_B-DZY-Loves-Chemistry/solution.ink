// Translated from solution.cpp.

var ans: dynamic;

var pa = cpp_array(55);

var r = cpp_array(55);

func findset(x: dynamic)
{
  return if ((pa[x] != x)) cpp_assign(pa[x], "=", findset(pa[x])) else x;
}

var f = [0];

var mm = cpp_array(55);

func get_2()
{
  mm[0] = 1;
  {
    var i = 1;
    while ((i < 55))
    {
      mm[i] = (mm[(i - 1)] * 2);
      i += 1;
    }
  }
}

func main()
{
  get_2();
  ans = 1;
  var n: dynamic;
  var m: dynamic;
  scanf("%d", (&n));
  scanf("%d", (&m));
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
    while ((i < (n + 1)))
    {
      pa[i] = i;
      r[i] = 0;
      i += 1;
    }
  }
  var k = 0;
  {
    var i = 0;
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
    var i = 1;
    while ((i < (n + 1)))
    {
      f[findset(i)] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
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
