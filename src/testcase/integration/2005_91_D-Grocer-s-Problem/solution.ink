// Translated from solution.cpp.

var N = 200002;

var ans: dynamic;

var cs: dynamic;

var twoCs: dynamic;

var threeCs: dynamic;

var n: dynamic;

var p = cpp_array(N);

func main()
{
  ios.sync_with_stdio(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(p[i]);
      i += 1;
    }
  }
  var mark = [];
  {
    var i = 1;
    while ((i <= n))
    {
      if (((!mark[i]) && (p[i] != i)))
      {
        var newC: dynamic;
        var v = i;
        while ((!mark[v]))
        {
          mark[v] = 1;
          newC.push_back(v);
          v = p[v];
        }
        cs.push_back(newC);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cs.size()))
    {
      while ((cs[i].size() >= 4))
      {
        var move: dynamic;
        {
          var j = max(0, (cpp_cast(cs[i].size()) - 5));
          while (((j + 1) < cs[i].size()))
          {
            move.push_back(make_pair(cs[i][j], cs[i][(j + 1)]));
            j += 1;
          }
        }
        move.push_back(make_pair(cs[i].back(), cs[i][max((cpp_cast(cs[i].size()) - 5), 0)]));
        ans.push_back(move);
        {
          var j = 0;
          while ((j < 4))
          {
            cs[i].pop_back();
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cs.size()))
    {
      if ((cs[i].size() == 2))
      {
        twoCs.push_back(cs[i]);
      } else if ((cs[i].size() == 3))
      {
        threeCs.push_back(cs[i]);
      }
      i += 1;
    }
  }
  while (threeCs.size())
  {
    var move: dynamic;
    var tt = threeCs.back();
    move.push_back(make_pair(tt[0], tt[1]));
    move.push_back(make_pair(tt[1], tt[2]));
    move.push_back(make_pair(tt[2], tt[0]));
    threeCs.pop_back();
    if (twoCs.size())
    {
      tt = twoCs.back();
      move.push_back(make_pair(tt[0], tt[1]));
      move.push_back(make_pair(tt[1], tt[0]));
      twoCs.pop_back();
    } else if (threeCs.size())
    {
      tt = threeCs.back();
      move.push_back(make_pair(tt[0], tt[1]));
      move.push_back(make_pair(tt[1], tt[0]));
      var newC: dynamic;
      newC.push_back(tt[0]);
      newC.push_back(tt[2]);
      twoCs.push_back(newC);
      threeCs.pop_back();
    }
    ans.push_back(move);
  }
  while (twoCs.size())
  {
    var move: dynamic;
    var tt = twoCs.back();
    move.push_back(make_pair(tt[0], tt[1]));
    move.push_back(make_pair(tt[1], tt[0]));
    twoCs.pop_back();
    if (twoCs.size())
    {
      tt = twoCs.back();
      move.push_back(make_pair(tt[0], tt[1]));
      move.push_back(make_pair(tt[1], tt[0]));
      twoCs.pop_back();
    }
    ans.push_back(move);
  }
  write(ans.size(), "\n");
  {
    var i = 0;
    while ((i < ans.size()))
    {
      write(ans[i].size(), "\n");
      {
        var j = 0;
        while ((j < ans[i].size()))
        {
          write(ans[i][j].first, " ");
          j += 1;
        }
      }
      write("\n");
      {
        var j = 0;
        while ((j < ans[i].size()))
        {
          write(ans[i][j].second, " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}
