// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(1202020);

var siz: dynamic = cpp_array(202020);

var mod: dynamic = (1e9 + 7);

var sum: dynamic = 0;

func getf(x: dynamic) -> dynamic
{
  if ((x == f[x]))
  {
    return x;
  }
  return cpp_assign(f[x], "=", getf(f[x]));
}

var m: dynamic = cpp_uninitialized();

var aa: dynamic = cpp_uninitialized();

var bb: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&aa));
      f[aa] += 1;
      i += 1;
    }
  }
  var fl: dynamic = 0;
  {
    var i: dynamic = 2;
    while ((i <= 1000000))
    {
      var sum: dynamic = 0;
      {
        var j: dynamic = 1;
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
