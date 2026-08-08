// Translated from solution.cpp.

class compare
{
  func operator_call(l: dynamic, r: dynamic)
  {
      if ((l.first == r.first))
      {
        return (l.second > r.second);
      }
      return (l.first > r.first);
    }
}

func isgood(A: dynamic, i: dynamic, j: dynamic)
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

func print(X: dynamic)
{
  {
    var i = 0;
    while ((i < X.size()))
    {
      {
        var j = 0;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var T: dynamic;
  read(n, m, T);
  var d = cpp_construct(m, 0);
  {
    var i = 0;
    while ((i < n))
    {
      var s: dynamic;
      read(s);
      {
        var j = 0;
        while ((j < m))
        {
          var a = (s[j] - cpp_char("0"));
          A[i][j] = a;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var Q: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
    var u = Q.top();
    Q.pop();
    var i = (u.second / 1000);
    var j = (u.second % 1000);
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
    var t = 0;
    while ((t < T))
    {
      var i: dynamic;
      var j: dynamic;
      var k: dynamic;
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
