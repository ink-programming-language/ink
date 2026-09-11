// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array((1001010));

var rt: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(40, (1001010));

var Vec: dynamic = cpp_array((1001010));

var ans: dynamic = cpp_uninitialized();

func Dfs(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < Vec[x].size()))
    {
      var y: dynamic = Vec[x][i];
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

func Lmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return a;
  }
  return b;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&A[i]));
      i += 1;
    }
  }
  rt = 1;
  {
    var i: dynamic = 2;
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
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d %d", (&x), (&y));
      Vec[x].push_back(y);
      Vec[y].push_back(x);
      i += 1;
    }
  }
  Dfs(rt);
  f[rt][0] = rt;
  {
    var j: dynamic = 1;
    while ((j <= 30))
    {
      {
        var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((i != rt))
      {
        var Min: dynamic = (cpp_cast(A[f[i][0]]) + cpp_cast(A[i]));
        {
          var k: dynamic = 1;
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
