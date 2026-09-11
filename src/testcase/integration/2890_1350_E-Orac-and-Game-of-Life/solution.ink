// Translated from solution.cpp.

class compare
{
  func operator_call(l: dynamic, r: dynamic) -> dynamic
  {
      if ((l.first == r.first))
      {
        return (l.second > r.second);
      }
      return (l.first > r.first);
    }
}

func isgood(A: dynamic, i: dynamic, j: dynamic) -> dynamic
{
  if (((i < (A.size() - 1)) && (A[(i + 1)][j] == A[i][j])))
  {
    return true;
  }
  if (((j < (A[i].size() - 1)) && (A[i][(j + 1)] == A[i][j])))
  {
    return true;
  }
  if (((i > 0) && (A[(i - 1)][j] == A[i][j])))
  {
    return true;
  }
  if (((j > 0) && (A[i][(j - 1)] == A[i][j])))
  {
    return true;
  }
  return false;
}

func print(X: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < X.size()))
    {
      {
        var j: dynamic = 0;
        while ((j < X[i].size()))
        {
          write(X[i][j], " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  read(n, m, T);
  var d: dynamic = cpp_construct(m, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var s: dynamic = cpp_uninitialized();
      read(s);
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var a: dynamic = (s[j] - cpp_char("0"));
          A[i][j] = a;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var Q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((X[i][j] == 0))
          {
            X[i][j] = isgood(A, i, j);
          }
          if ((X[i][j] == 1))
          {
            Q.push([1, ((1000 * i) + j)]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  while ((!Q.empty()))
  {
    var u: dynamic = Q.top();
    Q.pop();
    var i: dynamic = (u.second / 1000);
    var j: dynamic = (u.second % 1000);
    if (((i < (X.size() - 1)) && (X[(i + 1)][j] == 0)))
    {
      X[(i + 1)][j] = (X[i][j] + 1);
      Q.push([X[(i + 1)][j], ((1000 * ((i + 1))) + j)]);
    }
    if (((i > 0) && (X[(i - 1)][j] == 0)))
    {
      X[(i - 1)][j] = (X[i][j] + 1);
      Q.push([X[(i - 1)][j], ((1000 * ((i - 1))) + j)]);
    }
    if (((j < (X[i].size() - 1)) && (X[i][(j + 1)] == 0)))
    {
      X[i][(j + 1)] = (X[i][j] + 1);
      Q.push([X[i][(j + 1)], (((1000 * i) + j) + 1)]);
    }
    if (((j > 0) && (X[i][(j - 1)] == 0)))
    {
      X[i][(j - 1)] = (X[i][j] + 1);
      Q.push([X[i][(j - 1)], (((1000 * i) + j) - 1)]);
    }
  }
  {
    var t: dynamic = 0;
    while ((t < T))
    {
      var i: dynamic = cpp_uninitialized();
      var j: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      read(i, j, k);
      i -= 1;
      j -= 1;
      if (((X[i][j] > k) || (X[i][j] == 0)))
      {
        write(A[i][j], "\n");
      } else
      {
        write((((A[i][j] + ((((k - X[i][j]) + 1)) % 2))) % 2), "\n");
      }
      t += 1;
    }
  }
  return 0;
}
