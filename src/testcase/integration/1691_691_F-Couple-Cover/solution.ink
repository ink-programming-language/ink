// Translated from solution.cpp.

var MAX = (1E6 + 10);

var MAXP = 3E6;

var n: dynamic;

var m: dynamic;

var a = cpp_array(MAX);

var c = cpp_array(MAX);

var f = cpp_array((3 * MAX));

var f_less = cpp_array((3 * MAX));

var all: dynamic;

func main()
{
  scanf("%d", (&n));
  all = (cpp_cast(n) * ((n - 1)));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  sort(a, (a + n));
  var j = 0;
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = i;
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
    var i = 1;
    while ((i <= MAXP))
    {
      f_less[i] = (f_less[(i - 1)] + f[(i - 1)]);
      i += 1;
    }
  }
  scanf("%d", (&m));
  {
    var i = 0;
    while ((i < m))
    {
      var p: dynamic;
      scanf("%d", (&p));
      printf("%I64d\n", (all - f_less[p]));
      i += 1;
    }
  }
}
