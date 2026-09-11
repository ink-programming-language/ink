// Translated from solution.cpp.

var N: dynamic = (2e5 + 7);

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = 1e18;

func block(S: dynamic, i: dynamic, j: dynamic, n: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = [[1, 0], [-1, 0], [0, 1], [0, -1]];
  for (var g: dynamic in A)
  {
    var ii: dynamic = (i + g.first);
    var jj: dynamic = (j + g.second);
    if (((((ii >= 0) && (ii < n)) && (jj >= 0)) && (jj < m)))
    {
      if ((S[ii][jj] == cpp_char("G")))
      {
        return false;
      }
      if ((S[ii][jj] == cpp_char(".")))
      {
        S[ii][jj] = cpp_char("#");
      }
    }
  }
  return true;
}

func ok(S: dynamic, i: dynamic, j: dynamic, n: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = [[1, 0], [-1, 0], [0, 1], [0, -1]];
  S[i][j] = cpp_char("0");
  for (var g: dynamic in A)
  {
    var ii: dynamic = (i + g.first);
    var jj: dynamic = (j + g.second);
    if (((((ii >= 0) && (ii < n)) && (jj >= 0)) && (jj < m)))
    {
      if ((S[ii][jj] == cpp_char("1")))
      {
        S[i][j] = cpp_char("1");
        return true;
      }
      if (((S[ii][jj] == cpp_char(".")) || (S[ii][jj] == cpp_char("G"))))
      {
        if (ok(S, ii, jj, n, m))
        {
          S[i][j] = cpp_char("1");
          return true;
        }
      }
    }
  }
  return false;
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var S: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(S[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if (((S[i][j] == cpp_char("B")) && (!block(S, i, j, n, m))))
          {
            write("No\n");
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((S[(n - 1)][(m - 1)] == cpp_char(".")))
  {
    S[(n - 1)][(m - 1)] = cpp_char("1");
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if (((S[i][j] == cpp_char("G")) && (!ok(S, i, j, n, m))))
          {
            write("No\n");
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write("Yes\n");
  return;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
