// Translated from solution.cpp.

var MAX: dynamic = (1E6 + 10);

var MAXP: dynamic = 3E6;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAX);

var c: dynamic = cpp_array(MAX);

var f: dynamic = cpp_array((3 * MAX));

var f_less: dynamic = cpp_array((3 * MAX));

var all: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  all = (cpp_cast(n) * ((n - 1)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  sort(a, (a + n));
  var j: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] != a[j]))
      {
        j += 1;
        a[j] = a[i];
      }
      c[j] += 1;
      i += 1;
    }
  }
  n = (j + 1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = i;
        while ((j < n))
        {
          if (((MAXP / a[i]) < a[j]))
          {
            break;
          }
          if ((i == j))
          {
            f[(a[i] * a[i])] += (c[i] * ((c[i] - 1)));
          } else
          {
            f[(a[i] * a[j])] += ((2 * c[i]) * c[j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= MAXP))
    {
      f_less[i] = (f_less[(i - 1)] + f[(i - 1)]);
      i += 1;
    }
  }
  scanf("%d", (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var p: dynamic = cpp_uninitialized();
      scanf("%d", (&p));
      printf("%I64d\n", (all - f_less[p]));
      i += 1;
    }
  }
}
