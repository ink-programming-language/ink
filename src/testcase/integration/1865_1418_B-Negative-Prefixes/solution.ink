// Translated from solution.cpp.

var MAXN = 107;

var INF = (1e9 + 7);

var T: dynamic;

var N: dynamic;

var A = cpp_array(MAXN);

var l = cpp_array(MAXN);

var save: dynamic;

func solve()
{
  read(N);
  {
    var i = 1;
    while ((i <= N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= N))
    {
      read(l[i]);
      i += 1;
    }
  }
  save.clear();
  {
    var i = 1;
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
  var pos = 0;
  {
    var i = N;
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
    var i = 1;
    while ((i <= N))
    {
      write(A[i], " ");
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func main()
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
