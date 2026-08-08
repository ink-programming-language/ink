// Translated from solution.cpp.

var n: dynamic;

var A = cpp_array((1001010));

var rt: dynamic;

var f = cpp_array(40, (1001010));

var Vec = cpp_array((1001010));

var ans: dynamic;

func Dfs(x: dynamic)
{
  {
    var i = 0;
    while ((i < Vec[x].size()))
    {
      var y = Vec[x][i];
      if ((y == f[x][0]))
      {
        i += 1;
        continue;
      }
      f[y][0] = x;
      Dfs(y);
      i += 1;
    }
  }
}

func Lmin(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return a;
  }
  return b;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&A[i]));
      i += 1;
    }
  }
  rt = 1;
  {
    var i = 2;
    while ((i <= n))
    {
      if ((A[i] < A[rt]))
      {
        rt = i;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d %d", (&x), (&y));
      Vec[x].push_back(y);
      Vec[y].push_back(x);
      i += 1;
    }
  }
  Dfs(rt);
  f[rt][0] = rt;
  {
    var j = 1;
    while ((j <= 30))
    {
      {
        var i = 1;
        while ((i <= n))
        {
          f[i][j] = f[f[i][(j - 1)]][(j - 1)];
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i != rt))
      {
        var Min = (cpp_cast(A[f[i][0]]) + cpp_cast(A[i]));
        {
          var k = 1;
          while ((k <= 30))
          {
            Min = Lmin(Min, ((cpp_cast(k) * cpp_cast(min(A[i], A[f[i][k]]))) + cpp_cast(((A[i] + A[f[i][k]])))));
            k += 1;
          }
        }
        ans += Min;
      }
      i += 1;
    }
  }
  write(ans);
}
