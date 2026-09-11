// Translated from solution.cpp.

var N: dynamic = (3e5 + 5);

var inf: dynamic = (1e18 + 100);

var g: dynamic = cpp_array(N);

var used: dynamic = cpp_array(N);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= (3 * n)))
    {
      g[i].clear();
      used[i] = 0;
      i += 1;
    }
  }
  var seq: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
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
      var i: dynamic = 0;
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
    var i: dynamic = 1;
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
      var i: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    solve();
  }
  return 0;
}
