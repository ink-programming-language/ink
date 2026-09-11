// Translated from solution.cpp.

var l: dynamic = cpp_array(2, 100005);

var r: dynamic = cpp_array(2, 100005);

var x: dynamic = cpp_array(100005);

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  n -= 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (x + i));
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      r[i][1] =  (((x[i] == 1))) ? 0 : ((r[(i + 1)][1] + x[i]) & ((~1)));
      if ((x[i] % 2))
      {
        r[i][0] = max(r[i][1], (x[i] + r[(i + 1)][0]));
      } else
      {
        r[i][0] = max(r[i][1], ((x[i] - 1) + r[(i + 1)][0]));
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      l[i][1] =  (((x[(i - 1)] == 1))) ? 0 : ((l[(i - 1)][1] + x[(i - 1)]) & ((~1)));
      if ((x[(i - 1)] % 2))
      {
        l[i][0] = max(l[i][1], (x[(i - 1)] + l[(i - 1)][0]));
      } else
      {
        l[i][0] = max(l[i][1], ((x[(i - 1)] - 1) + l[(i - 1)][0]));
      }
      i += 1;
    }
  }
  var q: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      q = max(q, (r[i][0] + l[i][0]));
      i += 1;
    }
  }
  write(q, "\n");
  return 0;
}
