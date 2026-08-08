// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (n); ++i)");
}

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  var a: dynamic;
  var ans: dynamic;
  rep(i, (h - 1));
  {
    if ((mp[i][(w - 1)] == 1))
    {
      mp[i][(w - 1)] = 0;
      mp[(i + 1)][(w - 1)] = (((mp[(i + 1)][(w - 1)] + 1)) % 2);
      var v = [i, (w - 1), (i + 1), (w - 1)];
      ans.push_back(v);
    }
  }
  write(ans.size(), "\n");
  rep(i, ans.size());
  {
    cpp_statement("rep(j,4)");
    {
      if ((j > 0))
      {
        write(cpp_char(" "));
      }
      write((ans[i][j] + 1));
    }
    write("\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      read(a);
      mp[i][j] = (a % 2);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    cpp_statement("rep(j,w-1)");
    {
      if ((mp[i][j] == 1))
      {
        mp[i][j] = 0;
        mp[i][(j + 1)] = (((mp[i][(j + 1)] + 1)) % 2);
        var v = [i, j, i, (j + 1)];
        ans.push_back(v);
      }
    }
  }
