// Translated from solution.cpp.

var MAXN: dynamic = 107;

var INF: dynamic = (1e9 + 7);

var T: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(MAXN);

var l: dynamic = cpp_array(MAXN);

var save: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  read(N);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(l[i]);
      i += 1;
    }
  }
  save.clear();
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      if ((!l[i]))
      {
        save.push_back(A[i]);
      }
      i += 1;
    }
  }
  sort(save.begin(), save.end());
  var pos: dynamic = 0;
  {
    var i: dynamic = N;
    while ((i >= 1))
    {
      if ((l[i] == 0))
      {
        A[i] = save[cpp_update(pos, "++")];
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      write(A[i], " ");
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
