// Translated from solution.cpp.

var N = (3e5 + 5);

var inf = (1e18 + 100);

var g = cpp_array(N);

var used = cpp_array(N);

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= (3 * n)))
    {
      g[i].clear();
      used[i] = 0;
      i += 1;
    }
  }
  var seq: dynamic;
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      if (((!used[u]) && (!used[v])))
      {
        used[u] = 1;
        used[v] = 1;
        seq.push_back(i);
      }
      i += 1;
    }
  }
  if ((seq.size() >= n))
  {
    write("Matching", cpp_char("\n"));
    {
      var i = 0;
      while ((i < n))
      {
        write(seq[i], cpp_char(" "));
        i += 1;
      }
    }
    write(cpp_char("\n"));
    return;
  }
  seq.clear();
  {
    var i = 1;
    while ((i <= (3 * n)))
    {
      if ((!used[i]))
      {
        seq.push_back(i);
      }
      i += 1;
    }
  }
  if ((seq.size() >= n))
  {
    write("IndSet", cpp_char("\n"));
    {
      var i = 0;
      while ((i < n))
      {
        write(seq[i], cpp_char(" "));
        i += 1;
      }
    }
    write(cpp_char("\n"));
    return;
  }
  write("Impossible", cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    solve();
  }
  return 0;
}
