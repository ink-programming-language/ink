// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#i");

var MINF: dynamic = cpp_expression("#inclu");

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAX);

var b: dynamic = cpp_array(MAX);

var c: dynamic = cpp_array(MAX);

var memo: dynamic = cpp_array(MAX, MAX, MAX);

func solve(n: dynamic, l1: dynamic, m: dynamic) -> dynamic
{
  if ((n == N))
  {
    return 0;
  }
  var res: dynamic = memo[n][l1][m];
  if ((res != MINF))
  {
    return res;
  }
  {
    var i: dynamic = 0;
    while ((i <= min(8, m)))
    {
      {
        var j: dynamic = 0;
        while ((j <= i))
        {
          {
            var k: dynamic = 0;
            while ((k <= min(l1, (8 - j))))
            {
              var cost: dynamic = ( (((j + k) == 0)) ? c[n] : ((a[n] * j) + (b[n] * k)));
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

func main() -> dynamic
{
  read(N, M);
  {
    var i: dynamic = 0;
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
