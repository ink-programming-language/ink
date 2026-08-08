// Translated from solution.cpp.

var n: dynamic;

var f = cpp_array(1202020);

var siz = cpp_array(202020);

var mod = (1e9 + 7);

var sum = 0;

func getf(x: dynamic)
{
  if ((x == f[x]))
  {
    return x;
  }
  return cpp_assign(f[x], "=", getf(f[x]));
}

var m: dynamic;

var aa: dynamic;

var bb: dynamic;

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&aa));
      f[aa] += 1;
      i += 1;
    }
  }
  var fl = 0;
  {
    var i = 2;
    while ((i <= 1000000))
    {
      var sum = 0;
      {
        var j = 1;
        while (((j * i) <= 1000000))
        {
          sum += f[(i * j)];
          j += 1;
        }
      }
      if ((sum == n))
      {
        printf("not coprime");
        return 0;
      }
      if ((sum > 1))
      {
        fl = 1;
      }
      i += 1;
    }
  }
  if (fl)
  {
    printf("setwise coprime");
  } else
  {
    printf("pairwise coprime");
  }
  return 0;
}
