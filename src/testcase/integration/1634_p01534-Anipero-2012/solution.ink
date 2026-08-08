// Translated from solution.cpp.

var MAX = cpp_expression("#i");

var MINF = cpp_expression("#inclu");

var N: dynamic;

var M: dynamic;

var a = cpp_array(MAX);

var b = cpp_array(MAX);

var c = cpp_array(MAX);

var memo = cpp_array(MAX, MAX, MAX);

func solve(n: dynamic, l1: dynamic, m: dynamic)
{
  if ((n == N))
  {
    return 0;
  }
  var res = memo[n][l1][m];
  if ((res != MINF))
  {
    return res;
  }
  {
    var i = 0;
    while ((i <= min(8, m)))
    {
      {
        var j = 0;
        while ((j <= i))
        {
          {
            var k = 0;
            while ((k <= min(l1, (8 - j))))
            {
              var cost = (if (((j + k) == 0)) c[n] else ((a[n] * j) + (b[n] * k)));
              res = max(res, (solve((n + 1), i, (m - i)) + cost));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return res;
}

func main()
{
  read(N, M);
  {
    var i = 0;
    while ((i < N))
    {
      read(a[i], b[i], c[i]);
      i += 1;
    }
  }
  fill((&memo[0][0][0]), (&memo[(MAX - 1)][(MAX - 1)][(MAX - 1)]), MINF);
  write(solve(0, 0, M), "\n");
  return 0;
}
