// Translated from solution.cpp.

var N = (2e5 + 7);

var MOD = (1e9 + 7);

var INF = 1e18;

func block(S: dynamic, i: dynamic, j: dynamic, n: dynamic, m: dynamic)
{
  var A = [[1, 0], [-1, 0], [0, 1], [0, -1]];
  for (var g in A)
  {
    var ii = (i + g.first);
    var jj = (j + g.second);
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

func ok(S: dynamic, i: dynamic, j: dynamic, n: dynamic, m: dynamic)
{
  var A = [[1, 0], [-1, 0], [0, 1], [0, -1]];
  S[i][j] = cpp_char("0");
  for (var g in A)
  {
    var ii = (i + g.first);
    var jj = (j + g.second);
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

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var S = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(S[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
