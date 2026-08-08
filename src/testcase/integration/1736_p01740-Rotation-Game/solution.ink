// Translated from solution.cpp.

var sv: dynamic;

var tv: dynamic;

var W = 4040;

var dp = cpp_array(2, 4, W, W);

var used = cpp_array(2, 4, W, W);

func solve(i: dynamic, j: dynamic, k: dynamic, f: dynamic)
{
  if (cpp_binary((i == sv.size()), "and", (j == tv.size())))
  {
    return 0;
  }
  if ((j == tv.size()))
  {
    return (1 << 28);
  }
  k = min(k, (tv[j].first.first + 2));
  k = max(k, (tv[j].first.first - 1));
  var kf = (k - ((tv[j].first.first - 1)));
  if (used[i][j][kf][(f + 1)])
  {
    return dp[i][j][kf][(f + 1)];
  }
  used[i][j][kf][(f + 1)] = true;
  var re = dp[i][j][kf][(f + 1)];
  re = (1 << 28);
  if ((tv[j].second == 1))
  {
    re = min(re, solve(i, (j + 1), k, f));
  }
  if ((i == sv.size()))
  {
    return re;
  }
  if ((tv[j].first.first < sv[i].first.first))
  {
    var u = (sv[i].first.first - tv[j].first.first);
    if (((tv[j].first.first == sv[(i - 1)].first.first) && (f != -1)))
    {
      u -= 1;
    }
    re = min(re, (u + solve((i + 1), (j + 1), sv[i].first.first, -1)));
  } else if ((sv[i].first.first < tv[j].first.first))
  {
    var u = (tv[j].first.first - sv[i].first.first);
    if (((sv[i].first.first == sv[(i - 1)].first.first) && (f != -1)))
    {
      u -= 1;
    }
    re = min(re, (u + solve((i + 1), (j + 1), tv[j].first.first, -1)));
  } else
  {
    if ((sv[i].first.second == tv[j].first.second))
    {
      re = min(re, solve((i + 1), (j + 1), k, -1));
    } else
    {
      if (((((f == -1) || (sv[(i - 1)].first.second != sv[i].first.second))) && (tv[j].first.first <= k)))
      {
        re = min(re, solve((i + 1), (j + 1), k, -1));
      } else
      {
        re = min(re, (1 + solve((i + 1), (j + 1), (sv[i].first.first + 1), 0)));
      }
    }
  }
  return re;
}

func main()
{
  var w: dynamic;
  read(w);
  {
    var i = 0;
    while ((i < 2))
    {
      var s: dynamic;
      read(s);
      {
        var j = 0;
        while ((j < w))
        {
          if ((s[j] == cpp_char("o")))
          {
            sv.push_back(T(P(j, i), 0));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 2))
    {
      var s: dynamic;
      read(s);
      {
        var j = 0;
        while ((j < w))
        {
          if ((s[j] == cpp_char("o")))
          {
            tv.push_back(T(P(j, i), 0));
          } else if ((s[j] == cpp_char("*")))
          {
            tv.push_back(T(P(j, i), 1));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(sv.begin(), sv.end());
  sort(tv.begin(), tv.end());
  write(solve(0, 0, -1, -1), "\n");
  return 0;
}
